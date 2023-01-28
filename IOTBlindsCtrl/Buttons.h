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

#ifndef BUTTONUP_PIN
#define BUTTONUP_PIN     -1
#endif
#ifndef BUTTONDN_PIN
#define BUTTONDN_PIN     -1
#endif

#define DEBOUNCE_TIME       50   /* ms */
#define RESET_TIME          3000 /* ms */
#define RESET_COUNTS        (byte)(RESET_TIME/DEBOUNCE_TIME - 1)

//#define BUTTON_HW_DEBUG

#define UP_TIMER            0
#define DOWN_TIMER          1

class CButtons {
public:
  CButtons(); // constructor
  void init(void);
  void handle(void);
  boolean initButtonPressed(void);
private:
  enum buttonstate {idle = 0, debounce, pressed, fired};
  enum buttonaction {none = 0, up, down, both, reset};
  struct isrData {
    buttonstate state;  
    byte count;
  };
  void handleButton();
  buttonaction checkButton();
  void idleButton(buttonaction action);

  // static
  static void timerCallback(TimerHandle_t xTimer);
  static void IRAM_ATTR isr_buttonUp();
  static void IRAM_ATTR isr_buttonDn();
  static portMUX_TYPE upMux;
  static portMUX_TYPE dnMux;

  volatile static isrData upData;
  volatile static isrData dnData;
  static TimerHandle_t upTimer;
  static StaticTimer_t upTimerBuffer;
  static TimerHandle_t dnTimer;
  static StaticTimer_t dnTimerBuffer;
};

extern CButtons buttons;

#endif
