/* 
 * IOTBlindCtrl - Blind
 * Blind actuation and position calculation
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

/* 
 * Orignal: Blind actuation and position calculation
 * Arduino Nano V3.0
 * Version 0.00
 * 20-7-2012
 * new interrupt based version 26-11-2022
 */ 
 
 // down = 0 %
 // up = 100 %
 
#include "Blind.h"
#include "Settings.h"
#include "HWtimer.h"
#include "LEDs.h"

volatile CBlind::blindstate CBlind::blindState = stopped;
portMUX_TYPE CBlind::timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile unsigned long CBlind::moveTime = 0;
volatile unsigned long CBlind::syncTime = 0;
volatile unsigned long CBlind::stoppedTime = 0;

CBlind::CBlind() { // constructor
  pinMode(BLINDUPDN_PIN, OUTPUT);
  pinMode(BLINDONOFF_PIN, OUTPUT);
  digitalWrite(BLINDUPDN_PIN, LOW);
  digitalWrite(BLINDONOFF_PIN, LOW);
  blindEnabled = true;
}

void CBlind::init() {
  // assume blinds are up after reset, So go down when this happens at night.
  Startpos = BLIND_POSUP;
  Position = BLIND_POSUP;
  Setpoint = BLIND_POSUP;
  blindDir = none;
  hwtimer.init(hwtimer.getPrescaler(MAX_TIME), TMR_SINGLE);
  hwtimer.attachInterrupt(&isr_timer);
}

void CBlind::handle() { 
  if (blindDir != none) {
    if ((blindState == stopping) || (blindState == stopped) || (blindState == syncing)) {
      CalcPos(true);
      if ((blindState == stopping) || (blindState == stopped)) {
        blindDir = none;
        LED.Off();
#ifdef DEBUG_BLIND
        Serial.println("DEBUG: BlindDir None");
#endif
#ifdef DEBUG_BLIND_TIMING
        Serial.println("DEBUG: stoppedTime " + String(stoppedTime));
#endif
      }
    } else {
      CalcPos(false);
    }
  }
}

byte CBlind::getPosition() {
  return (Position);
}

byte CBlind::getSetPoint() {
  return (Setpoint);
}

boolean CBlind::getPositionDown() {
  return (Position <= BLIND_POSDOWN);
}

CBlind::blinddir CBlind::moveDir(blinddir Dir) {
  blinddir retdir = none;
  
  if (Dir == up) {
    if (Position < BLIND_POSUP) {
      retdir = movePos(BLIND_POSUP);
    }
  } else if (Dir == down) {
    if (Position > BLIND_POSDOWN) {
      retdir = movePos(BLIND_POSDOWN);
    }	
  }

#ifdef DEBUG_BLIND
  Serial.print("DEBUG: blind::moveDir("+ String(Dir) +") = " + String(retdir) +"\n");
#endif

  return (retdir);
}

CBlind::blinddir CBlind::movePos(byte Pos) {  
  if (getBlindEnabled() && (blindState == stopped)) {
    if (Position<Pos) { // move up to pos
      blindDir = up;
      Startpos = Position;
      Setpoint = Pos;
#ifdef DEBUG_BLIND
      Serial.println("DEBUG: Blind Up()");
#endif
      LED.Up();
      StartMove();
    } else if (Position>Pos) { // move down to pos  
      blindDir = down;
      Startpos = Position;
      Setpoint = Pos;
#ifdef DEBUG_BLIND
      Serial.println("DEBUG: Blind Down()");
#endif
      LED.Down();
      StartMove(); 
    } else {
      blindDir = none;
      LED.Off();
    }
  }

#ifdef DEBUG_BLIND
  Serial.print("DEBUG: blind::movePos("+ String(Pos) +") = " + String(blindDir) +"\n");
#endif
  
  return (blindDir);
}

CBlind::blinddir CBlind::stop() {
  if (blindState != stopped) {
    portENTER_CRITICAL(&timerMux);
    stoppedTime = hwtimer.getuTime();
    digitalWrite(BLINDONOFF_PIN, LOW);
    hwtimer.trigger(RELAY_DELAY_TIME);
    blindState = stopping;
    portEXIT_CRITICAL(&timerMux);
  }

#ifdef DEBUG_BLIND
  Serial.print("DEBUG: blind::stop()\n");
#endif
  
  return (none);
}

