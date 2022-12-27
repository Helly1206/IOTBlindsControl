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

const unsigned char INITIALIZATION_VECTOR[] = {0x2C, 0xCF, 0x47, 0xFB, 0x48, 0x1E, 0x10, 0x28, 0x0C, 0x51, 0xC8, 0x8C, 0xAB, 0x58, 0xE7, 0xBE};
const unsigned char CYPHER_KEY[] = {0xF2, 0x58, 0xC6, 0x81, 0x6C, 0x31, 0x06, 0x4D, 0x6C, 0x77, 0x70, 0x5E, 0xAA, 0xDC, 0xC3, 0xC9, 
                                    0xBF, 0x9A, 0xE8, 0x7C, 0x2E, 0x80, 0x6E, 0x7A, 0xB1, 0x0D, 0x0B, 0x6B, 0x13, 0x3E, 0xE1, 0x0C};

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
#ifdef DO_ENCRYPT
  Item *oldItem = new Item(DT_STRING, ssid->start, ssid->size);
  String value;
  get(oldItem, value);
  set(ssid, value);
  oldItem->start = password->start;
  oldItem->size = password->size;  
  get(oldItem, value);
  set(password, value);
  oldItem->start = mqttUsername->start;
  oldItem->size = mqttUsername->size;
  get(oldItem, value);
  set(mqttUsername, value);
  oldItem->start = mqttPassword->start;
  oldItem->size = mqttPassword->size;
  get(oldItem, value);
  set(mqttPassword, value);  
  update();
#endif
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
  } else if (item->datatype == DT_CYPHER) {
    s = getDecrypt(item);
  }
}

void cSettings::get(Item *item, char *s) {
  if ((item->datatype == DT_STRING) || (item->datatype == DT_CYPHER)) {
    char c = '-';
    for (unsigned short i = 0; (i < item->size); i++) {
      s[i] = char(EEPROM.read(item->start + i));
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

String cSettings::getDecrypt(Item *item) {
  char encrypted[item->size] = {0};
  char decrypted[item->size] = {0};
  get(item, encrypted);
  aesDecrypt(encrypted, decrypted, item->size);
  return String(decrypted);
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
  } else if (item->datatype == DT_CYPHER) {
    setEncrypt(item, s);
  }
}

void cSettings::set(Item *item, char *s) {
  if ((item->datatype == DT_STRING) || (item->datatype == DT_CYPHER)) {
    unsigned short i;
    for (i = 0; i < item->size; i++) {
      EEPROM.write(item->start + i, s[i]);
    }
  }
}

void cSettings::setEncrypt(Item *item, String &s) {
  char decrypted[item->size] = {0};
  char encrypted[item->size] = {0};
  strcpy(decrypted, s.c_str());
  aesEncrypt(decrypted, encrypted, item->size);
  set(item, encrypted);
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
  ssid = new Item(DT_CYPHER, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  password = new Item(DT_CYPHER, startAddress, PASSWORD_SIZE);
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
  mqttUsername = new Item(DT_CYPHER, startAddress, STANDARD_SIZE);
  startAddress += STANDARD_SIZE;
  mqttPassword = new Item(DT_CYPHER, startAddress, PASSWORD_SIZE);
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

void cSettings::aesDecrypt(char *input, char *output, int dataLength) {
  unsigned char iv[IV_LEN];
  memcpy(iv, INITIALIZATION_VECTOR, IV_LEN);
  if ((dataLength % 16) != 0) dataLength += 16 - (dataLength % 16);
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, CYPHER_KEY, KEY_BITS);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_DECRYPT, dataLength, iv, (const unsigned char*)input, (unsigned char *)output);
  mbedtls_aes_free(&aes);
}

void cSettings::aesEncrypt(const char *input, char *output, int dataLength) {
  unsigned char iv[IV_LEN];
  memcpy(iv, INITIALIZATION_VECTOR, IV_LEN);
  if ((dataLength % 16) != 0) dataLength += 16 - (dataLength % 16);
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, CYPHER_KEY, KEY_BITS);
  mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, dataLength, iv, (const unsigned char*)input, (unsigned char *)output);
  mbedtls_aes_free(&aes);
}

cSettings settings;
