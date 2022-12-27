/* 
 * IOTBlindCtrl - HWtimer
 * Hardware timer class
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 22-5-2022
 * Copyright: Ivo Helwegen
 */

#ifndef HwTimer_h
#define HwTimer_h

#include "Arduino.h"

#if   defined(ARDUINO_ARCH_AVR)
#define TMR_PRESC_OFF     0
#define TMR_PRESC_DIV1    1
#define TMR_PRESC_DIV8    2
#define TMR_PRESC_DIV64   3
#define TMR_PRESC_DIV256  4
#define TMR_PRESC_DIV1024 5
#define TMR_PRESC_T0F     6
#define TMR_PRESC_T0R     7

#define TMR_SINGLE        0
#define TMR_LOOP          1

#define TIMER1_RESOLUTION 65536UL  // Timer1 is 16 bit
#define TMR_OFF_TIME_US   50UL
#elif defined(ARDUINO_ARCH_ESP8266)
#define TMR_PRESC_DIV1    TIM_DIV1
#define TMR_PRESC_DIV16   TIM_DIV16
#define TMR_PRESC_DIV256  TIM_DIV256

#define TMR_SINGLE        TIM_SINGLE
#define TMR_LOOP          TIM_LOOP

#define TIMER1_RESOLUTION 8388607UL  // Timer1 is 23 bit
#elif defined(ARDUINO_ARCH_ESP32)
#define TMR_PRESC_MIN     2
#define TMR_PRESC_DIV80   80
#define TMR_PRESC_DIV800  800
#define TMR_PRESC_DIV8000 8000
#define TMR_PRESC_US      TMR_PRESC_DIV80
#define TMR_PRESC_10US    TMR_PRESC_DIV800
#define TMR_PRESC_100US   TMR_PRESC_DIV8000

#define TMR_SINGLE        false
#define TMR_LOOP          true

#define TIMER0_RESOLUTION 4294967295UL // Use 32 bit maximum, as it can hold more than 2000 s with prescaler 80
//#define TIMER0_RESOLUTION 18446744073709551615ULL  // Timer0 is 64 bit
#else 
  #error "This timer only supports boards with an AVR, ESP8266 or ESP32 processor."
#endif

class CHwTimer {
public:
  CHwTimer(); // constructor
  void init(unsigned short divider, byte reload);
  unsigned short getPrescaler(unsigned long umaxtime);
  void enable();
  void disable();
  void reset();
  void disablereset();
  void trigger(unsigned long utime);
  void update(unsigned long utime); // same as trigger, but doesn't enable
  void attachInterrupt(void (*userFunc)());
  void detachInterrupt();
  unsigned long getuTime();
private:
  unsigned short prescaler;
  byte prescalerbits;
  byte timer_reload;
  bool timer_enabled;
#if defined(ARDUINO_ARCH_ESP32)
  hw_timer_t *timer;
  unsigned long utimex;
#endif
};

extern CHwTimer hwtimer;

#endif
