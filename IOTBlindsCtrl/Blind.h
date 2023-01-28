/* 
 * IOTBlindCtrl - Blind
 * Blind actuation and position calculation
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Blind_h
#define Blind_h

#ifndef BLINDUPDN_PIN
#define BLINDUPDN_PIN   -1
#endif
#ifndef BLINDONOFF_PIN
#define BLINDONOFF_PIN  -1
#endif

#define BLIND_POSUP     100
#define BLIND_POSDOWN   0

#define RELAY_DELAY_TIME 100000   /* us */
#define MIN_TIME         1000     /* us */
#define MAX_TIME         65535000 /* us */

class CBlind {
public:
  enum blinddir {none, up, down};
  CBlind(); // constructor
  void init();
  void handle();
  byte getPosition();
  byte getSetPoint();
  boolean getPositionDown();
  blinddir moveDir(blinddir Dir);
  blinddir movePos(byte Pos);
  blinddir stop();
  boolean isMoving();
  void setBlindEnabled(boolean enable);
  boolean getBlindEnabled();
private:
  enum blindstate {stopped, starting, moving, stopping, syncing};
  void StartMove();
  unsigned long GetLength();
  void CalcPos(boolean endPos);
  byte Startpos;
  byte Position;
  byte Setpoint;
  boolean blindEnabled;
  blinddir blindDir;
  static void IRAM_ATTR isr_timer();
  static portMUX_TYPE timerMux;
  volatile static blindstate blindState;
  volatile static unsigned long moveTime;
  volatile static unsigned long syncTime;
  volatile static unsigned long stoppedTime;
};

extern CBlind blind;

#endif
