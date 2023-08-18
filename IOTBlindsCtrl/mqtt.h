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

#define MQTT_SERVER "mqtt.broker.com"
#define MQTT_PORT 1883

#define MQTT_PUBLISH_TIME   1000 /* ms */
#define MQTT_RECONNECT_TIME 5000 /* ms */

#define CONNECT_TIMER       0
#define PUBLISH_TIMER       1

typedef struct { 
  String tag;
  String description;
} topics;

typedef struct { 
  String value;
  unsigned long updateCounter;
} valueMem;

typedef struct { 
  String name;
  String id;
  String cla;
  String type;
} hatopic;

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

const char dev_mf[] = "IOTControl";
const char dev_mdl[] = "IOTBlindsControl";
const char ha_config[] = "config";
const char ha_status[] = "status";
const char ha_online[] = "online";
const char ha_offline[] = "offline";

const topics SubscribeTopics[] {
  {pos_updown, pos_updown_cmt},
  {pos_up, pos_up_cmt},
  {pos_down, pos_down_cmt},
  {pos_shade, pos_shade_cmt},
  {pos_pos, pos_pos_cmt}
};

const hatopic ha_blind  {"Blind",       "blind",  "blind",       "cover"}; // name, id, cla, type
const hatopic ha_up     {"Up",          "up",     "",            "button"}; // name, id, cla, type
const hatopic ha_down   {"Down",        "down",   "",            "button"}; // name, id, cla, type
const hatopic ha_shade  {"Shade",       "shade",  "",            "button"}; // name, id, cla, type
const hatopic ha_temp   {"Temperature", "temp",   "temperature", "sensor"}; // name, id, cla, type
const hatopic ha_light  {"Light",       "light",  "illuminance", "sensor"}; // name, id, cla, type

class cMqtt {
  public:
    cMqtt(); // constructor
    void init();
    void handle();
    void update();
    String fixTopic(String topic);
    String getValue(String tag);
    String buildTopic(String tag);
    String clientId;
    boolean connected;
  private:
    enum hastatus {unknown = 0, 
                   online  = 1, 
                   offline = 2};
    static void callback(char* topic, byte* payload, unsigned int length);
    void sendStatus();
    void reconnect();
    void homeAssistantDiscovery();
    static String getTag(String topic);
    static String getMain(String topic);
    static String bp2string(byte *payload, unsigned int length);
    static boolean getBoolean(String payload);
    static boolean getBoolean2(String payload);
    static byte getPercentage(String payload);
    String joinTopic(String topic, String tag);
    String us(String tag);
    valueMem *publishMem;
    boolean reconnect_wait;
    static void timerCallback(TimerHandle_t xTimer);
    TimerHandle_t conTimer;
    StaticTimer_t conTimerBuffer;
    static portMUX_TYPE mux;
    TimerHandle_t pubTimer;
    StaticTimer_t pubTimerBuffer;
    static boolean doPub;
    static hastatus statusHa;
    static boolean discoUpdate;
};

extern cMqtt mqtt;

#endif
