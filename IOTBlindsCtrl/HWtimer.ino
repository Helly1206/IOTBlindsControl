/* 
 * IOTBlindCtrl - HWtimer
 * Hardware timer class
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 22-5-2022
 * Copyright: Ivo Helwegen
 */

#include "HWtimer.h"

CHwTimer::CHwTimer() { // constructor
  prescalerbits = 0;
  prescaler = 1;
  timer_reload = TMR_LOOP;
  timer_enabled = false;
#if defined(ARDUINO_ARCH_ESP32)
  timer = NULL;
#endif
}

void CHwTimer::disablereset() {
  disable();
  reset();
};

void CHwTimer::trigger(unsigned long utime) {
  update(utime);
  enable();
}

#if   defined(ARDUINO_ARCH_AVR)
static volatile void (*user_cb)(void);
static volatile bool single = false;

void CHwTimer::init(unsigned short divider, byte reload) {
  prescalerbits = (byte)divider;
  if (divider == TMR_PRESC_DIV8) {
    prescaler = 8;
  } else if (divider == TMR_PRESC_DIV64) {
    prescaler = 64;
  } else if (divider == TMR_PRESC_DIV256) {
    prescaler = 256;
  } else if (divider == TMR_PRESC_DIV1024) {
    prescaler = 1024;
  } else {
    prescaler = 1;
  }
  timer_reload = reload;
  single = (reload == TMR_SINGLE);
  TCCR1A = 0;
  TCCR1B = _BV(WGM12);
  OCR1A = 0;
  TCNT1 = 0;
  TIMSK1 |= _BV(OCIE1A);
}

unsigned short CHwTimer::getPrescaler(unsigned long umaxtime) {
  unsigned short divider = TMR_PRESC_OFF;
  if ((((F_CPU/100000)*umaxtime)/10) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV1;
  } else if ((((F_CPU/100000)*umaxtime)/(8*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV8;
  } else if ((((F_CPU/100000)*umaxtime)/(64*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV64;
  } else if ((((F_CPU/100000)*umaxtime)/(256*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV256;
  } else if ((((F_CPU/100000)*umaxtime)/(1024*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV1024;
  }
  return divider;
}

void CHwTimer::attachInterrupt(void (*userFunc)()) {
  user_cb = userFunc;
}

void CHwTimer::detachInterrupt() {
  user_cb = NULL;
}

void CHwTimer::enable() {
  TCCR1B = _BV(WGM12) | prescalerbits;
}

void CHwTimer::disable() {
  TCCR1B = _BV(WGM12);
}

void CHwTimer::reset() {
  TCNT1 = 0;
}

void CHwTimer::update(unsigned long utime) {
  if (single) {
    OCR1A = (unsigned short)(((F_CPU/100000)*(utime-TMR_OFF_TIME_US))/(prescaler*10) - 1);
  } else {
    OCR1A = (unsigned short)(((F_CPU/100000)*utime)/(prescaler*10) - 1);
  }
}

unsigned long CHwTimer::getuTime() {
  return ((unsigned long)TCNT1)*(prescaler*10)/(F_CPU/100000);;
}

// Privates !!!!!!!!!!!!!

ISR(TIMER1_COMPA_vect) {
  if (single) {
    TCCR1B = _BV(WGM12);
    TCNT1  = 0;
  }
  if (user_cb) {
    user_cb();
  }
}

#elif defined(ARDUINO_ARCH_ESP8266)

void CHwTimer::init(unsigned short divider, byte reload) {
  prescalerbits = (byte)divider;
  if (divider == TMR_PRESC_DIV16) {
    prescaler = 16;
  } else if (divider == TMR_PRESC_DIV256) {
    prescaler = 256;
  } else {
    prescaler = 1;
  }
  timer1_isr_init();
  timer_reload = reload;
}

unsigned short CHwTimer::getPrescaler(unsigned long umaxtime) {
  unsigned short divider = TMR_PRESC_DIV1;
  if ((((F_CPU/100000)*umaxtime)/10) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV1;
  } else if ((((F_CPU/100000)*umaxtime)/(16*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV16;
  } else if ((((F_CPU/100000)*umaxtime)/(256*10)) < TIMER1_RESOLUTION) {
    divider = TMR_PRESC_DIV256;
  } 
  return divider;
}

void CHwTimer::attachInterrupt(void (*userFunc)()) {
  timer1_attachInterrupt((timercallback)userFunc);
}

void CHwTimer::detachInterrupt() {
  timer1_detachInterrupt();
}

void CHwTimer::enable() {
  if (!timer_enabled) {
    timer1_enable(prescalerbits, TIM_EDGE, timer_reload);
    timer_enabled = true;
  }
}

void CHwTimer::disable() {
  if (timer_enabled) {
    timer1_disable();
    timer_enabled = false;
  }
}

void CHwTimer::reset() {
}

void CHwTimer::update(unsigned long utime) {
  timer1_write(((F_CPU/100000)*utime)/(prescaler*10) - 1);
}

unsigned long CHwTimer::getuTime() {
  return timer1_read()*(prescaler*10)/(F_CPU/100000);
}

#elif defined(ARDUINO_ARCH_ESP32)

void CHwTimer::init(unsigned short divider, byte reload) {
  prescaler = divider;
  timer = timerBegin(0, prescaler, true);
  timer_reload = reload;
}

unsigned short CHwTimer::getPrescaler(unsigned long umaxtime) {
  unsigned short divider = TMR_PRESC_US;

  if (umaxtime > 0) {
    unsigned long maxMicros = TIMER0_RESOLUTION/TMR_PRESC_US;
    if (umaxtime > (maxMicros*TMR_PRESC_MIN)) {
      divider = (unsigned short)(umaxtime / maxMicros) + 1; // always round up
    } else {
      divider = TMR_PRESC_MIN; // why prescale if you can use the complete resolution, minimum divider = 2
    }
  }

  return divider;
}

void CHwTimer::attachInterrupt(void (*userFunc)()) {
  timerAttachInterrupt(timer, userFunc, true);
}

void CHwTimer::detachInterrupt() {
  timerDetachInterrupt(timer);
}

void CHwTimer::enable() {
    timerAlarmEnable(timer);
    timer_enabled = true;
}

void CHwTimer::disable() {
  if (timer_enabled) {
    timerAlarmDisable(timer);
    timer_enabled = false;
  }
}

void CHwTimer::reset() {
  timerRestart(timer);
}

void CHwTimer::update(unsigned long utime) {
  uint64_t ticks = (((uint64_t)utime)*TMR_PRESC_US/prescaler)-1;
  reset();
  timerAlarmWrite(timer, ticks, timer_reload);
}

unsigned long CHwTimer::getuTime() {
  return (unsigned long)timerReadMicros(timer);
}

#endif

CHwTimer hwtimer;
