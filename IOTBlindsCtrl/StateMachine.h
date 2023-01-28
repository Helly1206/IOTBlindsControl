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

#define CMD_DOWN       0
#define CMD_UP         100
#define CMD_SHADE      110
#define CMD_STOP       120
#define CMD_MANUAL     230    
#define CMD_NONE       255

class CStateMachine {
public:
  enum machinestate {ctrl, sunny, twilight, darkened, moving};
  CStateMachine(); // constructor
  void init(void);
  void handle(void);
  machinestate getState();
  boolean isManualOnly();
  byte setCmd(byte command);
  
private:
  byte getCmd(void);
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
  byte cmd;
  boolean ManualOnly;
  static portMUX_TYPE cmdMux;
};

extern CStateMachine stateMachine;

#endif
