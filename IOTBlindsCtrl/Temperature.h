/* 
 * IOTBlindCtrl - Temperature sensor
 * Reads DS18S20 and and processes Temperature sensor value
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Temperature_h
#define Temperature_h

#ifndef DS18S20_PIN
#define DS18S20_PIN        -1
#endif

#define MOVAV_TEMP         16 /* moving average of 16 samples to filter out noise */
#define LIMIT_FILTER_DIFF  (0.5)
//#define MOVAV_FILTER /* If defined: MovAv filter is used, else limit filter */

#define ADDR0_DS18S20  (0x10)
#define ADDR0_DS18B20  (0x28)
#define ABS_ZERO       (-273.15)

#define SAMPLE_TEMP         1000 /* ms */ /* 5000 ms over 5 states */

class CTemp {
public:
  CTemp();
  void init(void);
  void handle(void);
  float getTemp(void);
  float getRealTimeTemp(void);
private:
  enum tempstate {start = 0, wait1 = 1, wait2 = 2, conv = 3, read = 4};
  boolean DS18S20Address(void);
  void DS18S20StartConversion(void);
  float DS18S20Temp(void);
  float Filter(float Sample);
  void ClearFilter(void);
  byte Address[8];
  boolean noDevice;
  tempstate tempState;
#ifdef MOVAV_FILTER  
  float TempMemory[MOVAV_TEMP];
  float MovAvFilter(float Sample);
  void ClearMovAvFilter(void);
#else
  float TempMemory;
  boolean FilterInit;
  float LimitFilter(float Sample);
  void ClearLimitFilter(void);
#endif
  float FilteredTemp;
  float SampledTemp;
  static void timerCallback(TimerHandle_t xTimer);
  TimerHandle_t timer;
  StaticTimer_t timerBuffer;
  static portMUX_TYPE mux;
  static boolean sampled;
};

extern CTemp temp;

#endif
