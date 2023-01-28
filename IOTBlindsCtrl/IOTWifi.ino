/* 
 * IOTWifi
 * Wifi access point and Wifi control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#include "IOTWifi.h"

cIOTWifi::timerstatus cIOTWifi::timerStatus = none;
portMUX_TYPE cIOTWifi::mux = portMUX_INITIALIZER_UNLOCKED;

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
  timer = xTimerCreateStatic("wifi", pdMS_TO_TICKS(CONNECTIONDELAY), pdFALSE, (void *)0, timerCallback, &timerBuffer);
  connectWifi(false);
  LED.WifiNC();
  logger.printf(LOG_WIFI, "Settings size: " + String(settings.memsize) + " bytes");
  logger.printf(LOG_WIFI, "Press Flash or up button to enter access point mode, while LED is flashing");
}

void cIOTWifi::connect() {
  logger.printf(LOG_WIFI, "Connecting wifi");
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
        logger.connect();
        logger.printf("Wifi connection successfully established");
        logger.printf("Wifi connected to SSID: " + String(ssid));
        logger.printf("Wifi IP address: "+ String(WiFi.localIP().toString()));
        xTimerChangePeriod(timer, pdMS_TO_TICKS(MDNSCONNECTDELAY), portMAX_DELAY);
        portENTER_CRITICAL(&mux);
        timerStatus = none;
        portEXIT_CRITICAL(&mux);
        LED.WifiC();
        connected = true;
      } else {
        switch (s) {
          case WL_NO_SSID_AVAIL:
            logger.printf(LOG_WIFI, "Configured SSID cannot be reached");       
            break;
          case WL_CONNECT_FAILED:
            logger.printf(LOG_WIFI, "Connection failed");
            break;
          case WL_CONNECTION_LOST:
            logger.printf(LOG_WIFI, "WConnection lost");
            break;
          case WL_DISCONNECTED:
            logger.printf(LOG_WIFI, "Disconnected");
            break;  
          default:
            logger.printf(LOG_WIFI, "Unexpected Wifi status");
            break; 
        }
        LED.WifiNC();
        connected = false;
        logger.disconnect();
        if (status == WL_CONNECTED) {
          MDNS.end();
        }
      }
    }
    if (s != WL_CONNECTED) {
      if ((!digitalRead(FLASH_PIN)) || (buttons.initButtonPressed()) || (timerStatus == timeout)) { // enter access point mode
        logger.printf(LOG_WIFI, "Setting up as access point");           
        portENTER_CRITICAL(&mux);
        timerStatus = none;
        portEXIT_CRITICAL(&mux);
        connectAccessPoint();
      }
    }
    if (timerStatus == mdns) {
      portENTER_CRITICAL(&mux);
      timerStatus = none;
      portEXIT_CRITICAL(&mux);
      // Setup MDNS responder
      if (!MDNS.begin(hostname.c_str())) {
        logger.printf("Wifi error setting up MDNS responder!");
        MDNS.end();
        xTimerChangePeriod(timer, pdMS_TO_TICKS(MDNSCONNECTDELAY), portMAX_DELAY);
      } else {
        logger.printf("Wifi mDNS responder started");
        logger.printf("Wifi hostname: http://" + String(hostname) + ".local");
        // Add service to MDNS-SD
        MDNS.addService("http", "tcp", WEB_PORT); 
      }
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
  logger.printf(LOG_WIFI, "Connecting as access point...");
  WiFi.persistent(false);
  WiFi.disconnect();
  WiFi.softAPConfig(*apIP, *apIP, *netMsk);
  WiFi.softAP(APssid.c_str(), APPSK);
  delay(500); // Without delay I've seen the IP address blank
  logger.printf("Wifi access point IP address: " + String(WiFi.softAPIP().toString()));

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
  xTimerChangePeriod(timer, pdMS_TO_TICKS(CONNECTIONDELAY), portMAX_DELAY);
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
  logger.printf("Connecting as wifi client...");
  compSsidPass(password);
  if ((force) || (!compSsidPass(password))) {
    WiFi.begin(ssid.c_str(), password.c_str());  
  } else {
    WiFi.begin();
  } 
}

void cIOTWifi::timerCallback(TimerHandle_t xTimer) {
  portENTER_CRITICAL(&mux);
  if (pdTICKS_TO_MS(xTimerGetPeriod(xTimer)) >= CONNECTIONDELAY) {
    timerStatus = timeout;
  } else {
    timerStatus = mdns;
  }
  portEXIT_CRITICAL(&mux);
}

cIOTWifi iotWifi;
