/* 
 * IOTBlindCtrl - MQTT
 * mqtt control
 * Version 0.80
 * 5-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef MQTT_h
#define MQTT_h

#include <PubSubClient.h>

#ifdef DEBUG_SERIAL
//#define DEBUG_MQTT
#endif

#define MQTT_SERVER "mqtt.broker.com"
#define MQTT_PORT 1883

typedef struct { 
  String tag;
  String description;
} topics;

typedef struct { 
  String value;
  unsigned long updateCounter;
} valueMem;

const char pos_status[] = "pos_status";
const char temp_status[] = "temp_status";
const char light_status[] = "light_status";
const char pos_status_cmt[] = "publish: current position of the blind [%]";
const char temp_status_cmt[] = "publish: currently measured temperature [&#176;C]";
const char light_status_cmt[] = "publish: currently measured lights [10 bit]";

const topics PublishTopics[] {
  {pos_status, pos_status_cmt},
  {temp_status, temp_status_cmt},
  {light_status, light_status_cmt}
};

const char pos_updown[] = "updown";
const char pos_up[] = "up";
const char pos_down[] = "down";
const char pos_shade[] = "shade";
const char pos_pos[] = "pos";
const char pos_updown_cmt[] = "subscribe: move blind totally up (0) or down (1) [up/ down, on/ off, true/ false, 0/ 1]";
const char pos_up_cmt[] = "subscribe: move blind totally up [on/ off, true/ false, 0/ 1]";
const char pos_down_cmt[] = "subscribe: move blind totally down [on/ off, true/ false, 0/ 1]";
const char pos_shade_cmt[] = "subscribe: move blind to shade position [on/ off, true/ false, 0/ 1]";
const char pos_pos_cmt[] = "subscribe: move blind to position [0-100 %]";

const topics SubscribeTopics[] {
  {pos_updown, pos_updown_cmt},
  {pos_up, pos_up_cmt},
  {pos_down, pos_down_cmt},
  {pos_shade, pos_shade_cmt},
  {pos_pos, pos_pos_cmt}
};

class cMqtt {
  public:
    cMqtt(); // constructor
    void init();
    void handle();
    String fixTopic(String topic);
    String getValue(String tag);
    String buildTopic(String tag);
    String clientId;
    boolean connected;
  private:
    static void callback(char* topic, byte* payload, unsigned int length);
    void sendStatus();
    boolean reconnect();
    static String getTag(String topic);
    static String bp2string(byte *payload, unsigned int length);
    static boolean getBoolean(String payload);
    static boolean getBoolean2(String payload);
    static byte getPercentage(String payload);
    valueMem *publishMem;
};

extern cMqtt mqtt;

#endif
