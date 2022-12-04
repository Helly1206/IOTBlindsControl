/* 
 * IOTBlindCtrl - Buttons
 * Up/ Down button Control
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Buttons_h
#define Buttons_h

#ifdef DEBUG_SERIAL
//#define DEBUG_BUTTONS
#endif

#ifndef BUTTONUP_PIN
#define BUTTONUP_PIN     -1
#endif
#ifndef BUTTONDN_PIN
#define BUTTONDN_PIN     -1
#endif

class CButtons {
public:
  CButtons(); // constructor
  void init(void);
  void handle(void);
  boolean initButtonPressed(void);
private:
  enum buttonstate {idle, up, down, both, reset, upd, downd, bothd};
  void handleButton();
  buttonstate checkButton();
  static void IRAM_ATTR isr_buttons();
  static portMUX_TYPE isrMux;
  volatile static buttonstate buttonState;
};

extern CButtons buttons;

#endif
