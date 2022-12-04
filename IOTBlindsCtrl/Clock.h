/* 
 * Clock
 * semi real time clock using NTP time
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 30-5-2021
 * Copyright: Ivo Helwegen
 */

#ifndef Clock_h
#define Clock_h

#include <WiFiUdp.h>
#include <TimeLib.h>

#ifdef DEBUG_SERIAL
//#define DEBUG_NTPClient
#endif

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337
#define SECONDS_HOUR 3600
#define UPDATE_INTERVAL 14400000 // (4*3600*1000) update every 4 hours
#define UNSET_INTERVAL 604800000 // (7*24*3600*1000) unset after 7 days of no update

class cClock {
  public:
    cClock(); // constructor
    void init();
    void handle();
    void updateSettings();
    boolean isTimeSet();
    unsigned long getTime();
    String getFormattedDate();
    String getFormattedTime();
    boolean changedMOD();
    unsigned short getMOD();
    void newCommand();
  private:
    WiFiUDP *ntpUDP;
    String poolServerName;
    boolean useDST;
    boolean udpSetup;
    boolean timeSet;
    unsigned int port;
    long timeOffset;
    unsigned long updateInterval;  // In ms
    unsigned long currentEpoc;      // In s
    unsigned long lastUpdate;      // In ms
    byte packetBuffer[NTP_PACKET_SIZE];
    void sendNTPPacket();
    void begin();
    boolean update();
    boolean forceUpdate();
    unsigned long getEpochTime();
    int getStartDSTDay(int y);
    int getStopDSTDay(int y);
    boolean getDST(unsigned long nowTime);
    void handleFixedTimes();
    boolean minuteBit;
    boolean minuteChanged;
    boolean commandToday;
    boolean fixedClosed;
    boolean fixedOpened;
};

extern cClock Clock;

#endif
