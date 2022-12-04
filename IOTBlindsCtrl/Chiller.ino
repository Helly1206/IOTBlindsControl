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
  timeStamp = micros();

#ifdef DEBUG_CHILLER
  if (millis() - temptime > 1000) {
    Serial.print("Idle:");
    Serial.print(getIdlePercentage());
    Serial.println("%");
    temptime = millis();
  }
#endif
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

// Privates ........

void CChiller::sleepMode(boolean on) {
  if (on) {
    if (!hasSleepMode) {
#ifdef DEBUG_CHILLER
      Serial.println("Zzz...");
#endif
      WiFi.setSleep(SLEEP_TYPE);
      hasSleepMode = true;
    }
  } else {
    if (hasSleepMode) {
#ifdef DEBUG_CHILLER
      Serial.println("Wakey");
#endif
      WiFi.setSleep(WIFI_PS_NONE);
      hasSleepMode = false;
    }
  }
}

CChiller chiller;
