/* 
 * IOTBlindCtrl - Chiller
 * Managing delays for low power
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 12-6-2022
 * Copyright: Ivo Helwegen
 */

#ifndef Chiller_h
#define Chiller_h

#include "rom/rtc.h"

#ifdef DEBUG_SERIAL
//#define DEBUG_CHILLER
#endif

#define CHILLER_DELAY_TIME     10
#define CHILLER_INTERVAL_TIME  100 //500

#define SLEEP_TYPE             WIFI_PS_MAX_MODEM  /* WIFI_PS_NONE, WIFI_PS_MIN_MODEM */
#define SLEEP_INIT_ON          true

#define FREQ_BOOST             240
#define FREQ_CHILL             80

class CChiller {
public:
  enum chillmode {none, delay, interval};
  CChiller(); // constructor
  void init();
  void handle();
  void setMode(chillmode mode);
  chillmode getMode();
  void setSleep(boolean on);
  boolean getSleep();
  float getIdlePercentage();
  void boost(boolean doBoost);
  String getResetReason(int cpuNo);
  String getHeapMem();
  String getProgramMem();
  String getVersion();
  String getCPUFreq();
private:
  void sleepMode(boolean on);
  TickType_t xLastWakeTime;
  TickType_t xDuration;
  boolean hasSleepMode;
  chillmode chillMode;
  unsigned long uTaskDuration;
  unsigned long timeStamp;
#ifdef DEBUG_CHILLER
  unsigned long temptime;
#endif
};

extern CChiller chiller;

#endif
