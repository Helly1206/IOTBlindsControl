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

#ifdef DEBUG_SERIAL
#define DEBUG_IOTWIFI
#endif

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
    void connectAccessPoint();
    bool compSsidPass(String password);
    void connectWifi(bool force);
    boolean AccessPoint;
    DNSServer *dnsServer;
    IPAddress *netMsk;
    unsigned int status;
};

extern cIOTWifi iotWifi;

#endif
