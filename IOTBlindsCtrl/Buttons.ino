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

StaticTimer_t CButtons::upTimerBuffer;
TimerHandle_t CButtons::upTimer; // = NULL;
StaticTimer_t CButtons::dnTimerBuffer;
TimerHandle_t CButtons::dnTimer; // = NULL;
volatile CButtons::isrData CButtons::upData = {CButtons::idle, 0};
volatile CButtons::isrData CButtons::dnData = {CButtons::idle, 0};
portMUX_TYPE CButtons::upMux = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE CButtons::dnMux = portMUX_INITIALIZER_UNLOCKED;


CButtons::CButtons() { // constructor
  pinMode(BUTTONUP_PIN, INPUT); //4k7 pull-down, change to INPUT_PULLDOWN if no resistor
  pinMode(BUTTONDN_PIN, INPUT); //4k7 pull-down, change to INPUT_PULLDOWN if no resistor
}

void CButtons::init(void) {
  upTimer = xTimerCreateStatic("upbutton", pdMS_TO_TICKS(DEBOUNCE_TIME), pdFALSE, (void *)UP_TIMER, timerCallback, &upTimerBuffer);
  dnTimer = xTimerCreateStatic("dnbutton", pdMS_TO_TICKS(DEBOUNCE_TIME), pdFALSE, (void *)DOWN_TIMER, timerCallback, &dnTimerBuffer);
  if ((digitalRead(BUTTONDN_PIN)) || (digitalRead(BUTTONUP_PIN))) {
    blind.setBlindEnabled(false); // do not move blinds when in test mode
  }
  attachInterrupt(digitalPinToInterrupt(BUTTONUP_PIN), isr_buttonUp, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTONDN_PIN), isr_buttonDn, RISING);  
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
      logger.printf(LOG_BUTTONS, "Button up");
      stateMachine.setCmd(CMD_UP);
      break;
    case down:
      logger.printf(LOG_BUTTONS, "Button down");
      stateMachine.setCmd(CMD_DOWN);
      break;
    case both:
      logger.printf(LOG_BUTTONS, "Button both");
      stateMachine.setCmd(CMD_MANUAL);
      break;
    case reset:
      logger.printf("System reset");
      ESP.restart();
      break;
  }
}

CButtons::buttonaction CButtons::checkButton() {
  buttonaction action = none;
  isrData upCopy = {idle, 0};
  isrData dnCopy = {idle, 0};

#ifdef BUTTON_HW_DEBUG
  String logString = "";
#endif
  portENTER_CRITICAL(&upMux);
  if (upData.state != idle) {
#ifdef BUTTON_HW_DEBUG
    logString = "Up, state: " + String(upData.state) + ", count: " + String(upData.count);
#endif
    memcpy((void*)&upCopy, (void*)&upData, sizeof(isrData));
  }
  portEXIT_CRITICAL(&upMux);
#ifdef BUTTON_HW_DEBUG
  if (!logString.isEmpty()) {
    logger.printf(LOG_BUTTONS, logString);  
  }
  logString = "";
#endif
  portENTER_CRITICAL(&dnMux);
  if (dnData.state != idle) {
#ifdef BUTTON_HW_DEBUG
    logString = "Down, state: " + String(dnData.state) + ", count: " + String(dnData.count);
#endif
    memcpy((void*)&dnCopy, (void*)&dnData, sizeof(isrData));
  }
  portEXIT_CRITICAL(&dnMux);
#ifdef BUTTON_HW_DEBUG
  if (!logString.isEmpty()) {
    logger.printf(LOG_BUTTONS, logString);  
  }
#endif
  if (upCopy.state == fired) {
    if (dnCopy.state > debounce) { // both
      if (upCopy.count > RESET_COUNTS) {
        action = reset;
      } else {
        action = both;
      }
    } else { // up
      action = up;
    }
  } else if (dnCopy.state == fired) {
    if (upCopy.state > debounce) { // both
      if (dnCopy.count > RESET_COUNTS) {
        action = reset;
      } else {
        action = both;
      }
    } else { // dn
      action = down;
    }
  }

  idleButton(action);

  return action;
}

void CButtons::idleButton(buttonaction action) {
  if (action != none) { // if idle do nothing
#ifdef BUTTON_HW_DEBUG
    logger.printf(LOG_BUTTONS, "Action: " + String(action));
#endif
    if (action != down) { // up, both, reset
      portENTER_CRITICAL(&upMux);
      upData.state = idle;
      portEXIT_CRITICAL(&upMux);
    }
    if (action != up) { // down, both, reset
      portENTER_CRITICAL(&dnMux);
      dnData.state = idle;
      portEXIT_CRITICAL(&dnMux);
    }
  }
}

void IRAM_ATTR CButtons::isr_buttonUp() {
  portENTER_CRITICAL_ISR(&upMux);
  if (upData.state == idle) {
    if (xTimerStartFromISR(upTimer, NULL) == pdPASS) {
      upData.state = debounce;
      upData.count = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&upMux);
}

void IRAM_ATTR CButtons::isr_buttonDn() {
  portENTER_CRITICAL_ISR(&dnMux);
  if (dnData.state == idle) {
    if (xTimerStartFromISR(dnTimer, NULL) == pdPASS) {
      dnData.state = debounce;
      dnData.count = 0;
    }
  }
  portEXIT_CRITICAL_ISR(&dnMux);
}

void CButtons::timerCallback(TimerHandle_t xTimer) {
  if ((uint32_t)pvTimerGetTimerID(xTimer) == UP_TIMER) {
    if (digitalRead(BUTTONUP_PIN) == LOW) {
      portENTER_CRITICAL(&upMux);
      if (!upData.count) {
        upData.state = idle;  
      } else if (upData.state != idle) {
        upData.state = fired; 
        upData.count++;
      }
      portEXIT_CRITICAL(&upMux);      
    } else if (upData.state != idle) {
      portENTER_CRITICAL(&upMux);
      upData.state = pressed;
      upData.count++;
      portEXIT_CRITICAL(&upMux);
      xTimerStart(xTimer, portMAX_DELAY);
    }   
  } else { // DOWN_TIMER
    if (digitalRead(BUTTONDN_PIN) == LOW) {
      portENTER_CRITICAL(&dnMux);
      if (!dnData.count) {
        dnData.state = idle;  
      } else if (dnData.state != idle) {
        dnData.state = fired; 
        dnData.count++;
      }
      portEXIT_CRITICAL(&dnMux);      
    } else if (dnData.state != idle) {
      portENTER_CRITICAL(&dnMux);
      dnData.state = pressed;
      dnData.count++;
      portEXIT_CRITICAL(&dnMux);
      xTimerStart(xTimer, portMAX_DELAY);
    }
  }
}

CButtons buttons;
