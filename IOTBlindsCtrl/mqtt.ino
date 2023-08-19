/* 
 * IOTBlindCtrl - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#include "mqtt.h"

portMUX_TYPE cMqtt::mux = portMUX_INITIALIZER_UNLOCKED;
boolean cMqtt::doPub = false;
cMqtt::hastatus cMqtt::statusHa = cMqtt::unknown;
boolean cMqtt::discoUpdate = false;

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
  connected = false;
}

void cMqtt::init() {
  IPAddress ip;  
  client.setBufferSize(512);  
  if (ip.fromString(settings.getString(settings.brokerAddress))) {
    logger.printf(LOG_MQTT, "mqtt server set from IP");
    client.setServer(ip, settings.getShort(settings.mqttPort));
  } else {
    logger.printf(LOG_MQTT, "mqtt server set from hostname");
    client.setServer(settings.getString(settings.brokerAddress).c_str(), settings.getShort(settings.mqttPort));
  }
  client.setCallback(callback);
  clientId = "IOTBlindsCtrl_" + iotWifi.MacPart(6);
  conTimer = xTimerCreateStatic("mqttcon", pdMS_TO_TICKS(MQTT_RECONNECT_TIME), pdFALSE, (void *)CONNECT_TIMER, timerCallback, &conTimerBuffer);
  reconnect_wait = false;
  pubTimer = xTimerCreateStatic("mqttpub", pdMS_TO_TICKS(MQTT_PUBLISH_TIME), pdTRUE, (void *)PUBLISH_TIMER, timerCallback, &pubTimerBuffer);
  xTimerStart(pubTimer, portMAX_DELAY);
}

void cMqtt::handle() {
  if ((iotWifi.connected) && ((boolean)settings.getByte(settings.UseMqtt))) {
    connected = client.connected();
    if (!connected) {
      if (!reconnect_wait) {
        xTimerStart(conTimer, portMAX_DELAY);
        reconnect_wait = true;
      } else if (xTimerIsTimerActive(conTimer) == pdFALSE) {
        reconnect_wait = false;        
        reconnect();
      }
    } else { // Client connected
      if (discoUpdate) {
        portENTER_CRITICAL(&mux);
        discoUpdate = false;
        portEXIT_CRITICAL(&mux);
        homeAssistantDiscovery();
      }   
      client.loop();
      sendStatus();
    }
  } else {
    connected = false;
  }
}

void cMqtt::update() {
  if ((boolean)settings.getByte(settings.haDisco)) {
    portENTER_CRITICAL(&mux);
    discoUpdate = true;
    portEXIT_CRITICAL(&mux);
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
    value = String(lightSensor.getRaw());
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

  tag = getTag(String(topic));
  payld = bp2string(payload, length);
  logger.printf(LOG_MQTT, "Message received [" + String(topic) + "] " + String(payld));

  if (tag == pos_updown) {
    if (getBoolean2(payld)) {
      logger.printf(LOG_MQTTCMD, "Command DOWN");
      stateMachine.setCmd(CMD_DOWN);
    } else {
      logger.printf(LOG_MQTTCMD, "Command UP");
      stateMachine.setCmd(CMD_UP);
    }
  } else if (tag == pos_up) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command UP");      
      stateMachine.setCmd(CMD_UP);
    } else {
      logger.printf(LOG_MQTTCMD, "Command STOP");
      stateMachine.setCmd(CMD_STOP);
    }
  } else if (tag == pos_down) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command DOWN");
      stateMachine.setCmd(CMD_DOWN);
    } else {
      logger.printf(LOG_MQTTCMD, "Command STOP");
      stateMachine.setCmd(CMD_STOP);
    }
  } else if (tag == pos_shade) {
    if (getBoolean(payld)) {
      logger.printf(LOG_MQTTCMD, "Command SHADE");
      stateMachine.setCmd(CMD_SHADE);
    } else {
      logger.printf(LOG_MQTTCMD, "Command STOP");
      stateMachine.setCmd(CMD_STOP);
    }
  } else if (tag == pos_pos) {
    logger.printf(LOG_MQTTCMD, "Command POS");
    stateMachine.setCmd(getPercentage(payld));
  } else if ((getMain(topic) == settings.getString(settings.haTopic)) && (tag == ha_status)) { //homeassistant/status
    if (payld == ha_online) {
      logger.printf(LOG_MQTTCMD, "HA online");
      portENTER_CRITICAL(&mux);
      statusHa = online;
      discoUpdate = true;
      portEXIT_CRITICAL(&mux);
    } else if (payld == ha_offline) {
      logger.printf(LOG_MQTTCMD, "HA offline");
      portENTER_CRITICAL(&mux);
      statusHa = offline;
      portEXIT_CRITICAL(&mux);
    }
  } 
}

void cMqtt::sendStatus() { // publish on connected or (every ten minutes or) when value changes (5 seconds for temperature and light, 1 second for pos)
  if ((connected) && (doPub)) {
    portENTER_CRITICAL(&mux);
    doPub = false;
    portEXIT_CRITICAL(&mux);
    int publishLen = (sizeof(PublishTopics) / sizeof(topics));
    for (int i = 0; i < publishLen; i++) {
      String val = getValue(PublishTopics[i].tag);
      if (PublishTopics[i].tag == pos_status) {
        if (val != publishMem[i].value) {
          client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
          publishMem[i].value = val;
          publishMem[i].updateCounter = 0;
          logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
        }
      } else {
        if ((publishMem[i].updateCounter >= 5) && (val != publishMem[i].value)) {
          client.publish(buildTopic(PublishTopics[i].tag).c_str(), val.c_str(), (boolean)settings.getByte(settings.mqttRetain));
          publishMem[i].value = val;
          publishMem[i].updateCounter = 0;
          logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
        }
      }
      publishMem[i].updateCounter++;
    }
  }
}

void cMqtt::reconnect() {
  boolean connAttempt = false;
  String username = settings.getString(settings.mqttUsername);
  String password = settings.getString(settings.mqttPassword);
  if (username.length() > 0) {
    connAttempt = client.connect(clientId.c_str(), username.c_str(), password.c_str());
  } else {
    connAttempt = client.connect(clientId.c_str());
  }  
  
  if (connAttempt) {
    logger.printf(LOG_MQTT, "MQTT connected");
    logger.printf(logger.l14, "MQTT connected");
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
      logger.printf(LOG_MQTT, "Message published [" + String(buildTopic(PublishTopics[i].tag)) + "] " + String(val));
    }
    update();
  } else {
    logger.printf(LOG_MQTT, "MQTT connection failed, rc=" + String(client.state()) + " try again in 5 seconds");
    logger.printf(logger.l14, "MQTT connection failed, rc=" + String(client.state()) + " try again in 5 seconds");
  }
  return;
}

void cMqtt::homeAssistantDiscovery() {
  logger.printf(LOG_MQTT, "Home Assistant Discovery");
  JSON jString;
  JSON jDeviceString;
  String arraystr[1];
  String devName = getTag(settings.getString(settings.mainTopic));
  String topic;

  arraystr[0] = iotWifi.MacPart(6);
  jDeviceString.AddArray("ids", arraystr, 1);
  jDeviceString.AddItem("name", devName);
  jDeviceString.AddItem("mf", String(dev_mf));
  jDeviceString.AddItem("mdl", String(dev_mdl));
  //logger.printf(jDeviceString.GetJson());
  
  jString.Clear();

  jString.AddItem("name", ha_blind.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("dev_cla", ha_blind.cla);
  jString.AddItem("cmd_t", "~/updown");
  jString.AddItem("pl_cls", "1");
  jString.AddItem("pl_open", "0");
  jString.AddItem("set_pos_t", "~/pos");
  jString.AddItem("pos_t", "~/pos_status"); 
  jString.AddItem("pos_clsd", 0); 
  jString.AddItem("pos_open", 100); 
  jString.AddItem("uniq_id", arraystr[1] + us(ha_blind.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_blind.type), devName + us(ha_blind.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_up.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/up");
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_up.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_up.type), devName + us(ha_up.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_down.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/down");
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_down.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_down.type), devName + us(ha_down.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_shade.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("cmd_t", "~/shade"); 
  jString.AddItem("pl_prs", "1");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_shade.id));
  jString.AddItem("dev", jDeviceString);
  //logger.printf(jString.GetJson());

  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_shade.type), devName + us(ha_shade.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_temp.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("dev_cla", ha_temp.cla);
  jString.AddItem("stat_t", "~/temp_status");
  jString.AddItem("unit_of_meas", "°C");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_temp.id));
  jString.AddItem("dev", jDeviceString);
  
  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_temp.type), devName + us(ha_temp.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);

  jString.Clear();

  jString.AddItem("name", ha_light.name);
  jString.AddItem("~", settings.getString(settings.mainTopic));
  jString.AddItem("dev_cla", ha_light.cla);
  jString.AddItem("stat_t", "~/light_status");
  jString.AddItem("uniq_id", arraystr[1] + us(ha_light.id));
  jString.AddItem("dev", jDeviceString);
  
  topic = joinTopic(joinTopic(joinTopic(settings.getString(settings.haTopic), ha_light.type), devName + us(ha_light.id)), ha_config);
  client.publish(topic.c_str(), jString.GetJson().c_str(), true);
}

String cMqtt::getTag(String topic) {
  String tag = "";
  int slashPos = topic.lastIndexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(slashPos+1);
  }
  return tag;
}

String cMqtt::getMain(String topic) {
  String tag = "";
  int slashPos = topic.indexOf('/');
  if (slashPos > 0) {
    tag = topic.substring(0, slashPos);
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

String cMqtt::joinTopic(String Topic, String tag) {
  return Topic + "/" + tag;
}

String cMqtt::us(String tag) {
  return "_" + tag;
}

void cMqtt::timerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == PUBLISH_TIMER) {
    portENTER_CRITICAL(&mux);
    doPub = true;
    portEXIT_CRITICAL(&mux);
  }
}

cMqtt mqtt;
