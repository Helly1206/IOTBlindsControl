/* 
 * IOTBlindCtrl - Timers
 * Timers class
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

/* 
 * Original: Very simple Timers Class
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 */ 
 
#include "Timers.h"
 
CTimers::CTimers() { // constructor

}

void CTimers::init(void) {
  int i;
  for (i=0; i<MAXTIMERS; i++) {
    ClearTimer(i);
  }
}

void CTimers::start(byte timerno, unsigned long Interval, bool Repeat) {
	MyTimers[timerno].Repeat = Repeat;
	MyTimers[timerno].Interval = Interval;
	MyTimers[timerno].LastTime = millis();
}

void CTimers::restart(byte timerno) {
	MyTimers[timerno].LastTime = millis();
} 

void CTimers::stop(byte timerno) {
	ClearTimer(timerno);
}

unsigned long CTimers::getElapsed(byte timerno) {
  return(CalcElapsed(timerno));
}

boolean CTimers::getTimer(byte timerno) {
  if (MyTimers[timerno].Interval == 0) {
    return (false);
  }
  
  unsigned long Elapsed = CalcElapsed(timerno);
	
  if (Elapsed > MyTimers[timerno].Interval) {
    if (MyTimers[timerno].Repeat) { // arm for next one
      if (Elapsed < 2*MyTimers[timerno].Interval) { // calc next interval
        MyTimers[timerno].LastTime += MyTimers[timerno].Interval;
      } else { // automatic reset
	      restart(timerno);
      }
    } else { // only one shot --> clear
      ClearTimer(timerno);
    }
    return(true);
  } else {
    return(false);
  }
}

boolean CTimers::running(byte timerno) {
  return (MyTimers[timerno].Interval > 0);
}

///////////// PRIVATES ///////////////////////////
 
void CTimers::ClearTimer(byte timerno) {
	MyTimers[timerno].Repeat = false;
	MyTimers[timerno].Interval = 0;
	MyTimers[timerno].LastTime = 0;
}

unsigned long CTimers::CalcElapsed(byte timerno) {
  unsigned long retval = 0;
  
  if (MyTimers[timerno].Interval > 0) { // not available
    retval = CalcNewElapsed(MyTimers[timerno].LastTime);
  }
  return(retval);
}

unsigned long CTimers::CalcNewElapsed(unsigned long LastTime) {
  return (millis()-LastTime);
}

CTimers timers;