boolean CBlind::isMoving() {
  return (blindDir != none);
}

void CBlind::setBlindEnabled(boolean enable) {
  blindEnabled = enable;
}
  
boolean CBlind::getBlindEnabled() {
  return blindEnabled ? (boolean)settings.getByte(settings.MotorEnabled) : false;
}

///////////// PRIVATES ///////////////////////////

unsigned long CBlind::GetLength() {
  unsigned long Length;
  
  if (blindDir == up) {
    Length = Setpoint - Startpos;
  } else if (blindDir == down) {
    Length = Startpos - Setpoint;
  } else {
    Length = 0;
  }
  return (Length);
}

void CBlind::StartMove() {
  if (blindDir == none) {
    return;
  }
  unsigned long imoveTime = ((unsigned long)settings.getShort(settings.MotorMoveTime))*GetLength()*10; // %2us = *1000/100
  unsigned long isyncTime = ((unsigned long)settings.getShort(settings.MotorMoveTime))*((unsigned long)settings.getByte(settings.MotorSyncPerc))*10;
  if (blindDir == up) {
    if (Setpoint < BLIND_POSUP) {
      isyncTime = 0;
    }
    digitalWrite(BLINDUPDN_PIN, HIGH); // up is high, down is low
  }
  if (blindDir == down) {
    if (Setpoint > BLIND_POSDOWN) {
      isyncTime = 0;
    }
    digitalWrite(BLINDUPDN_PIN, LOW); // up is high, down is low, should be low already
  }
  portENTER_CRITICAL(&timerMux);
  moveTime = imoveTime;
  syncTime = isyncTime;
  stoppedTime = 0;
  hwtimer.trigger(RELAY_DELAY_TIME);
  blindState = starting;
  portEXIT_CRITICAL(&timerMux);
#ifdef DEBUG_BLIND_TIMING
  Serial.println("DEBUG: moveTime, " + String(moveTime));
  Serial.println("DEBUG: syncTime, " + String(syncTime));
#endif
}

void CBlind::CalcPos(boolean endPos) {
  byte Done;  
  
  if (endPos) {
    Done = (byte)((GetLength()*stoppedTime)/moveTime);
  } else {
    Done = (byte)((GetLength()*hwtimer.getuTime())/moveTime);
  }
  if (blindDir == up) {
    Position = Startpos + Done;
    if (Position > BLIND_POSUP) {
      Position = BLIND_POSUP;
    }
  } else  if (blindDir == down) {
    if ((Done + BLIND_POSDOWN) > Startpos) {
      Position = BLIND_POSDOWN;
    } else {
      Position = Startpos - Done;
    }
  }
}

void IRAM_ATTR CBlind::isr_timer() {
  portENTER_CRITICAL_ISR(&timerMux);
  switch (blindState) {
    case starting:
      if (moveTime >= MIN_TIME) {
        // on output and upping/ downing timer
        digitalWrite(BLINDONOFF_PIN, HIGH);
        hwtimer.trigger(moveTime);
        blindState = moving;
      } else {
        // another 100 ms timer before switching dir relay off
        stoppedTime = 0;
        hwtimer.trigger(RELAY_DELAY_TIME);
        blindState = stopping;
      }
      break;
    case moving:
      stoppedTime = hwtimer.getuTime();
      if (syncTime >= MIN_TIME) {
        hwtimer.trigger(syncTime);
        blindState = syncing;
      } else { // make extra stop function
        digitalWrite(BLINDONOFF_PIN, LOW);
        hwtimer.trigger(RELAY_DELAY_TIME);
        blindState = stopping;
      }
      break;
    case stopping:
      digitalWrite(BLINDUPDN_PIN, LOW); // always low, save power
      blindState = stopped;
      break;
    case syncing:
      digitalWrite(BLINDONOFF_PIN, LOW);
      hwtimer.trigger(RELAY_DELAY_TIME);
      blindState = stopping;    
      break;
    default: 
      break;
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

CBlind blind;
