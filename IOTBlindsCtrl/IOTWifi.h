/* 
 * IOTWifi
 * Wifi access point and Wifi control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */


#ifndef IOTWifiCtrl_h
#define IOTWifiCtrl_h

#include <WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>

#define APSSID           "IOTBlindsCtrlAP"
#define APPSK            "12345678"
#define DNS_PORT         53
#define WEB_PORT         80
#define MDNSCONNECTDELAY 1000
#define CONNECTIONDELAY  20000

#ifndef FLASH_PIN
#define FLASH_PIN       0
#endif

class cIOTWifi {
  public:
    cIOTWifi(); // constructor
    void init();
    void connect();
    void handle();
    String MacPart(int n);
    boolean isAccessPoint();
    boolean wakingUp();
    IPAddress *apIP;
    String ssid;
    String APssid;
    String hostname;
    boolean connected;
  private:
    enum timerstatus {none = 0, timeout = 1, mdns = 2};
    void connectAccessPoint();
    bool compSsidPass(String password);
    void connectWifi(bool force);
    boolean AccessPoint;
    DNSServer *dnsServer;
    IPAddress *netMsk;
    unsigned int status;
    static void timerCallback(TimerHandle_t xTimer);
    TimerHandle_t timer;
    StaticTimer_t timerBuffer;
    static portMUX_TYPE mux;
    static timerstatus timerStatus;
};

extern cIOTWifi iotWifi;

#endif
