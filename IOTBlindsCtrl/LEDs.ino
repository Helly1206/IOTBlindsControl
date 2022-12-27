/* 
 * IOTBlindCtrl - LEDs
 * Switching the LED(s)
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */ 
 
#include "LEDs.h"

Cled::ledmodes Cled::modes;
Cled::ledflash Cled::flash;
portMUX_TYPE Cled::mux = portMUX_INITIALIZER_UNLOCKED;

Cled::Cled() {
  initLED(SLED_PIN);
  initLED(LEDUP_PIN);
  initLED(LEDDN_PIN); 
}

void Cled::init() {
  setLED(SLED_PIN, LOW);
  setLED(LEDUP_PIN, LOW);
  setLED(LEDDN_PIN, LOW);
  flash.repeatCount = 0;
  flash.t1 = 0;
  flash.t2 = 0;
  flash.isOn = false;
  modes.q = none;
  modes.d = none;
  modes.c = none;
  timer = xTimerCreateStatic("", pdMS_TO_TICKS(T_TIME3), pdFALSE, (void *)0, timerCallback, &timerBuffer);
}

void Cled::handle(void) {
  portENTER_CRITICAL(&mux);
  if (modes.c != modes.d) {
    if (modes.c == none) {
      modes.c = modes.q;
      modes.q = none;
#ifdef DEBUG_LED
      Serial.print("DEBUG: cmd: +, modes.c: " + String(modes.c) + ", modes.q: " + String(modes.q) + "\n");
#endif
    }
    switch(modes.c) {
      case up:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED up\n");
#endif
        setLED(LEDUP_PIN, HIGH);
        setLED(LEDDN_PIN, LOW);
        setLED(SLED_PIN, LOW);
        break;
      case down:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED down\n");
#endif
        setLED(LEDUP_PIN, LOW);
        setLED(LEDDN_PIN, HIGH);
        setLED(SLED_PIN, LOW);
        break;
      case both:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED both\n");
#endif
        setLED(LEDUP_PIN, HIGH);
        setLED(LEDDN_PIN, HIGH);
        setLED(SLED_PIN, HIGH);
        break;
      case flsh:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash\n");
#endif
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case shrt:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short\n");
#endif
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case lng:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long\n");
#endif
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 255;
        flashOn(timer);
        break;
      case flsh1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash1\n");
#endif
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case shrt1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short1\n");
#endif
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case lng1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long1\n");
#endif
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 1;
        flashOn(timer);
        break;
      case flsh3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash3\n");
#endif
        flash.t1 = T_TIME3;
        flash.t2 = T_TIME3;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      case shrt3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short3\n");
#endif
        flash.t1 = T_TIME2;
        flash.t2 = T_TIME1;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      case lng3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long3\n");
#endif
        flash.t1 = T_TIME1;
        flash.t2 = T_TIME2;
        flash.repeatCount = 3;
        flashOn(timer);
        break;
      default: //LED_STOP (none)
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED Stop\n");
#endif
        setLED(LEDUP_PIN, LOW);
        setLED(LEDDN_PIN, LOW);
        setLED(SLED_PIN, LOW);
        break;
    }
  }

  modes.d = modes.c;
  portEXIT_CRITICAL(&mux);
}

void Cled::Up() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::Up()\n");
#endif
  setMode(up);
}

void Cled::Down() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::Down()\n");
#endif
  setMode(down);
}

void Cled::Off() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::Off()\n");
#endif
  setMode(none);
}

void Cled::WifiApC() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::WifiApC()\n");
#endif
  setMode(lng);
}

void Cled::WifiNC() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::WifiNC()\n");
#endif
  setMode(shrt);
}

void Cled::WifiC() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::WifiC()\n");
#endif
  setMode(stop);
}

void Cled::Command() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::Command()\n");
#endif
  setMode(shrt1);
}

void Cled::ManualCommand() {
#ifdef DEBUG_LED
  Serial.print("DEBUG: LED::ManualCommand()\n");
#endif
  setMode(lng1);
}

///////////// PRIVATES ///////////////////////////

void Cled::setMode(Cled::ledmode cmd) {
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
#ifdef DEBUG_LED
  Serial.print("DEBUG: cmd: " + String(cmd) + ", modes.c: " + String(modes.c) + ", modes.q: " + String(modes.q) + "\n");
#endif
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
#ifdef DEBUG_LED
      Serial.print("DEBUG: cmd: -, modes.c: " + String(modes.c) + ", modes.q: " + String(modes.q) + "\n");
#endif
    }
  }
  portEXIT_CRITICAL(&mux);
}

void Cled::flashOn(TimerHandle_t xTimer) {
  setLED(LEDUP_PIN, HIGH);
  setLED(LEDDN_PIN, HIGH);
  setLED(SLED_PIN, HIGH);
  if ((flash.repeatCount > 0) && (flash.repeatCount < 255)) {
    flash.repeatCount--;
  }
  xTimerChangePeriod(xTimer, pdMS_TO_TICKS(flash.t1), portMAX_DELAY);
  flash.isOn = true; 
}

void Cled::flashOff(TimerHandle_t xTimer) {
  setLED(LEDUP_PIN, LOW);
  setLED(LEDDN_PIN, LOW);
  setLED(SLED_PIN, LOW);
  xTimerChangePeriod(xTimer, pdMS_TO_TICKS(flash.t2), portMAX_DELAY);
  flash.isOn = false;
}

Cled LED;
