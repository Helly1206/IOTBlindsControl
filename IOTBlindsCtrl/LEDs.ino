/* 
 * IOTBlindCtrl - LEDs
 * Switching the LED(s)
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */ 
 
#include "LEDs.h"

Cled::ledmodes Cled::modes = {Cled::none, Cled::none, Cled::none};
Cled::ledflash Cled::flash = {0, 0, 0, false};
portMUX_TYPE Cled::mux = portMUX_INITIALIZER_UNLOCKED;
bool Cled::timerModeChange = false;

Cled::Cled() {
#ifdef USE_SYSTEM_LED
  initLED(SLED_PIN);
#endif
  initLED(LEDUP_PIN);
  initLED(LEDDN_PIN); 
}

void Cled::init() {
#ifdef USE_SYSTEM_LED
  setLED(SLED_PIN, LOW);
#endif
  setLED(LEDUP_PIN, LOW);
  setLED(LEDDN_PIN, LOW);
  timer = xTimerCreateStatic("leds", pdMS_TO_TICKS(T_TIME3), pdFALSE, (void *)0, timerCallback, &timerBuffer);
}

void Cled::handle(void) {
  String logString = "";
  portENTER_CRITICAL(&mux);
  if (timerModeChange) {
    logString = "Cmd: deqtimer, modes.c: " + String((byte)modes.c) + ", modes.q: " + String((byte)modes.q) + "/ ";
    timerModeChange = false;
  }
  if (modes.c != modes.d) {
    if ((modes.c == none) && (modes.q != none)) {
      modes.c = modes.q;
      modes.q = none;
      logString += "Cmd: deqhandle, modes.c: " + String((byte)modes.c) + ", modes.q: " + String((byte)modes.q) + "/ ";
    }
    switch(modes.c) {
      case up:
        logString += "LED up";
        setLED(LEDUP_PIN, HIGH);
        setLED(LEDDN_PIN, LOW);
#ifdef USE_SYSTEM_LED
        setLED(SLED_PIN, LOW);
#endif
        break;
      case down:
        logString += "LED down";
        setLED(LEDUP_PIN, LOW);
        setLED(LEDDN_PIN, HIGH);
#ifdef USE_SYSTEM_LED
        setLED(SLED_PIN, LOW);
#endif
        break;
      case both:
        logString += "LED both";
        setLED(LEDUP_PIN, HIGH);
        setLED(LEDDN_PIN, HIGH);
#ifdef USE_SYSTEM_LED
        setLED(SLED_PIN, HIGH);
#endif
        break;
      case flsh:
        logString += "LED flash";
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case shrt:
        logString += "LED short";
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case lng:
        logString += "LED long";
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case flsh1:
        logString += "LED flash1";
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case shrt1:
        logString += "LED short1";
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case lng1:
        logString += "LED long1";
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case flsh3:
        logString += "LED flash3";
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      case shrt3:
        logString += "LED short3";
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      case lng3:
        logString += "LED long3";
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      default: //LED_STOP (none)
        logString += "LED Stop";
        setLED(LEDUP_PIN, LOW);
        setLED(LEDDN_PIN, LOW);
#ifdef USE_SYSTEM_LED
        setLED(SLED_PIN, LOW);
#endif
        break;
    }
  }

  modes.d = modes.c;
  portEXIT_CRITICAL(&mux);

  if (logString.length() > 0) {
    logger.printf(LOG_LEDS, logString);
  }
}

void Cled::Up() {
  logger.printf(LOG_LEDS, "Up");
  setMode(up);
}

void Cled::Down() {
  logger.printf(LOG_LEDS, "Down");
  setMode(down);
}

void Cled::Off() {
  logger.printf(LOG_LEDS, "Off");
  setMode(none);
}

void Cled::WifiApC() {
  logger.printf(LOG_LEDS, "WifiApC");
  setMode(lng);
}

