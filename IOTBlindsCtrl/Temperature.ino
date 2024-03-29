/* 
 * IOTBlindCtrl - Temperature sensor
 * Reads DS18S20 and and processes Temperature sensor value
 * Hardware: Lolin S2 Mini
 * Version 0.80
 * 13-6-2021
 * Copyright: Ivo Helwegen
 */
 
/* 
 * Original: DS18S20 temperature sensor readout
 * Arduino Nano V3.0
 * Version 0.00
 * 28-7-2012
 */

#include <OneWire.h>
#include "Temperature.h"

//Temperature chip i/o
OneWire ds(DS18S20_PIN);

portMUX_TYPE CTemp::mux = portMUX_INITIALIZER_UNLOCKED;
boolean CTemp::sampled = false;

CTemp::CTemp() { // constructor
  SampledTemp = ABS_ZERO;
  noDevice = true;
}

void CTemp::init(void) {
  ClearFilter();
  timer = xTimerCreateStatic("temp", pdMS_TO_TICKS(SAMPLE_TEMP), pdTRUE, (void *)0, timerCallback, &timerBuffer);
  xTimerStart(timer, portMAX_DELAY);
  tempState = start;
}

void CTemp::handle(void) {
  if (sampled) { // interval elapsed
    portENTER_CRITICAL(&mux);
    sampled = false;
    portEXIT_CRITICAL(&mux);
    switch (tempState) {
      case start:
        if (noDevice) {
          DS18S20Address();
          ClearFilter();
        }
        tempState = wait1;
        break;
      case wait1:
        tempState = wait2;
        break;
      case wait2:
        tempState = conv;
        break;
      case conv:
        if (!noDevice) {
          DS18S20StartConversion();
        }
        tempState = read;
        break;
      case read:
        if (!noDevice) {
          FilteredTemp = Filter(DS18S20Temp());
        }
        logger.printf(LOG_TEMPERATURE, "Temperature [degC]: " + String(FilteredTemp));
      default:
        tempState = start;
        break;
    }
  }
}

float CTemp::getTemp(void) {
  if (blind.getPositionDown() || (SampledTemp == ABS_ZERO)) {   // only measure when blind is really down
    SampledTemp = FilteredTemp;
  }
  
  return (SampledTemp);
}

float CTemp::getRealTimeTemp(void) {
  return (FilteredTemp);
}

boolean CTemp::DS18S20Address(void) {
  ds.reset_search();

  if ( !ds.search(Address)) {
    //no more sensors on chain, reset search
    ds.reset_search();
    logger.printf(LOG_TEMPERATURE, "DS18S20 - No more sensors on chain!");
    return (false);
  }

  if ( OneWire::crc8( Address, 7) != Address[7]) {
    logger.printf(LOG_TEMPERATURE, "DS18S20 - CRC is not valid!");
    return (false);
  }

  if ((Address[0] != ADDR0_DS18S20) && (Address[0] != ADDR0_DS18B20)) {
    if (!noDevice) {
      logger.printf(LOG_TEMPERATURE, "DS18S20 - Device is not recognized");
    }
    noDevice = true;
    SampledTemp = ABS_ZERO;
    return (false);
  } else {
    noDevice = false;
  }

  logger.printf(LOG_TEMPERATURE, "DS18S20 - Address: " + String(Address[0], HEX) + (", ") + String(Address[1], HEX));
  return (true);
}

void CTemp::DS18S20StartConversion(void) {
  if ((Address[0] != ADDR0_DS18S20) && (Address[0] != ADDR0_DS18B20)) {
    if (!noDevice) {
      logger.printf(LOG_TEMPERATURE, "DS18S20 - Device is not recognized");
    }
    noDevice = true;
    return; // 0 degrees if no sensor attached
  }  
  noDevice = false;
  ds.reset();
  ds.select(Address);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  if (ds.reset() <= 0) {
    logger.printf(LOG_TEMPERATURE, "DS18S20 - Unable to reset device");
    noDevice = true;
    return; // 0 degrees if no sensor attached
  }
}

float CTemp::DS18S20Temp(void) {
  //returns the temperature from one DS18S20 in Degrees Celsius
  byte data[12];

  if (noDevice) {
    return (ABS_ZERO);
  }
  
  ds.select(Address);    
  ds.write(0xBE); // Read Scratchpad
 
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

#ifdef DEBUG_TEMP
  logger.printf(LOG_TEMPERATURE, "DS18S20 - Data: " + String(data[1], HEX) + ", " + String(data[0], HEX));
#endif

  int16_t raw = ((data[1] << 8) | data[0]); //using two's compliment
  if (Address[0] == ADDR0_DS18S20) { // DS18S20
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else { // DS18B20
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  
  return (float)raw / 16.0; 
}

float CTemp::Filter(float Sample) {
#ifdef MOVAV_FILTER
  return(MovAvFilter(Sample));
#else
  return(LimitFilter(Sample));
#endif
}

void CTemp::ClearFilter(void) {
#ifdef MOVAV_FILTER
  ClearMovAvFilter();
#else
  ClearLimitFilter();
#endif
}

#ifdef MOVAV_FILTER
float CTemp::MovAvFilter(float Sample) {
  int i;
  float SumMemory = 0;

  if (Sample > ABS_ZERO) {
    for (i=MOVAV_TEMP-1;i>0;i--) {
      TempMemory[i] = TempMemory[i-1];
      SumMemory += TempMemory[i];
    }
    TempMemory[0] = Sample;
    SumMemory += TempMemory[0];
  } else {
    for (i=MOVAV_TEMP-1;i>=0;i--) {
      SumMemory += TempMemory[i];
    }
  }
#ifdef DEBUG_TEMP
  logger.printf(LOG_TEMPERATURE, "Sample: " + String(Sample) + ", sum: " + String(SumMemory));
#endif

  return (SumMemory/MOVAV_TEMP);
}

void CTemp::ClearMovAvFilter(void) {
  int i;
  
  for (i=0;i<MOVAV_TEMP;i++) {
    TempMemory[i] = 0.0;
  }
}

#else

float CTemp::LimitFilter(float Sample) {
  if (FilterInit) {
    FilterInit = false;
    TempMemory = Sample;
  } else {
    if (Sample > TempMemory + LIMIT_FILTER_DIFF) { //too large value
      TempMemory += LIMIT_FILTER_DIFF;
    } else if (Sample < TempMemory - LIMIT_FILTER_DIFF) { //too small value
      TempMemory -= LIMIT_FILTER_DIFF;
    } else { // correct value
      TempMemory = Sample;
    }
  }

#ifdef DEBUG_TEMP
  logger.printf(LOG_TEMPERATURE, "Sample: " + String(Sample) + ", mem: " + String(TempMemory));
#endif
  
  return (TempMemory);
}

void CTemp::ClearLimitFilter(void) {
  FilterInit = true;
  TempMemory = 0.0;
}

#endif

void CTemp::timerCallback(TimerHandle_t xTimer) {
  portENTER_CRITICAL(&mux);
  sampled = true;
  portEXIT_CRITICAL(&mux);
}

CTemp temp;
