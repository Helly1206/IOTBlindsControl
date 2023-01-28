/* 
 * HellysRollerBlind
 * State Machine
 * Hardware: Lolin S2 Mini
 * Version 0.00
 * 20-7-2012
 * Copyright Ivo Helwegen
 */ 
 
#include "StateMachine.h"

portMUX_TYPE CStateMachine::cmdMux = portMUX_INITIALIZER_UNLOCKED;

CStateMachine::CStateMachine() { // constructor
  setCmd(CMD_NONE);
  State = ctrl;
  ManualOnly = false;
  SunnyInterrupted = false;
  TwilightInterrupted = false;
  HotDayInterrupted = false;
  HotDayTimeStart = 0;
}

void CStateMachine::init(void) {
  setCmd(CMD_DOWN); // move down at start
}

void CStateMachine::handle(void) {
  if (iotWifi.wakingUp()) {
    return;
  }
  byte InputResult = CheckInput(); // valid commands are CMD_NONE, CMD_STOP (Only if moving), and pos 0 .. 100, the rest is already set at checkinput
  CheckNewDay();
  // stop is also allowed when moving to sunny, dark or whatever. the setup is interrupted in that case.
  switch (State) {
    case ctrl:
      if (CtrlMove(InputResult)) {
        logger.printf(LOG_STATEMACHINE, "State ctrl");
        State = ctrl;
      } else if (CheckHotDay()) {
        blind.moveDir(blind.down);
        SetHotDayTimeStart();
        logger.printf(LOG_STATEMACHINE, "State darkened");
        State = darkened;
      } else if (CheckSunny()) {
        blind.movePos(settings.getByte(settings.MotorSunnyPos));
        lightSensor.setSunny(true);
        logger.printf(LOG_STATEMACHINE, "State sunny");
        State = sunny;
      } else if (CheckTwilight()) { // dark has priority above twilight (because same sensor higher value)
        blind.movePos(settings.getByte(settings.MotorSunnyPos));
        lightSensor.setTwilight(true);
        logger.printf(LOG_STATEMACHINE, "State twilight");
        State = twilight;
      }
      break;  
    case sunny:
      // don't check for dark when in sunny position, back to idle state first
      SunnyInterrupted = false;
      if (CtrlMove(InputResult)) {
        SunnyInterrupted = true;
        logger.printf(LOG_STATEMACHINE, "State ctrl");
        State = ctrl;
      } else if (CheckHotDay()) {
        blind.moveDir(blind.down);
        SetHotDayTimeStart();
        logger.printf(LOG_STATEMACHINE, "State darkened");
        State = darkened;
      } else if (CheckSunGone()) { // sun is gone
        if (!blind.isMoving()) { // if still moving into sunny position, end this move
          blind.moveDir(blind.up);
          lightSensor.setSunny(false);
          logger.printf(LOG_STATEMACHINE, "State ctrl");
          State = ctrl;  
        }
      } else if (CheckTwilight()) { // should not happen, but if sudden twilight appears, change state
        if (!blind.isMoving()) { // if still moving into sunny position, end this move
          lightSensor.setTwilight(true);
          logger.printf(LOG_STATEMACHINE, "State twilight");
          State = twilight;
        }
      }
      break;
    case twilight:
      TwilightInterrupted = false;
      if (CtrlMove(InputResult)) {
        TwilightInterrupted = true;
        logger.printf(LOG_STATEMACHINE, "State ctrl");
        State = ctrl;
      } else if (CheckTwilightGone()) { // twilight is gone
        if (!blind.isMoving()) { // if still moving into twilight position, end this move
          blind.moveDir(blind.up);
          lightSensor.setTwilight(false);
          logger.printf(LOG_STATEMACHINE, "State ctrl");
          State = ctrl;  
        }    
      } else if (CheckSunny()) { // should not happen, but if sunny again, change state
        if (!blind.isMoving()) { // if still moving into twilight position, end this move
          lightSensor.setSunny(true);
          logger.printf(LOG_STATEMACHINE, "State sunny");
          State = sunny;
        }
      }
      break;
    case darkened:
      // no sunny check on dark
      HotDayInterrupted = false;
      if (CtrlMove(InputResult)) {
        HotDayInterrupted = true;
        logger.printf(LOG_STATEMACHINE, "State ctrl");
        State = ctrl;
      } else if (CheckHotDayTimeOut()) {
        if (!blind.isMoving()) { // if still moving into twilight position, end this move
          blind.moveDir(blind.up);
          lightSensor.setTwilight(false);
          lightSensor.setSunny(false);
          logger.printf(LOG_STATEMACHINE, "State ctrl");
          State = ctrl;  
        }
      } // no sunny/ dark check now
      break;
    default:
      logger.printf(LOG_STATEMACHINE, "State ctrl");
      State = ctrl;
      break;
  }
}

CStateMachine::machinestate CStateMachine::getState() {
  machinestate istate = ctrl;
  if (blind.isMoving()) {
    istate = moving;
  } else {
    istate = State;
  }

  return istate;
}

boolean CStateMachine::isManualOnly() {
  return ManualOnly;
}

byte CStateMachine::setCmd(byte command) {
  logger.printf(LOG_STATEMACHINE, "setCmd: " + String(command));
  portENTER_CRITICAL(&cmdMux);
  cmd = command;
  portEXIT_CRITICAL(&cmdMux);
  Clock.newCommand();
  return command;
}