void Cled::WifiNC() {
  logger.printf(LOG_LEDS, "WifiNC");
  setMode(shrt);
}

void Cled::WifiC() {
  logger.printf(LOG_LEDS, "WifiC");
  setMode(stop);
}

void Cled::Command() {
  logger.printf(LOG_LEDS, "Command");
  setMode(shrt1);
}

void Cled::ManualCommand() {
  logger.printf(LOG_LEDS, "ManualCommand");
  setMode(lng1);
}

///////////// PRIVATES ///////////////////////////

void Cled::setMode(Cled::ledmode cmd) {
  portENTER_CRITICAL(&mux);
  switch (cmd) {
    case none: // motion modes
    case up:
    case down:
    case both:
    case flsh:
      if (modes.c < shrt) { // motion mode
        modes.c = cmd;
        modes.q = none;
      } else if (modes.c < flsh1) { // wifi mode
        modes.q = modes.c;
        modes.c = cmd;
      } else { // command mode
        modes.q = cmd;
      }
      break;
    case shrt: // wifi modes
    case lng:
      if (((modes.c > flsh) && (modes.c < flsh1)) || (modes.c == none)) { // wifi mode
        modes.c = cmd;
        modes.q = none;
      } else { // motion, command mode
        modes.q = cmd;
      }
      break;
    case stop:
      modes.q = none;
      if ((modes.c > flsh) && (modes.c < flsh1)) { // wifi mode
        modes.c = none;
      }
      break;
    default:  // command modes
      if (modes.c < flsh1) { // motion, wifi mode
        modes.q = modes.c;
        modes.c = cmd;
      } else { // wifi, command mode
        modes.c = cmd;
        modes.q = none;
      }
      break;
  }
  portEXIT_CRITICAL(&mux);
  logger.printf(LOG_LEDS, "Cmd: " + String((byte)cmd) + ", modes.c: " + String((byte)modes.c) + ", modes.q: " + String((byte)modes.q));
}

void Cled::initLED(uint8_t pin) {
#ifdef HIGH_IMPEDANCE_OUTPUTS
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
#else
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
#endif
}

void Cled::setLED(uint8_t pin, uint8_t value) {
#ifdef HIGH_IMPEDANCE_OUTPUTS
  if (value == HIGH) {
    pinMode(pin, OUTPUT);
  } else {
    pinMode(pin, INPUT);
  }
#else
  digitalWrite(pin, value);  
#endif
}

void Cled::timerCallback(TimerHandle_t xTimer) {
  portENTER_CRITICAL(&mux);
  if (modes.c > both) { // flashing
    if (flash.repeatCount > 0) {
      if (flash.isOn) {
        flashOff(xTimer);
      } else {
        flashOn(xTimer);
      }
    } else {
      modes.c = modes.q;
      modes.q = none;
      flash.isOn = false;
      timerModeChange = true;
    }
  }
  portEXIT_CRITICAL(&mux);
}

void Cled::flashOn(TimerHandle_t xTimer) {
  setLED(LEDUP_PIN, HIGH);
  setLED(LEDDN_PIN, HIGH);
#ifdef USE_SYSTEM_LED
  setLED(SLED_PIN, HIGH);
#endif
  if ((flash.repeatCount > 0) && (flash.repeatCount < 255)) {
    flash.repeatCount--;
  }
  xTimerChangePeriod(xTimer, pdMS_TO_TICKS(flash.t1), portMAX_DELAY);
  flash.isOn = true; 
}

void Cled::flashOff(TimerHandle_t xTimer) {
  setLED(LEDUP_PIN, LOW);
  setLED(LEDDN_PIN, LOW);
#ifdef USE_SYSTEM_LED
  setLED(SLED_PIN, LOW);
#endif
  xTimerChangePeriod(xTimer, pdMS_TO_TICKS(flash.t2), portMAX_DELAY);
  flash.isOn = false;
}

Cled LED;
