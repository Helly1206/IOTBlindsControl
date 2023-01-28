/* 
 * IOTBlindCtrl - udplogger
 * UDP logger
 * Version 0.80
 * 13-1-2023
 * Copyright: Ivo Helwegen
 */

#ifndef UDPLOGGER_h
#define UDPLOGGER_h

#include <WiFiUdp.h>

#define UDP_ADDRESS      "255.255.255.255"

#define LOG_BLIND         logger.l1
#define LOG_BLIND_TIMING  logger.l2
#define LOG_BUTTONS       logger.l3
#define LOG_CHILLER       logger.l4
#define LOG_NTP           logger.l5
#define LOG_WIFI          logger.l6
#define LOG_LEDS          logger.l7
#define LOG_STATEMACHINE  logger.l8
#define LOG_TEMPERATURE   logger.l9
#define LOG_WEBSERVER     logger.l10
#define LOG_MQTT          logger.l11
#define LOG_MQTTCMD       logger.l12

const char text0[] = "blind actuation";
const char text1[] = "blind actuation timing";
const char text2[] = "buttons";
const char text3[] = "chiller (power saving)";
const char text4[] = "NTP (clock)";
const char text5[] = "WiFi";
const char text6[] = "LEDs";
const char text7[] = "statemachine";
const char text8[] = "temperature sensor";
const char text9[] = "webserver";
const char text10[] = "MQTT (command interface)";
const char text11[] = "MQTT commands";
const char text12[] = "N/A";
const char text13[] = "N/A";
const char text14[] = "N/A";
const char text15[] = "N/A";

const String levelTexts[] {
  text0, text1, text2, text3, text4, text5, text6, text7, text8, text9, text10, text11, text12, text13, text14, text15
};

const char stext0[] = "BLIND";
const char stext1[] = "TIMING";
const char stext2[] = "BUTTONS";
const char stext3[] = "CHILLER";
const char stext4[] = "NTP";
const char stext5[] = "WIFI";
const char stext6[] = "LEDS";
const char stext7[] = "STATE";
const char stext8[] = "TEMP";
const char stext9[] = "WEB";
const char stext10[] = "MQTT";
const char stext11[] = "MQTTCMD";
const char stext12[] = "NA";
const char stext13[] = "NA";
const char stext14[] = "NA";
const char stext15[] = "NA";

const String shortTexts[] {
  stext0, stext1, stext2, stext3, stext4, stext5, stext6, stext7, stext8, stext9, stext10, stext11, stext12, stext13, stext14, stext15
};

class cUdpLogger {
  public:
    enum loglevel {l1 = 0, l2 = 1, l3 = 2, l4 = 3, l5 = 4, l6 = 5, l7 = 6, l8 = 7, l9 = 8, l10 = 9, l11 = 10, l12 = 11, l13 = 12, l14 = 13, l15 = 14, l16 = 15};
    cUdpLogger(); // constructor
    void connect();
    void disconnect();
    void printf(String data);
    void printf(loglevel level, String data);
    void enable(bool bEnable);
    bool isEnabled();
    void setDebug(uint16_t level);
    uint16_t getDebug();
  private:
    void initSettings();
    WiFiUDP *logUDP;
    bool connected;
    bool enabled;
    uint16_t debugLevel;
    uint16_t port;    
};

extern cUdpLogger logger;

#endif