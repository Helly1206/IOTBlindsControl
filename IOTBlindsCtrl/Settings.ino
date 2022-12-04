/*
 * IOTBlindCtrl - Settings
 * Reads and writes settings from EEPROM
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#include "Settings.h"
#include "Defaults.h"

Item::Item(byte _datatype, unsigned short _start) { // constructor
  datatype = _datatype;
  start = _start;
  size = settings.getSize(_datatype);
}

Item::Item(byte _datatype, unsigned short _start, unsigned short _size) { // constructor
  datatype = _datatype;
  start = _start;
  size = _size;
}

cSettings::cSettings() { // constructor
  initParameters();
}

cSettings::~cSettings() { // destructor
  EEPROM.end();
}

void cSettings::init() {
  EEPROM.begin(EEPROM_SIZE); //Initialasing EEPROM
#if 0
  int i;
  byte b;
  delay(2000);
  for (i=0;i<UseMqtt->start + UseMqtt->size; i++) {
    EEPROM.get(i, b);
    Serial.print(b, HEX);
    Serial.print("-");
  }
#endif
#ifdef FORCE_DEFAULTS
#ifdef DEBUG_SETTINGS
  Serial.print("Settings: Forcing default settings\n");
#endif
  resetSettings(TwilightIGain->start, (UseMqtt->start + UseMqtt->size) - TwilightIGain->start);
#endif
  if (IsEmpty(TwilightIGain->start, (MotorEnabled->start + MotorEnabled->size) - TwilightIGain->start)) {
#ifdef DEBUG_SETTINGS
    Serial.print("Settings: No blind settings, loading default\n");
#endif
    defaultBlindParameters();
  }
  if (IsEmpty(ssid->start, (UseDST->start + UseDST->size) - ssid->start)) {
#ifdef DEBUG_SETTINGS
    Serial.print("Settings: No wifi settings, loading default\n");
#endif
    defaultWifiParameters();
  }
  if (IsEmpty(brokerAddress->start, (UseMqtt->start + UseMqtt->size) - brokerAddress->start)) {
#ifdef DEBUG_SETTINGS
    Serial.print("Settings: No mqtt settings, loading default\n");
#endif
    defaultMqttParameters();
  }
}

void cSettings::get(Item *item, byte &b) {
  if (item->datatype == DT_BYTE) {
    EEPROM.get(item->start, b);
  }
}

void cSettings::get(Item *item, unsigned short &s) {
  if (item->datatype == DT_SHORT) {
    EEPROM.get(item->start, s);
  }
}

void cSettings::get(Item *item, unsigned long &l) {
  if (item->datatype == DT_LONG) {
    EEPROM.get(item->start, l);
  }
}

void cSettings::get(Item *item, float &f) {
  if (item->datatype == DT_FLOAT) {
    EEPROM.get(item->start, f);
  }
}

void cSettings::get(Item *item, String &s) {
  if (item->datatype == DT_STRING) {
    char c = '-';
    s = "";
    for (unsigned short i = 0; ((i < item->size) && (c != '\0')); i++) {
      c = char(EEPROM.read(item->start + i));
      if (c != '\0') {
        s += c;
      }
    }
  }
}

byte cSettings::getByte(Item *item) {
  byte b;
  get(item, b);
  return b;
}

unsigned short cSettings::getShort(Item *item) {
  unsigned short s;
  get(item, s);
  return s;
}

unsigned long cSettings::getLong(Item *item) {
  unsigned long l;
  get(item, l);
  return l;
}

float cSettings::getFloat(Item *item) {
  float f;
  get(item, f);
  return f;
}

String cSettings::getString(Item *item) {
  String s;
  get(item, s);
  return s;
}

void cSettings::set(Item *item, byte &b) {
  if (item->datatype == DT_BYTE) {
    EEPROM.put(item->start, b);
  }
}

void cSettings::set(Item *item, unsigned short &s) {
  if (item->datatype == DT_SHORT) {
    EEPROM.put(item->start, s);
  }
}

void cSettings::set(Item *item, unsigned long &l) {
  if (item->datatype == DT_LONG) {
    EEPROM.put(item->start, l);
  }
}

void cSettings::set(Item *item, float &f) {
  if (item->datatype == DT_FLOAT) {
    EEPROM.put(item->start, f);
  }
}

void cSettings::set(Item *item, String &s) {
  if (item->datatype == DT_STRING) {
    if (s.length() > item->size) {
      s = s.substring(0, item->size);
    }
    unsigned short i;
    for (i = 0; i < s.length(); i++) {
      EEPROM.write(item->start + i, s[i]);
    }
    for (i = s.length(); i < item->size; i++) {
      EEPROM.write(item->start + i, 0);
    }
  }
}

void cSettings::update() {
  EEPROM.commit();
}

unsigned short cSettings::getSize(byte datatype) {
  unsigned short dsize = 0;
  switch (datatype) {
    case DT_BYTE:
      dsize = sizeof(byte);
      break;
    case DT_SHORT:
      dsize = sizeof(unsigned short);
      break;
    case DT_LONG:
      dsize = sizeof(unsigned long);
      break;
    case DT_FLOAT:
      dsize = sizeof(float);
      break;
    case DT_STRING:
      dsize = 4;
      break;
    default:
      break;
  }
  return dsize;
}

byte cSettings::getDatatype(Item *item) {
  return item->datatype;
}

///////////// PRIVATES ///////////////////////////

void cSettings::initParameters() {
  unsigned short startAddress = EEPROM_START;

  // Sensor parameters
  TwilightIGain = new Item(DT_SHORT, startAddress);       // [0..65535]
  startAddress += getSize(DT_SHORT);
  SunnyIGain = new Item(DT_SHORT, startAddress);          // [0..65535]
  startAddress += getSize(DT_SHORT);
  SampleTime = new Item(DT_SHORT, startAddress);          // [ms] [0..65535]
  startAddress += getSize(DT_SHORT);
  OutputThreshold = new Item(DT_SHORT, startAddress);     // [0..65535]
  startAddress += getSize(DT_SHORT);
  FixedOpen = new Item(DT_SHORT, startAddress);           // [mod] [0..1439]
  startAddress += getSize(DT_SHORT);
  FixedClose = new Item(DT_SHORT, startAddress);          // [mod] [0..1439]
  startAddress += getSize(DT_SHORT);
  TwilightThreshold = new Item(DT_SHORT, startAddress);   // [0..1023]
  startAddress += getSize(DT_SHORT);
  TwilightHysterises = new Item(DT_SHORT, startAddress);  // [0..1023]
  startAddress += getSize(DT_SHORT);
  SunnyThreshold = new Item(DT_SHORT, startAddress);      // [0..1023]
  startAddress += getSize(DT_SHORT);
  SunnyHysterises = new Item(DT_SHORT, startAddress);     // [0..1023]
  startAddress += getSize(DT_SHORT);
  SunnyEnableTemp = new Item(DT_FLOAT, startAddress);     // [degC]
  startAddress += getSize(DT_FLOAT);

  HotDayTemp = new Item(DT_FLOAT, startAddress);          // [degC]
  startAddress += getSize(DT_FLOAT);
  HotDayEnableTimeout = new Item(DT_SHORT, startAddress); // [mod] [0..1439]
  startAddress += getSize(DT_SHORT);
  SensorEnabled = new Item(DT_BYTE, startAddress);        // [bool]
  startAddress += getSize(DT_BYTE);

  // Motor parameters
  MotorMoveTime = new Item(DT_SHORT, startAddress);       // [ms] [0..65535]
  startAddress += getSize(DT_SHORT);
  MotorSyncPerc = new Item(DT_BYTE, startAddress);        // [%] [0..100]
  startAddress += getSize(DT_BYTE);
  MotorSunnyPos = new Item(DT_BYTE, startAddress);        // [%] [0..100]
  startAddress += getSize(DT_BYTE);
  MotorEnabled = new Item(DT_BYTE, startAddress);         // [bool]
  startAddress += getSize(DT_BYTE);

  // Wifi parameters
  ssid = new Item(DT_STRING, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  password = new Item(DT_STRING, startAddress, PASSWORD_SIZE);
  startAddress += PASSWORD_SIZE;
  hostname = new Item(DT_STRING, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  NtpServer = new Item(DT_STRING, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  NtpZone = new Item(DT_BYTE, startAddress);              // [byte] [-12..12]
  startAddress += getSize(DT_BYTE);
  UseDST = new Item(DT_BYTE, startAddress);               // [bool]
  startAddress += getSize(DT_BYTE);

  // MQTT parameters
  brokerAddress = new Item(DT_STRING, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  mqttPort = new Item(DT_SHORT, startAddress);            // [0..65535]
  startAddress += getSize(DT_SHORT);
  mqttUsername = new Item(DT_STRING, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  mqttPassword = new Item(DT_STRING, startAddress, PASSWORD_SIZE);
  startAddress += PASSWORD_SIZE;
  mainTopic = new Item(DT_STRING, startAddress, PASSWORD_SIZE);
  startAddress += PASSWORD_SIZE;
  mqttQos = new Item(DT_BYTE, startAddress);               // [byte] [0..1]
  startAddress += getSize(DT_BYTE);
  mqttRetain = new Item(DT_BYTE, startAddress);            // [bool]
  startAddress += getSize(DT_BYTE);
  UseMqtt = new Item(DT_BYTE, startAddress);               // [bool]
  startAddress += getSize(DT_BYTE);

  memsize = startAddress;
}

void cSettings::resetSettings(unsigned short start, unsigned short size) {
  byte bval = 0xFF;

  for (int i=0; i < size; i++) {
    EEPROM.write(start + i, bval);
  }
  update();
}

boolean cSettings::IsEmpty(unsigned short start, unsigned short size) {
  boolean empty = true;
  byte rd = 0;

  for (int i=0; ((i < size) && (empty)); i++) {
    rd = EEPROM.read(start + i);
    if ((rd != 0x00) && (rd != 0xFF)) {
      empty = false;
    }
  
  }
  return empty;
}

void cSettings::defaultBlindParameters() {
    unsigned short val = 0;
    float fval = 0;
    byte bval = true;
    set(TwilightIGain, val = DEF_TWILIGHTIGAIN);
    set(SunnyIGain, val = DEF_SUNNYIGAIN);
    set(SampleTime, val = DEF_SAMPLETIME);
    set(OutputThreshold, val = DEF_OUTPUTTHRESHOLD);
    set(FixedOpen, val = DEF_FIXEDOPEN);
    set(FixedClose, val = DEF_FIXEDCLOSE);
    set(TwilightThreshold, val = DEF_TWILIGHTTHRESHOLD);
    set(TwilightHysterises, val = DEF_TWILIGHTHYSTERISES);
    set(SunnyThreshold, val = DEF_SUNNYTHRESHOLD);
    set(SunnyHysterises, val = DEF_SUNNYHYSTERISES);
    set(SunnyEnableTemp, fval = DEF_SUNNYENABLETEMP);
    set(HotDayTemp, fval = DEF_HOTDAYTEMP);
    set(HotDayEnableTimeout, val = DEF_HOTDAYENABLETIMEOUT);
    set(SensorEnabled, bval = DEF_SENSORENABLED);
    set(MotorMoveTime, val = DEF_MOTORMOVETIME);
    set(MotorSyncPerc, bval = DEF_MOTORSYNCPERC);
    set(MotorSunnyPos, bval = DEF_MOTORSUNNYPOS);
    set(MotorEnabled, bval = DEF_MOTORENABLED);
    update();
}

void cSettings::defaultWifiParameters() {
    String sval = "";
    byte bval = true;
    set(ssid, sval = DEF_SSID);
    set(password, sval = DEF_PASSWORD);
    set(hostname, sval = DEF_HOSTNAME);
    set(NtpServer, sval = DEF_NTPSERVER);
    set(NtpZone, bval = DEF_NTPZONE);
    set(UseDST, bval = DEF_USEDST);
    update();
}

void cSettings::defaultMqttParameters() {
    String sval = "";
    byte bval = true;
    unsigned short val = 0;
    set(brokerAddress, sval = DEF_BROKERADDRESS);
    set(mqttPort, val = DEF_MQTTPORT);
    set(mqttUsername, sval = DEF_MQTTUSERNAME);
    set(mqttPassword, sval = DEF_MQTTPASSWORD);
    set(mainTopic, sval = DEF_MAINTOPIC);
    set(mqttQos, bval = DEF_MQTTQOS);
    set(mqttRetain, bval = DEF_MQTTRETAIN);
    set(UseMqtt, bval = DEF_USEMQTT);
    update();
}

cSettings settings;
