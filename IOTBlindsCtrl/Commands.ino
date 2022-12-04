/* 
 * IOTBlindCtrl - Commands
 * Command Queue
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

/* 
 * Original: Command Queue
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 */
 
#include "Commands.h"
#include "Clock.h"

CCommandQueue::CCommandQueue() {
  pCmdQ_Front = 0; // increased when a command is added, so contains next command to be added
  pCmdQ_Rear = 0; // increased when a command is read, so contains next command to be read
  CmdSemaphore = false;
  clearStoredCommand();
}

byte CCommandQueue::getNextCommand() {
  byte Cmd = CMD_NONE;
  if (pCmdQ_Amount() <= 0) return Cmd; // return if no command waiting
  if (CmdSemaphore) return Cmd; // no processing allowed when in command callback
    
  // process next command
  Cmd = CommandQueue[pCmdQ_Rear];
  clearCommand(pCmdQ_Rear); 
  inc_pCmdQ_Rear();
      
  return Cmd;
}

byte CCommandQueue::addCommand(byte Cmd) {
  if (pCmdQ_Full()) return CMD_NONE; // don't process if buffer full .. press key again later
  
  CmdSemaphore = true;
  CommandQueue[pCmdQ_Front] = Cmd;
  inc_pCmdQ_Front();
  CmdSemaphore = false;
  Clock.newCommand();
  
  return Cmd;
}

// command storage for when busy ...
// only one stored command possible, user is responsible for correct handling
boolean CCommandQueue::commandStored(void) {
  return (StoredCommand != CMD_NONE);
}

void CCommandQueue::storeCommand(byte Cmd) {
  if (!commandStored()) {
    StoredCommand = Cmd;
  }
}

byte CCommandQueue::getStoredCommand() {
  return StoredCommand;
}

void CCommandQueue::clearStoredCommand(void) {
  StoredCommand = CMD_NONE;
}

///////////// PRIVATES ///////////////////////////

void CCommandQueue::clearCommand(byte pCmdQ) {
  CommandQueue[pCmdQ] = CMD_NONE;  
}

void CCommandQueue::inc_pCmdQ_Front() {
  if (pCmdQ_Front < MAX_COMMANDS - 1) {
    pCmdQ_Front++;
  } else {
    pCmdQ_Front = 0;
  }
}

void CCommandQueue::inc_pCmdQ_Rear() {
  if (pCmdQ_Rear < MAX_COMMANDS - 1) {
    pCmdQ_Rear++;
  } else {
    pCmdQ_Rear = 0;
  }
}

byte CCommandQueue::pCmdQ_Amount() {
  byte retval = 0;
  if (pCmdQ_Front >= pCmdQ_Rear) {
    retval = pCmdQ_Front - pCmdQ_Rear; // number of commands in queue
  } else { // ex. front = 3, rear = 29, commands = 29,30,31,0,1,2 = 6
    retval = MAX_COMMANDS + pCmdQ_Front - pCmdQ_Rear; // 32 + 3 - 29 = 6
  }
  return(retval);
}

boolean CCommandQueue::pCmdQ_Full() {
  return ((pCmdQ_Amount() >= (MAX_COMMANDS - 1)) ? true:false); 
}

CCommandQueue cmdQueue;
