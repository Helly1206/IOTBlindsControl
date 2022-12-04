/* 
 * IOTBlindCtrl - StateMachine
 * State Machine
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef StateMachine_h
#define StateMachine_h

class CStateMachine {
public:
  enum machinestate {ctrl, sunny, twilight, darkened, moving};
  CStateMachine(); // constructor
  void init(void);
  void handle(void);
  machinestate getState();
  boolean ManualOnly;
private:
  byte CheckInput(void);
  boolean CtrlMove(byte Command);
  boolean CheckSunny(void);
  boolean CheckSunGone(void);
  boolean CheckHotDay(void);
  boolean CheckTwilight(void);
  boolean CheckTwilightGone(void);
  void SetHotDayTimeStart(void);
  boolean CheckHotDayTimeOut(void);
  void CheckNewDay(void);
  machinestate State;
  boolean SunnyInterrupted;
  boolean TwilightInterrupted;
  boolean HotDayInterrupted;
  unsigned short HotDayTimeStart;
};

extern CStateMachine stateMachine;

#endif