///////////// PRIVATES ///////////////////////////

byte CStateMachine::getCmd(void) {
  byte command = CMD_NONE;
  portENTER_CRITICAL(&cmdMux);
  command = cmd;
  cmd = CMD_NONE;
  portEXIT_CRITICAL(&cmdMux);
  return command;
}

// Stored command drawback: Button action won't work when stored command is waiting for execution
byte CStateMachine::CheckInput(void) {
  byte retval = CMD_NONE;
  byte CurrentCommand = getCmd();
  if (CurrentCommand != CMD_NONE) { // new command waiting
    if (CurrentCommand == CMD_MANUAL) { // both buttons pressed
      ManualOnly = !ManualOnly;
      if (!ManualOnly) {
          SunnyInterrupted = false;
          TwilightInterrupted = false;
          HotDayInterrupted = false;
      }
      LED.ManualCommand();
      retval = CMD_NONE;
    } else if (blind.isMoving()) {
      // stop moving if any command when moving (exceptions can be made later, but are dangerous)
      LED.Command(); // short 1
      logger.printf(LOG_STATEMACHINE, "Stop moving blinds");
      retval = CMD_STOP;
      lightSensor.reset(); // reset lightSensor at any move to not get strange moves immediately after because of integrator values
    } else { // all commands are move commands, so always reset the lightSensor
      LED.Command(); // short 1
      if (CurrentCommand == CMD_SHADE) {
        CurrentCommand = settings.getByte(settings.MotorSunnyPos);
      }
      retval = CurrentCommand;
      lightSensor.reset(); // reset lightSensor at any move to not get strange moves immediately after because of integrator values
    }
  }
  return (retval);
}

boolean CStateMachine::CtrlMove(byte Command) {
  boolean ctrlAction = false;
  if (Command != CMD_NONE) {    
    logger.printf(LOG_STATEMACHINE, "Ctrl Command: " + String(Command));
  }
  switch (Command) {
    case CMD_STOP:
      if (blind.isMoving()) {
        logger.printf("Command: Stop!");
        blind.stop();
        ctrlAction = true;
      }
      break;
    case CMD_NONE:
      break;
    default: // pos 0 .. 100
      logger.printf("Command: " + String(Command));
      blind.movePos(Command);
      ctrlAction = true;
      break;  
  }
  return ctrlAction;
}

boolean CStateMachine::CheckSunny(void) {
  boolean retval = false;
  if ((settings.getByte(settings.SensorEnabled)) && (!ManualOnly) && (!SunnyInterrupted)) {
    if (temp.getTemp() >= settings.getFloat(settings.SunnyEnableTemp)) { // only go down if temperature is high enough
      if (lightSensor.Sunny) {
        if (blind.getPosition() > settings.getByte(settings.MotorSunnyPos)) { // only go down if higher than desired position
          retval = true;
        }
      }
    }
  }
  return (retval);
}

boolean CStateMachine::CheckSunGone(void) {
  boolean retval = false;
  if ((settings.getByte(settings.SensorEnabled)) && (!ManualOnly) && (!SunnyInterrupted)) {
    if (!lightSensor.Sunny) {
      retval = true;
    }
  }
  return (retval);
}

boolean CStateMachine::CheckHotDay(void) {
  boolean retval = false;
  if ((settings.getByte(settings.SensorEnabled)) && (!ManualOnly) && (!HotDayInterrupted)) {
    if (temp.getTemp() >= settings.getFloat(settings.HotDayTemp)) { // only go down if temperature is high enough
      if (lightSensor.Sunny) {
        retval = true;
      }
    }
  }
  return (retval);
}

boolean CStateMachine::CheckTwilight(void) {
  boolean retval = false;
  if ((settings.getByte(settings.SensorEnabled)) && (!ManualOnly) && (!TwilightInterrupted)) {
    if (lightSensor.Twilight) {
      if (blind.getPosition() > settings.getByte(settings.MotorSunnyPos)) { // only go down if higher than desired position
        retval = true;
      }
    }
  }
  return (retval);
} 
  
boolean CStateMachine::CheckTwilightGone(void) {
  boolean retval = false;
  if ((settings.getByte(settings.SensorEnabled)) && (!ManualOnly) && (!TwilightInterrupted)) {
    if (!lightSensor.Twilight) {
      retval = true;
    }
  }
  return (retval);
}

// hotday start at mod
void CStateMachine::SetHotDayTimeStart(void) {
  HotDayTimeStart = Clock.getMOD();
}

// hotday check at mod - HotDayTimeStart > HotdayTimeout
boolean CStateMachine::CheckHotDayTimeOut(void) {
  boolean retval = false;
  if (Clock.changedMOD()) {
    if ((unsigned short)(Clock.getMOD() - HotDayTimeStart) > settings.getShort(settings.HotDayEnableTimeout)) {
      retval = true; 
    }
  }
  return (retval);
}

void CStateMachine::CheckNewDay(void) {
  if (Clock.changedMOD()) {
    if ((unsigned short)(Clock.getMOD()) == 0) {
      SunnyInterrupted = false;
      TwilightInterrupted = false;
      HotDayInterrupted = false;
    }
  }
}

CStateMachine stateMachine;
