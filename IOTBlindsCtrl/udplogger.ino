/* 
 * IOTBlindCtrl - udplogger
 * UDP logger
 * Version 0.80
 * 13-1-2023
 * Copyright: Ivo Helwegen
 */

#include "udplogger.h"

cUdpLogger::cUdpLogger() { // constructor
  logUDP = new WiFiUDP();
  connected = false;
}

void cUdpLogger::connect() {
  initSettings();
  logUDP->begin(port);
  connected = true;
}

void cUdpLogger::disconnect() {
  logUDP->stop();
  connected = false;
}

void cUdpLogger::printf(String data) {
  if ((connected) && (enabled)) {
    logUDP->beginPacket(UDP_ADDRESS, port);
    logUDP->printf(data.c_str());
    logUDP->endPacket();
  }  
}

void cUdpLogger::printf(loglevel level, String data) {
  uint16_t mask = 1 << (uint8_t)level;
  if (((debugLevel & mask) != 0) && ((connected) && (enabled))) {
    cUdpLogger::printf(shortTexts[(uint8_t)level] + ":" + data);
  }
}

void cUdpLogger::enable(bool bEnable) {
  enabled = bEnable;
}

bool cUdpLogger::isEnabled() {
  return enabled;
}

void cUdpLogger::setDebug(uint16_t level) {
  debugLevel = level;
}

uint16_t cUdpLogger::getDebug() {
  return debugLevel;
}

///////////// PRIVATES ///////////////////////////

void cUdpLogger::initSettings() {
  settings.get(settings.UdpPort, port);
  settings.get(settings.UdpEnabled, (byte&)enabled);
  settings.get(settings.UpdDebugLevel, debugLevel);
}

cUdpLogger logger;