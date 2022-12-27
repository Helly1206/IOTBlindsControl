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
#include "Commands.h"

volatile CButtons::buttonstate CButtons::buttonState = idle;
TimerHandle_t CButtons::timer;
portMUX_TYPE CButtons::isrMux = portMUX_INITIALIZER_UNLOCKED;

CButtons::CButtons() { // constructor
  pinMode(BUTTONUP_PIN, INPUT);
  pinMode(BUTTONDN_PIN, INPUT);
}

void CButtons::init(void) {
  timer = xTimerCreateStatic("", pdMS_TO_TICKS(RESET_TIME), pdFALSE, (void *)0, timerCallback, &timerBuffer);
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
  boolean bUp = digitalRead(BUTTONUP_PIN);
  boolean bDown = digitalRead(BUTTONDN_PIN);
  unsigned long elapsed = RESET_TIME - pdTICKS_TO_MS(xTimerGetExpiryTime(timer) - xTaskGetTickCount());
  switch (buttonState) {
    case upd:
      if (!bUp) {
        portENTER_CRITICAL_ISR(&isrMux);
        if (elapsed > DEBOUNCE_TIME) {
          buttonState = up;
        } else {
          buttonState = idle;
        }
        portEXIT_CRITICAL_ISR(&isrMux);
      } else if (bDown) {
        BaseType_t higher_task_woken = pdFALSE;
        if (xTimerStartFromISR(timer, &higher_task_woken) == pdPASS) {
          portENTER_CRITICAL_ISR(&isrMux);
          buttonState = bothd;
          portEXIT_CRITICAL_ISR(&isrMux);
        } else {
          portENTER_CRITICAL_ISR(&isrMux);
          buttonState = idle;
          portEXIT_CRITICAL_ISR(&isrMux);
        }
        if (higher_task_woken) {
          portYIELD_FROM_ISR();
        }
      } else {
        portENTER_CRITICAL_ISR(&isrMux);
        buttonState = idle;
        portEXIT_CRITICAL_ISR(&isrMux);
      }
      break;
    case downd:
      if (!bDown) {
        portENTER_CRITICAL_ISR(&isrMux);
        if (elapsed > DEBOUNCE_TIME) {
          buttonState = down;
        } else {
          buttonState = idle;
        }
        portEXIT_CRITICAL_ISR(&isrMux);
      } else if (bUp) {
        BaseType_t higher_task_woken = pdFALSE;
        if (xTimerStartFromISR(timer, &higher_task_woken) == pdPASS) {
          portENTER_CRITICAL_ISR(&isrMux);
          buttonState = bothd;
          portEXIT_CRITICAL_ISR(&isrMux);
        } else {
          portENTER_CRITICAL_ISR(&isrMux);
          buttonState = idle;
          portEXIT_CRITICAL_ISR(&isrMux);
        }
        if (higher_task_woken) {
          portYIELD_FROM_ISR();
        }
      } else {
        portENTER_CRITICAL_ISR(&isrMux);
        buttonState = idle;
        portEXIT_CRITICAL_ISR(&isrMux);
      }
      break;
    case bothd:
      if (!bDown & !bUp) {
        portENTER_CRITICAL_ISR(&isrMux);
        if (xTimerIsTimerActive(timer) == pdFALSE) {
          buttonState = reset;
        } else if (elapsed > DEBOUNCE_TIME) {
          buttonState = both;
        } else {
          buttonState = idle;
        }
        portEXIT_CRITICAL_ISR(&isrMux);
      } else if (bDown & bUp) {
        portENTER_CRITICAL_ISR(&isrMux);
        buttonState = idle;
        portEXIT_CRITICAL_ISR(&isrMux);
      }
      break;
    default:
      if (bUp|bDown) {
        BaseType_t higher_task_woken = pdFALSE;
        if (xTimerStartFromISR(timer, &higher_task_woken) == pdPASS) {
          portENTER_CRITICAL_ISR(&isrMux);
          if (bUp & bDown) {
            buttonState = bothd;
          } else if (bUp) {
            buttonState = upd;
          } else if (bDown) {
            buttonState = downd;
          }
          portEXIT_CRITICAL_ISR(&isrMux);
        }
        if (higher_task_woken) {
          portYIELD_FROM_ISR();
        }
      } 
      break;
  }
}

void CButtons::timerCallback(TimerHandle_t xTimer) {
}

CButtons buttons;
