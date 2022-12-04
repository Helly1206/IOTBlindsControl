/* 
 * IOTBlindCtrl - LEDs
 * Switching the LED(s)
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef IOTLEDs_h
#define IOTLEDs_h

#ifdef DEBUG_SERIAL
//#define DEBUG_LED
#endif

#ifndef SLED_PIN
#define SLED_PIN    -1
#endif
#ifndef LEDUP_PIN
#define LEDUP_PIN  -1
#endif
#ifndef LEDDN_PIN
#define LEDDN_PIN  -1
#endif

#define T_TIME1     (900)
#define T_TIME2     (100)
#define T_TIME3     (500)

#define HIGH_IMPEDANCE_OUTPUTS

class Cled {
public:
  Cled();
  void init();
  void handle(void);
  void Up();
  void Down();
  void Off();
  void WifiApC();
  void WifiNC();
  void WifiC();
  void Command();
  void ManualCommand();
private:
  enum ledmode {none   = 0, 
                up     = 1,
                down   = 2,
                both   = 3, 
                flash  = 4,   // 0100
                shrt   = 5,   // 0101
                lng    = 6,   // 0110
                stop   = 7,   // 0111
                flash1 = 8,   // 1000
                shrt1  = 9,   // 1001
                lng1   = 10,  // 1010
                flash3 = 12,  // 1100
                shrt3  = 13,  // 1101
                lng3   = 14}; // 1110
  ledmode mode;
  ledmode mode_q;
  ledmode mode_d;
  byte repeatCount;
  boolean flashRunning;
  unsigned long t1;
  unsigned long t2;
  boolean ledflash;
  void setMode(ledmode cmd);
  void initLED(uint8_t pin);
  void setLED(uint8_t pin, uint8_t value);
};

extern Cled LED;

#endif
