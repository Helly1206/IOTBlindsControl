/* 
 * IOTBlindCtrl - LEDs
 * Switching the LED(s)
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */ 
 
#include "LEDs.h"

Cled::Cled() {
  initLED(SLED_PIN);
  initLED(LEDUP_PIN);
  initLED(LEDDN_PIN); 
}

void Cled::init() {
  setLED(SLED_PIN, LOW);
  setLED(LEDUP_PIN, LOW);
  setLED(LEDDN_PIN, LOW);
  flashRunning = false;
  t1 = 0;
  t2 = 0;
  mode_q = none;
  mode_d = none;
  mode = none;
  repeatCount = 0;
  ledflash = false;
}

void Cled::handle(void) {

  if (mode != mode_d) {
    if (mode == none) {
      mode = mode_q;
      mode_q = none;
#ifdef DEBUG_LED
      Serial.print("DEBUG: cmd: +, mode: " + String(mode) + ", mode_q: " + String(mode_q) + "\n");
#endif
    }
    switch(mode) {
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
      case flash:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash\n");
#endif
        t1 = T_TIME3;
        t2 = T_TIME3;
        repeatCount = 255;
        break;
      case shrt:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short\n");
#endif
        t1 = T_TIME2;
        t2 = T_TIME1;
        repeatCount = 255;
        break;
      case lng:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long\n");
#endif
        t1 = T_TIME1;
        t2 = T_TIME2;
        repeatCount = 255;
        break;
      case flash1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash1\n");
#endif
        t1 = T_TIME3;
        t2 = T_TIME3;
        repeatCount = 1;
        break;
      case shrt1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short1\n");
#endif
        t1 = T_TIME2;
        t2 = T_TIME1;
        repeatCount = 1;
        break;
      case lng1:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long1\n");
#endif
        t1 = T_TIME1;
        t2 = T_TIME2;
        repeatCount = 1;
        break;
      case flash3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED flash3\n");
#endif
        t1 = T_TIME3;
        t2 = T_TIME3;
        repeatCount = 3;
        break;
      case shrt3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED short3\n");
#endif
        t1 = T_TIME2;
        t2 = T_TIME1;
        repeatCount = 3;
        break;
      case lng3:
#ifdef DEBUG_LED
        Serial.print("DEBUG: LED long3\n");
#endif
        t1 = T_TIME1;
        t2 = T_TIME2;
        repeatCount = 3;
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

  mode_d = mode;

  if (mode > both) { // flashing
    if (!flashRunning) {
      ledflash = true;
    }
    if ((ledflash) && ((!flashRunning) || (timers.getTimer(TIMER_LEDS)))) {
      if (repeatCount > 0) {
        setLED(LEDUP_PIN, HIGH);
        setLED(LEDDN_PIN, HIGH);
        setLED(SLED_PIN, HIGH);
      } else {
        mode = mode_q;
        mode_q = none;
#ifdef DEBUG_LED
        Serial.print("DEBUG: cmd: -, mode: " + String(mode) + ", mode_q: " + String(mode_q) + "\n");
#endif
      }
      if ((repeatCount > 0) && (repeatCount < 255)) {
        repeatCount--;
      }
      timers.start(TIMER_LEDS, t1, false);
      ledflash = false;
      flashRunning = true;
    } else if ((!ledflash) && (timers.getTimer(TIMER_LEDS))) {
      if (repeatCount > 0) {
        setLED(LEDUP_PIN, LOW);
        setLED(LEDDN_PIN, LOW);
        setLED(SLED_PIN, LOW);
      } else {
        mode = mode_q;
        mode_q = none;
#ifdef DEBUG_LED
        Serial.print("DEBUG: cmd: -, mode: " + String(mode) + ", mode_q: " + String(mode_q) + "\n");
#endif
      }
      timers.start(TIMER_LEDS, t2, false);
      ledflash = true;
      flashRunning = true;
    }
  } else {
    flashRunning = false;
  }
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
    case flash:
      if (mode < shrt) { // motion mode
        mode = cmd;
        mode_q = none;
      } else if (mode < flash1) { // wifi mode
        mode_q = mode;
        mode = cmd;
      } else { // command mode
        mode_q = cmd;
      }
      break;
    case shrt: // wifi modes
    case lng:
      if (((mode > flash) && (mode < flash1)) || (mode == none)) { // wifi mode
        mode = cmd;
        mode_q = none;
      } else { // motion, command mode
        mode_q = cmd;
      }
      break;
    case stop:
      mode_q = none;
      if ((mode > flash) && (mode < flash1)) { // wifi mode
        mode = none;
      }
      break;
    default:  // command modes
      if (mode < flash1) { // motion, wifi mode
        mode_q = mode;
        mode = cmd;
      } else { // wifi, command mode
        mode = cmd;
        mode_q = none;
      }
      break;
  }
#ifdef DEBUG_LED
  Serial.print("DEBUG: cmd: " + String(cmd) + ", mode: " + String(mode) + ", mode_q: " + String(mode_q) + "\n");
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

Cled LED;
