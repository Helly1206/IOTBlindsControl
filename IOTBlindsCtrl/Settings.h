/* 
 * IOTBlindCtrl - Settings
 * Reads and writes settings from EEPROM
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 14-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef IOTSettings_h
#define IOTSettings_h

#include <EEPROM.h>

#ifdef DEBUG_SERIAL
//#define DEBUG_SETTINGS
#endif
//#define FORCE_DEFAULTS

#define EEPROM_SIZE   512
#define EEPROM_START  0
#define STANDARD_SIZE 32
#define PASSWORD_SIZE 64

#define DT_BYTE   0
#define DT_SHORT  1
#define DT_LONG   2
#define DT_FLOAT  3
#define DT_STRING 4

class Item {
  public:
    Item(byte _datatype, unsigned short _start); // constructor
    Item(byte _datatype, unsigned short _start, unsigned short _size); // constructor
    byte datatype;
    unsigned short start;
    unsigned short size;
};

class cSettings {
  public:
    cSettings(); // constructor
    ~cSettings(); // destructor
    void init();
    void get(Item *item, byte &b);
    void get(Item *item, unsigned short &s);
    void get(Item *item, unsigned long &l);
    void get(Item *item, float &f);
    void get(Item *item, String &s);
    byte getByte(Item *item);
    unsigned short getShort(Item *item);
    unsigned long getLong(Item *item);
    float getFloat(Item *item);
    String getString(Item *item);

    void set(Item *item, byte &b);
    void set(Item *item, unsigned short &s);
    void set(Item *item, unsigned long &l);
    void set(Item *item, float &f);
    void set(Item *item, String &s);

    void update();
    unsigned short getSize(byte datatype);
    byte getDatatype(Item *item);
    
    // Sensor parameters
    Item *TwilightIGain;       // [0..65535]
    Item *SunnyIGain;          // [0..65535]
    Item *SampleTime;          // [ms] [0..65535]
    Item *OutputThreshold;     // [0..65535]
    Item *FixedOpen;           // [mod] [0..1439]
    Item *FixedClose;          // [mod] [0..1439]
    Item *TwilightThreshold;   // [0..1023]
    Item *TwilightHysterises;  // [0..1023]
    Item *SunnyThreshold;      // [0..1023]
    Item *SunnyHysterises;     // [0..1023]
    Item *SunnyEnableTemp;     // [degC]
    Item *HotDayTemp;          // [degC]
    Item *HotDayEnableTimeout; // [mod] [0..1439]
    Item *SensorEnabled;       // [bool]

    // Motor parameters
    Item *MotorMoveTime;       // [ms] [0..65535]
    Item *MotorSyncPerc;       // [%] [0..100]
    Item *MotorSunnyPos;       // [%] [0..100]
    Item *MotorEnabled;        // [bool]
  
    // Wifi parameters
    Item *ssid;                // String 32
    Item *password;            // String 64
    Item *hostname;            // String 32
    Item *NtpServer;           // String 32
    Item *NtpZone;             // [byte] [-12..12]
    Item *UseDST;              // [bool]

    // MQTT parameters
    Item *brokerAddress;       // String 32
    Item *mqttPort;            // [0..65535]
    Item *mqttUsername;        // String 32
    Item *mqttPassword;        // String 64
    Item *mainTopic;           // String 64
    Item *mqttQos;             // [byte] [0..1]
    Item *mqttRetain;          // [bool]
    Item *UseMqtt;             // [bool]

    unsigned short memsize;
  private:
    void initParameters();
    void resetSettings(unsigned short start, unsigned short size);
    boolean IsEmpty(unsigned short start, unsigned short size);
    void defaultBlindParameters();
    void defaultWifiParameters();
    void defaultMqttParameters();
};

extern cSettings settings;

#endif
