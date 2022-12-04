/* 
 * IOTWifi
 * Wifi access point and Wifi control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#include "IOTWifi.h"
#include "Settings.h"
#include "LEDs.h"
#include "Timers.h"

cIOTWifi::cIOTWifi() { // constructor
  AccessPoint = false;
  dnsServer = new DNSServer;
  apIP = new IPAddress(8, 8, 8, 8);
  netMsk = new IPAddress(255, 255, 255, 0);
  status = WL_IDLE_STATUS;
  connected = false;
}

void cIOTWifi::init() {
  pinMode(FLASH_PIN, INPUT_PULLUP);
  APssid = String(APSSID)+ "_" + MacPart(6);
  settings.get(settings.hostname, hostname);
  connectWifi(false);
  LED.WifiNC();
  
#ifdef DEBUG_IOTWIFI
  Serial.printf("DEBUG: settings size: %d bytes\n",settings.memsize);
  Serial.print("Wifi: Press Flash or up button to enter access point mode, while LED is flashing\n");
#endif
}

void cIOTWifi::connect() {
#ifdef DEBUG_IOTWIFI
  Serial.print("Wifi: Connecting wifi\n");
#endif
  connectWifi(true);
}

void cIOTWifi::handle() {
  if (AccessPoint) {
    dnsServer->processNextRequest();
    connected = false;
  } else {
    unsigned int s = WiFi.status();
    if (status != s) { // WLAN status change
      if (s == WL_CONNECTED) {
#ifdef DEBUG_IOTWIFI
          Serial.print("Wifi: Connection successfully established\n");
          Serial.print("Wifi: Connected to SSID: " + ssid + "\n");
          Serial.print("Wifi: IP address: ");
          Serial.print(WiFi.localIP());
          Serial.print("\n");
#endif
          timers.start(TIMER_WIFI, MDNSCONNECTDELAY, false);
          LED.WifiC();
          connected = true;
      } else {
#ifdef DEBUG_IOTWIFI
        switch (s) {
          case WL_NO_SSID_AVAIL:
            Serial.print("Wifi: Configured SSID cannot be reached\n");       
            break;
          case WL_CONNECT_FAILED:
            Serial.print("Wifi: Connection failed\n");
            break;
          case WL_CONNECTION_LOST:
            Serial.print("Wifi: Connection lost\n");
            break;
          case WL_DISCONNECTED:
            Serial.print("Wifi: Disconnected\n");
            break;  
          default:
            Serial.print("Wifi: unexpected Wifi status\n");
            break; 
        }
#endif
        LED.WifiNC();
        connected = false;
      }
    }
    if (s != WL_CONNECTED) {
      if ((!digitalRead(FLASH_PIN)) || (buttons.initButtonPressed()) || (timers.getTimer(TIMER_WIFI))) { // enter access point mode
#ifdef DEBUG_IOTWIFI
        Serial.print("Wifi: Setting up as access point\n");
#endif            
        connectAccessPoint();
      }
    }
    if (s == WL_CONNECTED) {
      if (timers.running(TIMER_WIFI)) {
        if (timers.getTimer(TIMER_WIFI)) {
          // Setup MDNS responder
          if (!MDNS.begin(hostname.c_str())) {
#ifdef DEBUG_IOTWIFI
            Serial.print("Wifi: Error setting up MDNS responder!\n");
#endif
            MDNS.end();
            timers.start(TIMER_WIFI, MDNSCONNECTDELAY, false);
          } else {
#ifdef DEBUG_IOTWIFI
            Serial.print("Wifi: mDNS responder started\n");
            Serial.print("Wifi: Hostname: http://" + hostname + ".local\n");
#endif
            // Add service to MDNS-SD
            MDNS.addService("http", "tcp", WEB_PORT); 
          }
        }
      }
    } else if (status == WL_CONNECTED) {
      MDNS.end();
    }
    status = s;
  }
}

String cIOTWifi::MacPart(int n) {
  String Mac = WiFi.macAddress();
  Mac.replace(":", "");
  if ((n > 0) && (n < 12)) {
    Mac.remove(0, 12 - n);
  }
  return Mac;
}

boolean cIOTWifi::isAccessPoint() {
  return AccessPoint;
}

boolean cIOTWifi::wakingUp() {
  return (!AccessPoint & !connected);
}

///////////// PRIVATES ///////////////////////////

void cIOTWifi::connectAccessPoint() {
  AccessPoint = true;
#ifdef DEBUG_IOTWIFI
  Serial.print("Wifi: Connecting as access point...\n");
#endif
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.softAPConfig(*apIP, *apIP, *netMsk);
  WiFi.softAP(APssid.c_str(), APPSK);
  delay(500); // Without delay I've seen the IP address blank
#ifdef DEBUG_IOTWIFI
  Serial.print("Wifi: Access point IP address: ");
  Serial.print(WiFi.softAPIP());
  Serial.print("\n");
#endif

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", *apIP);
  LED.WifiApC();
}

bool cIOTWifi::compSsidPass(String password) {
  bool equal = true;
  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);
  equal = (String((char *)conf.sta.ssid).equals(ssid));
  if (equal) {
    equal = (String((char *)conf.sta.password).equals(password));
  }
  return equal;
}

void cIOTWifi::connectWifi(bool force) {
  String password;
  timers.start(TIMER_WIFI, CONNECTIONDELAY, false);
  if (AccessPoint) {
    WiFi.softAPdisconnect(true);
  }
  AccessPoint = false;
  settings.get(settings.ssid, ssid);
  settings.get(settings.password, password);
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
#ifdef DEBUG_IOTWIFI
  Serial.print("Wifi: Connecting as wifi client...\n");
#endif
  compSsidPass(password);
  if ((force) || (!compSsidPass(password))) {
    WiFi.begin(ssid.c_str(), password.c_str());  
  } else {
    WiFi.begin();
  } 
}

cIOTWifi iotWifi;
