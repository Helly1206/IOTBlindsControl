/* 
 * IOTBlindCtrl - Chiller
 * Managing delays for low power
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 12-6-2022
 * Copyright: Ivo Helwegen
 */
 
#include "Chiller.h"

CChiller::CChiller() { // constructor
  chillMode = none;
  hasSleepMode = false;
  timeStamp = 0;
  uTaskDuration = 0;
  ctr = 0;
}

void CChiller::init() {
  boost(false);
  sleepMode(SLEEP_INIT_ON);
  setMode(interval);
  xLastWakeTime = xTaskGetTickCount();
  timeStamp = micros();
}

void CChiller::handle() {
  uTaskDuration = micros() - timeStamp;
  if (chillMode != none) {
    if (chillMode == delay) {
      vTaskDelay(xDuration/portTICK_PERIOD_MS);
      xLastWakeTime = xTaskGetTickCount();
    } else { // interval
      vTaskDelayUntil(&xLastWakeTime, xDuration/portTICK_PERIOD_MS);
    }
  }
  if (ctr > 1000/xDuration) {
    logger.printf(LOG_CHILLER, "Idle:" + String(getIdlePercentage()) + "%");
    ctr = 0;
  } else {
    ctr++;
  }
  timeStamp = micros();
}

void CChiller::setMode(CChiller::chillmode mode) {
  chillMode = mode;
  if (chillMode == delay) {
    xDuration = CHILLER_DELAY_TIME;
  } else {
    xDuration = CHILLER_INTERVAL_TIME;
  }
}

CChiller::chillmode CChiller::getMode() {
  return chillMode;
}

void CChiller::setSleep(boolean on) {
  sleepMode(on);
}

boolean CChiller::getSleep() {
  return hasSleepMode;
}

float CChiller::getIdlePercentage() {
  unsigned long uDuration = (xDuration*1000/portTICK_PERIOD_MS);
  if (uDuration < uTaskDuration) {
    uDuration = uTaskDuration;
  }
  return ((float)((uDuration - uTaskDuration)*100)/(float)(uDuration));
}

void CChiller::boost(boolean doBoost) {
  if (doBoost) {
    setCpuFrequencyMhz(FREQ_BOOST);
  } else {
    setCpuFrequencyMhz(FREQ_CHILL);
  }
}

String CChiller::getResetReason(int cpuNo) {
  String retStr;
  switch (rtc_get_reset_reason(cpuNo)) {
    case 1  : retStr = "Vbat power on reset"; break;
    case 3  : retStr = "Software reset digital core"; break;
    case 4  : retStr = "Legacy watch dog reset digital core"; break;
    case 5  : retStr = "Deep Sleep reset digital core"; break;
    case 6  : retStr = "Reset by SLC module, reset digital core"; break;
    case 7  : retStr = "Timer Group0 Watch dog reset digital core"; break;
    case 8  : retStr = "Timer Group1 Watch dog reset digital core"; break;
    case 9  : retStr = "RTC Watch dog Reset digital core"; break;
    case 10 : retStr = "Instrusion tested to reset CPU"; break;
    case 11 : retStr = "Time Group reset CPU"; break;
    case 12 : retStr = "Software reset CPU"; break;
    case 13 : retStr = "RTC Watch dog Reset CPU"; break;
    case 14 : retStr = "for APP CPU, reseted by PRO CPU"; break;
    case 15 : retStr = "Reset when the vdd voltage is not stable"; break;
    case 16 : retStr = "RTC Watch dog reset digital core and rtc module"; break;
    default : retStr = "Unknown reset reason";
  }
  return retStr;
}

String CChiller::getHeapMem() {
  uint32_t freeHeap = ESP.getFreeHeap();
  uint32_t sizeHeap = ESP.getHeapSize();
  uint32_t usedHeap = sizeHeap-freeHeap;
    
  return String(float(usedHeap)/1024) + " kB [" + String((usedHeap*100)/sizeHeap) + " %]";
}

String CChiller::getProgramMem() {
  uint32_t usedSketch = ESP.getSketchSize();
  uint32_t freeSketch = ESP.getFreeSketchSpace();
  return String(float(usedSketch)/1024) + " kB [" + String((usedSketch*100)/freeSketch) + " %]";
}

String CChiller::getVersion() {
  return String(ESP.getSdkVersion());
}

String CChiller::getCPUFreq() {
  return String(getCpuFrequencyMhz()) + " MHz/ " + String(getXtalFrequencyMhz()) + " MHz";
}

// Privates ........

void CChiller::sleepMode(boolean on) {
  if (on) {
    if (!hasSleepMode) {
      logger.printf(LOG_CHILLER, "Zzz...");
      WiFi.setSleep(SLEEP_TYPE);
      hasSleepMode = true;
    }
  } else {
    if (hasSleepMode) {
      logger.printf(LOG_CHILLER, "Wakey");
      WiFi.setSleep(WIFI_PS_NONE);
      hasSleepMode = false;
    }
  }
}

CChiller chiller;
