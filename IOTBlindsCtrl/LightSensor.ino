/* 
 * IOTBlindCtrl - Lightsensor
 * Reads and and processes analog LightSensor value
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */ 

/*
 * Original: LightSensor
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 */
 
#include "LightSensor.h"
#include "Timers.h"
#include "Settings.h"

CLightSensor::CLightSensor() { // constructor
  Raw = 0;
  OutSunny = 0;
  OutTwilight = 0;
  Twilight = false;
  Sunny = false;
  Twilightd = false;
  Sunnyd = false;
#ifdef USE_PIND
  //pinMode(SENSOR_PIND, FUNCTION_3); // not required
  pinMode(SENSOR_PIND, INPUT);
#endif
}

void CLightSensor::init(void) {
  timers.start(TIMER_SENSOR, settings.getShort(settings.SampleTime), true);
  reset();
}

void CLightSensor::handle(void) {
  if (timers.getTimer(TIMER_SENSOR)) { // interval elapsed
    Raw = analogRead(SENSOR_PINA)>>3; // use 10 bit resolution
    CalcTwilight();
    CalcSunny();
  }
}

void CLightSensor::reset(void) {
  OutSunny = 0;
  Sunny = false;
  OutTwilight = 0;
  Twilight = false;
  Sunnyd = false;
  Twilightd = false;
}

// set integrators after switching to dark/ light (time hysteresis)
void CLightSensor::setTwilight(boolean bval) {
  if (bval) {
    OutTwilight = 0xFFFF;
  } else {
    OutTwilight = 0x0000;
  }
} 

void CLightSensor::setSunny(boolean bval) {
  if (bval) {
    OutSunny = 0xFFFF;
  } else {
    OutSunny = 0x0000;
  }
}

void CLightSensor::CalcTwilight(void) {
  unsigned short Threshold;
  if (Twilight) { // if already dark, use hysteresis to get light again
    Threshold = settings.getShort(settings.TwilightThreshold) - settings.getShort(settings.TwilightHysterises);
  } else {
    Threshold = settings.getShort(settings.TwilightThreshold);
  }
  // higher number is darker, so positive threshold
  OutTwilight = Integrate(OutTwilight, settings.getShort(settings.TwilightIGain), CompThreshold(Raw, Threshold, POSITIVE));  
  if (!Twilightd) {
    Twilight = CompThreshold(OutTwilight, settings.getShort(settings.OutputThreshold), POSITIVE);
    Twilightd = Twilight;
  } else { // after manual interrupt, twilight won't work until light first
    Twilightd = CompThreshold(OutTwilight, settings.getShort(settings.OutputThreshold), POSITIVE);
  }  
}

void CLightSensor::CalcSunny(void) {
  unsigned short Threshold;
  if (Sunny) { // if already sunny, use hysteresis to get darker again
    Threshold = settings.getShort(settings.SunnyThreshold) - settings.getShort(settings.SunnyHysterises);
  } else {
    Threshold = settings.getShort(settings.SunnyThreshold);
  }
  // lower number is lighter, so negative threshold
  OutSunny = Integrate(OutSunny, settings.getShort(settings.SunnyIGain), CompThreshold(Raw, Threshold, NEGATIVE));  
 #ifdef USE_PIND 
  if (SENSOR_PIND) { // only check sunny if digital pin is high, otherwise broken sensor might detect as sunny
 #else
  if (true) {
 #endif
    if (!Sunnyd) {
      Sunny = CompThreshold(OutSunny, settings.getShort(settings.OutputThreshold), POSITIVE);
      Sunnyd = Sunny;
    } else { // after manual interrupt, sunny won't work until dark first
      Sunnyd = CompThreshold(OutSunny, settings.getShort(settings.OutputThreshold), POSITIVE);
    }
  } else {
    Sunny = false;
    Sunnyd = Sunny;
  }    
}

unsigned short CLightSensor::Integrate(unsigned short value, unsigned short gain, boolean sign) {
  unsigned short Result=0;
  
  if (sign) { // positive
    Result = value + gain;
    if (Result < value) { // wrap around 16 bit
      Result = 0xFFFF;
    }     
  } else { // negative
    Result = value - gain;
    if (Result > value) { // wrap around 16 bit
      Result = 0x0000;
    }
  }
   
  return (Result);
}

boolean CLightSensor::CompThreshold(unsigned short value, unsigned short threshold, boolean sign) {
  boolean Result = (value > threshold);
  
  if (!sign) { // negative
    Result = !Result;
  }
  return (Result);
}

CLightSensor lightSensor;
