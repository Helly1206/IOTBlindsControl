/* 
 * IOTBlindCtrl - Timers
 * Timers class
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Timers_h
#define Timers_h 

#define MAXTIMERS           8

#define TIMER_SENSOR        0 /* sample time of sensor */
#define TIMER_DEBOUNCE      1 /* button debounce timer */
#define DEBOUNCE_TIME       200 /* ms */
#define TIMER_TEMP          2 /* repeating temperature sample timer */
#define SAMPLE_TEMP         1000 /* ms */ /* 5000 ms over 5 states */
#define MQTT_PUBLISH        3 /* repeating MQTT publish timer */
#define MQTT_PUBLISH_TIME   1000 /* ms */
#define MQTT_RECONNECT      4 /* single shot MQTT reconnect timer */
#define MQTT_RECONNECT_TIME 5000 /* ms */
#define TIMER_WIFI          5 /* single shot timer for start access point selection delay and MDNS setup delay */
#define TIMER_LEDS          6 /* single shot timer for flashing LEDs, timer will be restarted and toggled */
#define TIMER_RESETBTN      7 /* button timer to detect reset request */
#define RESET_TIME          3000 /* ms */

class CTimerParams {
public:
  boolean Repeat;
  unsigned long Interval;
  unsigned long LastTime;
};

class CTimers {
public:
  CTimers(); // constructor
  void init(void);
  void start(byte timerno, unsigned long Interval, bool Repeat);
  void restart(byte timerno);
  void stop(byte timerno);
  unsigned long getElapsed(byte timerno);
  boolean getTimer(byte timerno);
  boolean running(byte timerno);
private:
  CTimerParams MyTimers[MAXTIMERS];
  void ClearTimer(byte timerno);
  unsigned long CalcElapsed(byte timerno);
  unsigned long CalcNewElapsed(unsigned long LastTime);
};

extern CTimers timers;

#endif
