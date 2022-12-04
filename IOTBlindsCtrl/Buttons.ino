/* 
 * IOTBlindCtrl - Buttons
 * Up/ Down button Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

/* 
 * Original: Up/ Down button Control
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 */ 
 
#include "Buttons.h"
#include "Settings.h"
#include "Blind.h"
#include "Timers.h"
#include "Commands.h"

volatile CButtons::buttonstate CButtons::buttonState = idle;
portMUX_TYPE CButtons::isrMux = portMUX_INITIALIZER_UNLOCKED;

CButtons::CButtons() { // constructor
  pinMode(BUTTONUP_PIN, INPUT);
  pinMode(BUTTONDN_PIN, INPUT);
}

void CButtons::init(void) {
  if ((digitalRead(BUTTONDN_PIN)) || (digitalRead(BUTTONUP_PIN))) {
    blind.setBlindEnabled(false); // do not move blinds when in test mode
  }
  attachInterrupt(BUTTONUP_PIN, isr_buttons, CHANGE);
  attachInterrupt(BUTTONDN_PIN, isr_buttons, CHANGE);
}

void CButtons::handle(void) {
  if (!iotWifi.wakingUp()) {
    handleButton();
  }
}

boolean CButtons::initButtonPressed(void) {
  return checkButton() == up;
}

// privates ...

void CButtons::handleButton() {
  switch (checkButton()) {
    case up:
#ifdef DEBUG_BUTTONS
      Serial.println("button up");
#endif
      cmdQueue.addCommand(CMD_UP);
      break;
    case down:
#ifdef DEBUG_BUTTONS
      Serial.println("button down");
#endif
      cmdQueue.addCommand(CMD_DOWN);
      break;
    case both:
#ifdef DEBUG_BUTTONS
      Serial.println("button both");
#endif
      cmdQueue.addCommand(CMD_MANUAL);
      break;
    case reset:
#ifdef DEBUG_BUTTONS
      Serial.println("reset");
#endif
      ESP.restart();
      break;
  }
}

CButtons::buttonstate CButtons::checkButton() {
  buttonstate state = idle;

  portENTER_CRITICAL(&isrMux);
  if ((buttonState == up) || (buttonState == down) || (buttonState == both) || (buttonState == reset)) {
    state = buttonState;
    buttonState = idle;
  }
  portEXIT_CRITICAL(&isrMux);

  return state;
}

void IRAM_ATTR CButtons::isr_buttons() {
  portENTER_CRITICAL_ISR(&isrMux);
  boolean bUp = digitalRead(BUTTONUP_PIN);
  boolean bDown = digitalRead(BUTTONDN_PIN);
  switch (buttonState) {
    case idle:
      if (bUp|bDown) {
        timers.start(TIMER_DEBOUNCE, DEBOUNCE_TIME, false);
      } 
      if (bUp & bDown) {
        timers.start(TIMER_RESETBTN, RESET_TIME, false);
        buttonState = bothd;
      } else if (bUp) {
        buttonState = upd;
      } else if (bDown) {
        buttonState = downd;
      }
      break;
    case upd:
      if (!bUp) {
        if (timers.getTimer(TIMER_DEBOUNCE)) {
          buttonState = up;
        } else {
          buttonState = idle;
        }
      } else if (bDown) {
        timers.start(TIMER_DEBOUNCE, DEBOUNCE_TIME, false);
        timers.start(TIMER_RESETBTN, RESET_TIME, false);
        buttonState = bothd;
      }
      break;
    case downd:
      if (!bDown) {
        if (timers.getTimer(TIMER_DEBOUNCE)) {
          buttonState = down;
        } else {
          buttonState = idle;
        }
      } else if (bUp) {
        timers.start(TIMER_DEBOUNCE, DEBOUNCE_TIME, false);
        timers.start(TIMER_RESETBTN, RESET_TIME, false);
        buttonState = bothd;
      }
      break;
    case bothd:
      if (!bDown & !bUp) {
        if (timers.getTimer(TIMER_RESETBTN)) {
          buttonState = reset;
        } else if (timers.getTimer(TIMER_DEBOUNCE)) {
          buttonState = both;
        } else {
          buttonState = idle;
        }
      }
      break;
    default:
      break;
  }
  portEXIT_CRITICAL_ISR(&isrMux);
}

CButtons buttons;
