/* 
 * IOTBlindCtrl - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#include "mqtt.h"
#include "IOTWifi.h"
#include "Settings.h"
#include "Commands.h"
#include "Blind.h"
#include "LightSensor.h"
#include "Temperature.h"

WiFiClient espClient;
PubSubClient client(espClient);

cMqtt::cMqtt() { // constructor
  int publishLen = (sizeof(PublishTopics) / sizeof(topics));
  publishMem = new valueMem[publishLen];
  for (int i = 0; i < publishLen; i++) {
    publishMem[i].value = "";
    publishMem[i].updateCounter = 0;
  }
  clientId = "";
  connected = true;
}

void cMqtt::init() {
  IPAddress ip;  
  if (ip.fromString(settings.getString(settings.brokerAddress))) {
#ifdef DEBUG_MQTT
    Serial.print("DEBUG: mqtt server set from IP\n");
#endif
    client.setServer(ip, settings.getShort(settings.mqttPort));
  } else {
#ifdef DEBUG_MQTT
    Serial.print("DEBUG: mqtt server set from hostname\n");
#endif
    client.setServer(settings.getString(settings.brokerAddress).c_str(), settings.getShort(settings.mqttPort));
  }
  client.setCallback(callback);
  clientId = "IOTBlindsCtrl_" + iotWifi.MacPart(6);
  timers.start(MQTT_PUBLISH, MQTT_PUBLISH_TIME, true);
}

void cMqtt::handle() {
  if (!client.connected()) {
    if ((iotWifi.connected) && ((boolean)settings.getByte(settings.UseMqtt))) {
      if (!timers.running(MQTT_RECONNECT)) {
        timers.start(MQTT_RECONNECT, MQTT_RECONNECT_TIME, false);
      } else if (timers.getTimer(MQTT_RECONNECT)) {
        reconnect();
      }
    }
  } else { // Client connected
    if ((!iotWifi.connected) || (!(boolean)settings.getByte(settings.UseMqtt))) {
      client.disconnect();
      connected = false;
    } else {
      client.loop();
      sendStatus();
    }
  }
}

String cMqtt::fixTopic(String topic) {
  String slash = "/";
  if (topic.endsWith(slash)) {
    topic.remove(topic.length() - 1, 1);
  }
  if (topic.startsWith(slash)) {
    topic.remove(0, 1);
  }
  return topic;
}

String cMqtt::getValue(String tag) {
  String value = "";
  if (tag == pos_status) {
    value = String(blind.getPosition());
  } else if (tag == temp_status) {
    value = String(temp.getRealTimeTemp());
  } else if (tag == light_status) {
    value = String(lightSensor.Raw);
  }
  return value;
}

String cMqtt::buildTopic(String tag) {
  return settings.getString(settings.mainTopic) + "/" + tag;
}

///////////// PRIVATES ///////////////////////////


void cMqtt::callback(char* topic, byte* payload, unsigned int length) {
  String tag = "";
  String payld = "";
#ifdef DEBUG_MQTT
  Serial.print("DEBUG: Message received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
#endif

  tag = getTag(String(topic));
  payld = bp2string(payload, length);

  if (tag == pos_updown) {
    if (getBoolean2(payld)) {
      cmdQueue.addCommand(CMD_DOWN);
    } else {
      cmdQueue.addCommand(CMD_UP);
    }
  } else if (tag == pos_up) {
    if (getBoolean(payld)) {
      cmdQueue.addCommand(CMD_UP);
    } else {
      cmdQueue.addCommand(CMD_STOP);
    }
  } else if (tag == pos_down) {
    if (getBoolean(payld)) {
      cmdQueue.addCommand(CMD_DOWN);
    } else {
      cmdQueue.addCommand(CMD_STOP);
    }
  } else if (tag == pos_shade) {
    if (getBoolean(payld)) {
      cmdQueue.addCommand(CMD_SHADE);
    } else {
      cmdQueue.addCommand(CMD_STOP);
    }
  } else if (tag == pos_pos) {
    cmdQueue.addCommand(getPercentage(payld));
  }
}

void cMqtt::sendStatus() { // publish on connected or (every ten minutes or) when value changes (5 seconds for temperature and light, 1 second for pos)
  if (connected) {
    if (timers.getTimer(MQTT_PUBLISH)) {
      int publishLen = (sizeof(PublishTopics) / sizeof(topics));
      for (int i = 0; i < publishLen; i++) {
        String val = getValue(PublishTopics[i].tag);
        if (PublishTopics[i].tag == pos_status) {
          if (val != publishMem[i].value) {
            client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
            publishMem[i].value = val;
            publishMem[i].updateCounter = 0;
#ifdef DEBUG_MQTT
            Serial.print("DEBUG: Message published [");
            Serial.print(buildTopic(PublishTopics[i].tag));
            Serial.print("] ");
            Serial.print(val);
            Serial.println();
#endif
          }
        } else {
          if ((publishMem[i].updateCounter >= 5) && (val != publishMem[i].value)) {
            client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
            publishMem[i].value = val;
            publishMem[i].updateCounter = 0;
 #ifdef DEBUG_MQTT
            Serial.print("DEBUG: Message published [");
            Serial.print(buildTopic(PublishTopics[i].tag));
            Serial.print("] ");
            Serial.print(val);
            Serial.println();
#endif
          }
        }
        publishMem[i].updateCounter++;
      }
    }
  }
}

boolean cMqtt::reconnect() {
  boolean connAttempt = false;
  String username = settings.getString(settings.mqttUsername);
  String password = settings.getString(settings.mqttPassword);
  if (username.length() > 0) {
    connAttempt = client.connect(clientId.c_str(), username.c_str(), password.c_str());
  } else {
    connAttempt = client.connect(clientId.c_str());
  }  
  
  if (connAttempt) {
#ifdef DEBUG_MQTT
    if (!connected) {
      Serial.print("MQTT: connected\n");
    }
#endif
    int publishLen = (sizeof(PublishTopics) / sizeof(topics));
    int subscribeLen = (sizeof(SubscribeTopics) / sizeof(topics));
    for (int i = 0; i < subscribeLen; i++) {
      client.subscribe(buildTopic(SubscribeTopics[i].tag).c_str(), (int)settings.getByte(settings.mqttQos));
    }
    for (int i = 0; i < publishLen; i++) {
      String val = getValue(PublishTopics[i].tag);
      client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
      publishMem[i].value = val;
      publishMem[i].updateCounter = 1;
#ifdef DEBUG_MQTT
      Serial.print("DEBUG: Message published [");
      Serial.print(buildTopic(PublishTopics[i].tag));
      Serial.print("] ");
      Serial.print(val);
      Serial.println();
#endif
    }
  } else {
#ifdef DEBUG_MQTT
    if (connected) {
      Serial.print("MQTT: connection failed, rc=" + String(client.state()) + " try again in 5 seconds\n");
    }
#endif
  }
  connected = client.connected();
  return connected;
}

String cMqtt::getTag(String topic) {
  String tag = "";
  int slashPos = topic.lastIndexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(slashPos+1);
  }
  return tag;
}

String cMqtt::bp2string(byte *payload, unsigned int length) {
  String payld = "";
  for (int i = 0; i < length; i++) {
    payld.concat((char)payload[i]);
  }
  return payld;
}

boolean cMqtt::getBoolean(String payload) {
  boolean value = false;
  payload.toLowerCase();
  if ((payload == "on") || (payload == "true") || (payload == "1")) {
    value = true;
  }
  return value;
}

boolean cMqtt::getBoolean2(String payload) {
  boolean value = false;
  payload.toLowerCase();
  if ((payload == "on") || (payload == "true") || (payload == "1") || (payload == "down")) {
    value = true;
  }
  return value;
}

byte cMqtt::getPercentage(String payload) {
  float perc = 0;
  perc = payload.toFloat();
  if (perc < 0) {
    perc = 0;
  }
  if (perc > 100) {
    perc = 100;
  }
  return (byte)round(perc);
}

cMqtt mqtt;
