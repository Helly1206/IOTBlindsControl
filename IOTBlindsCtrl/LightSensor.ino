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

CLightSensor::rawReadout CLightSensor::raw = {0, false};
portMUX_TYPE CLightSensor::mux = portMUX_INITIALIZER_UNLOCKED;

CLightSensor::CLightSensor() { // constructor
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
  timer = xTimerCreateStatic("sensor", pdMS_TO_TICKS(settings.getShort(settings.SampleTime)), pdTRUE, (void *)0, timerCallback, &timerBuffer);
  xTimerStart(timer, portMAX_DELAY);
  reset();
}

void CLightSensor::handle(void) {
  if (raw.sample) { // interval elapsed
    CalcTwilight();
    CalcSunny();
    portENTER_CRITICAL(&mux);
    raw.sample = false;
    portEXIT_CRITICAL(&mux);
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

unsigned short CLightSensor::getRaw() {
  return raw.value;
}

boolean CLightSensor::getDig() {
#ifdef USE_PIND 
    return !digitalRead(SENSOR_PIND);
#else
    return false;
#endif
}

void CLightSensor::CalcTwilight(void) {
  unsigned short Threshold;
  if (Twilight) { // if already dark, use hysteresis to get light again
    Threshold = settings.getShort(settings.TwilightThreshold) - settings.getShort(settings.TwilightHysterises);
  } else {
    Threshold = settings.getShort(settings.TwilightThreshold);
  }
  // higher number is darker, so positive threshold
  OutTwilight = Integrate(OutTwilight, settings.getShort(settings.TwilightIGain), CompThreshold(raw.value, Threshold, POSITIVE));  
  if (!Twilightd) {
    Twilight = CompThreshold(OutTwilight, settings.getShort(settings.OutputThreshold), POSITIVE);
    if (Twilight) {
        logger.printf(LOG_SENSOR, "Twilight state detected");
      }
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
  OutSunny = Integrate(OutSunny, settings.getShort(settings.SunnyIGain), CompThreshold(raw.value, Threshold, NEGATIVE));  
#ifdef USE_PIND 
  if (digitalRead(SENSOR_PIND) == LOW) { // only check sunny if digital pin is high, otherwise broken sensor might detect as sunny
#else
  if (true) {
#endif
    if (!Sunnyd) {
      Sunny = CompThreshold(OutSunny, settings.getShort(settings.OutputThreshold), POSITIVE);
      if (Sunny) {
        logger.printf(LOG_SENSOR, "Sunny state detected");
      }
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

void CLightSensor::timerCallback(TimerHandle_t xTimer) {
  if (!raw.sample) {
    portENTER_CRITICAL(&mux);    
    raw.value = analogRead(SENSOR_PINA)>>3; // use 10 bit resolution
    raw.sample = true;
    portEXIT_CRITICAL(&mux);
  }
}

CLightSensor lightSensor;
