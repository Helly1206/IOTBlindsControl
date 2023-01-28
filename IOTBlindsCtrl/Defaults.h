/*
 * IOTBlindCtrl - Defaults
 * Webpage content
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 1-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef DEFAULTS_h
#define DEFAULTS_h

//#define FORCE_DEFAULTS

#define DEF_TWILIGHTIGAIN       109
#define DEF_SUNNYIGAIN          55
#define DEF_SAMPLETIME          1000
#define DEF_OUTPUTTHRESHOLD     32768
#define DEF_FIXEDOPEN           525
#define DEF_FIXEDCLOSE          1335
#define DEF_TWILIGHTTHRESHOLD   1023
#define DEF_TWILIGHTHYSTERISES  25
#define DEF_SUNNYTHRESHOLD      27
#define DEF_SUNNYHYSTERISES     100
#define DEF_SUNNYENABLETEMP     25
#define DEF_HOTDAYTEMP          27
#define DEF_HOTDAYENABLETIMEOUT 600
#define DEF_SENSORENABLED       true
#define DEF_MOTORMOVETIME       20000
#define DEF_MOTORSYNCPERC       10
#define DEF_MOTORSUNNYPOS       55
#define DEF_MOTORENABLED        true

#define DEF_SSID                ""
#define DEF_PASSWORD            ""
#define DEF_HOSTNAME            "iotblindsctrl"
#define DEF_NTPSERVER           "pool.ntp.org"
#define DEF_NTPZONE             1
#define DEF_USEDST              true
#define DEF_LOGPORT             6309
#define DEF_LOGENABLE           true
#define DEF_LOGDEBUG            0

#define DEF_BROKERADDRESS       "mqtt.broker.com"
#define DEF_MQTTPORT            1883
#define DEF_MQTTUSERNAME        ""
#define DEF_MQTTPASSWORD        ""
#define DEF_MAINTOPIC           "myhome/iotblindsctrl"
#define DEF_MQTTQOS             1
#define DEF_MQTTRETAIN          false
#define DEF_USEMQTT             true

#endif
