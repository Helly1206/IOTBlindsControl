/* 
 * IOTBlindCtrl - Commands
 * Command Queue
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Commands_h
#define Commands_h

#define MAX_COMMANDS  16

#define CMD_DOWN       0
#define CMD_UP         100
#define CMD_SHADE      110
#define CMD_STOP       120
#define CMD_MANUAL     230    
#define CMD_NONE       255

class CCommandQueue {
public:
  CCommandQueue(); // constructor
  byte getNextCommand();
  byte addCommand(byte Cmd);
  // command storage for when busy ...
  boolean commandStored(void);
  void storeCommand(byte Cmd);
  byte getStoredCommand();
  void clearStoredCommand(void);
private:
  void clearCommand(byte pCmdQ);
  void inc_pCmdQ_Front();
  void inc_pCmdQ_Rear();
  byte pCmdQ_Amount();
  boolean pCmdQ_Full();
  
  byte CommandQueue[MAX_COMMANDS];
  byte pCmdQ_Front; // increased when a command is added, so contains next command to be added
  byte pCmdQ_Rear; // increased when a command is read, so contains next command to be read
  boolean CmdSemaphore;
  byte StoredCommand;
};

extern CCommandQueue cmdQueue;

#endif
