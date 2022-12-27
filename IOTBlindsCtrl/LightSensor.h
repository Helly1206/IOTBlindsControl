/* 
 * IOTBlindCtrl - Lightsensor
 * Reads and and processes analog LightSensor value
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef LightSensor_h
#define LightSensor_h

#define USE_PIND
#ifndef SENSOR_PIND
#define SENSOR_PIND        -1
#endif
#ifndef SENSOR_PINA
#define SENSOR_PINA        -1
#endif

#define NEGATIVE           false
#define POSITIVE           true

class CLightSensor {
public:
  CLightSensor(); // constructor
  void init(void);
  void handle(void);
  void reset(void);
  void setTwilight(boolean bval);
  void setSunny(boolean bval);
  unsigned short getRaw();
  unsigned short OutSunny;
  unsigned short OutTwilight;
  boolean Twilight;
  boolean Sunny;
  boolean Twilightd;
  boolean Sunnyd;
private:
  struct rawReadout {
    unsigned short value;
    bool sample;
  };
  void CalcDark(void);
  void CalcTwilight(void);
  void CalcSunny(void);
  unsigned short Integrate(unsigned short value, unsigned short gain, boolean sign);
  boolean CompThreshold(unsigned short value, unsigned short threshold, boolean sign);
  static void timerCallback(TimerHandle_t xTimer);
  TimerHandle_t timer;
  StaticTimer_t timerBuffer;
  static portMUX_TYPE mux;
  static rawReadout raw;
};

extern CLightSensor lightSensor;

#endif
