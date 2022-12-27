/*
 * IOTBlindsCtrl
 * Roller Blinds control software with IOT (web and MQTT communication)
 * Hardware: Lolin S2 Mini (based on HellysRollerBlind)
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

//#define DEBUG_SERIAL

#define APPVERSION       "v1.1.0"

#define BLINDUPDN_PIN  35 // GPIO35 // if blind then no chill or timer interrupts
#define BLINDONOFF_PIN 33 // GPIO33
#define BUTTONUP_PIN   39 // GPIO39 // interrupt
#define BUTTONDN_PIN   37 // GPIO37 // interrupt
#define FLASH_PIN       0 // GPIO0  // keep
#define SLED_PIN       15 // GPIO15 // system led
#define LEDUP_PIN      18 // GPIO18 // keep multiple of interval
#define LEDDN_PIN      16 // GPIO16 // same
#define SENSOR_PIND     5 // GPIO5  // keep
#define SENSOR_PINA     3 // ADC1_CH2 // keep
#define DS18S20_PIN    12 // GPIO12 // keep

#include "IOTWifi.h"
#include "WebServer.h"
#include "Chiller.h"
#include "LEDs.h"
#include "Buttons.h"
#include "Blind.h"
#include "LightSensor.h"
#include "Temperature.h"
#include "Settings.h"
#include "StateMachine.h"
#include "Clock.h"
#include "mqtt.h"

void setup() {
#ifdef DEBUG_SERIAL
  Serial.begin(115200);
  delay(2000);
#endif
  settings.init();
  LED.init();
  buttons.init();
  blind.init();
  lightSensor.init();
  temp.init();
  stateMachine.init();
  iotWifi.init();
  webServer.init();
  Clock.init();
  mqtt.init();
  chiller.init();
}

void loop() {
  LED.handle();
  buttons.handle();
  blind.handle();
  lightSensor.handle();
  temp.handle();
  stateMachine.handle();
  iotWifi.handle();
  webServer.handle();
  Clock.handle();
  mqtt.handle();
  chiller.handle();


}
