/*
  DS3231.cpp - Arduino/chipKit library support for the DS3231 I2C Real-Time Clock
  Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved

  This library has been made to easily interface and use the DS3231 RTC with
  an Arduino or chipKit.

  You can find the latest version of the library at
  http://www.RinkyDinkElectronics.com/

  This library is free software; you can redistribute it and/or
  modify it under the terms of the CC BY-NC-SA 3.0 license.
  Please see the included documents for further information.

  Commercial use of this library requires you to buy a license that
  will allow commercial use. This includes using the library,
  modified or not, as a tool to sell products.

  The license applies to all part of the library including the
  examples and tools supplied with the library.
*/
#ifndef DS3231_h
#define DS3231_h
//#include "Arduino.h"
#include <stdint.h>

#define REG_SEC 0x00
#define REG_MIN 0x01
#define REG_HOUR 0x02
#define REG_DOW 0x03
#define REG_DAY 0x04
#define REG_MONTH 0x05
#define REG_YEAR 0x06

#define REG_CONTROL 0x0E     // Control Register Address
#define REG_CONTROL_CONV 0x5 // Convert Temperature
#define REG_STATUS 0x0F      // Status Register Address
#define REG_AGING 0x10       // Aging Register Address
#define REG_TEMP 0x11        // Temperature Register Address

#define REG_I2C 0xD0    // DS3231 (Slave) Address 0x68<<1, damit Platz für R/W-Bit ist
#define REG_ALARM1 0x07 // Alarm 1 Register Address
#define REG_ALARM2 0x0B // Alarm 2 Register Address

// control register bits
#define BIT_A1IE 0  // Alarm 1 Interrupt Enable
#define BIT_A2IE 1  // Alarm 2 Interrupt Enable
#define BIT_INTCN 2 // Interrupt Control

// status register bits
#define BIT_A1F 0    // Alarm 1 Flag
#define BIT_A2F 1    // Alarm 2 Flag
#define REG_OSF 0x80 // Oscillator Stop Flag

#define SEC_1970_TO_2000 946684800

#ifndef TWI_FREQ
#define TWI_FREQ 400000L
#endif

#define DS3231_ADDR_R 0xD1
#define DS3231_ADDR_W 0xD0

#define FORMAT_LITTLEENDIAN 1
#define FORMAT_BIGENDIAN 2
#define FORMAT_MIDDLEENDIAN 3

#define SQW_RATE_1 0
#define SQW_RATE_1K 1
#define SQW_RATE_4K 2
#define SQW_RATE_8K 3

#define OUTPUT_SQW 0
#define OUTPUT_INT 1

class Time
{
public:
    uint8_t hour;
    uint8_t min;
    uint8_t sec;

    Time();
};

class Date
{
public:
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t dow;

    Date();
};

class DS3231
{
public:
    void begin();
    Time getTime();
    Date getDate();
    void setTime(uint8_t hour, uint8_t min, uint8_t sec);
    void setHour(uint8_t hour);
    void setDate(uint8_t day, uint8_t month, uint16_t year);
    void setDOW();
    void setDOW(uint8_t dow);
    float getTemp();

    // Alarm1
    void setInternAlarm1(const uint8_t sec, const uint8_t min, const uint8_t hour);
    void setShowAlarm1(uint8_t minute, uint8_t hour, uint8_t diff);
    Time getInternAlarm1();
    Time getShowAlarm1(const uint8_t diff);
    void clearAlarm1Flag();
    uint8_t triggeredAlarm1();
    void setAlarm1Interrupt(bool enable);

    // Alarm2
    void setInternAlarm2(const uint8_t min, const uint8_t hour);
    void setShowAlarm2(uint8_t minute, uint8_t hour, uint8_t diff);
    Time getInternAlarm2();
    Time getShowAlarm2(const uint8_t diff);
    void clearAlarm2Flag();
    uint8_t triggeredAlarm2();
    void setAlarm2Interrupt(bool enable);

    void forceTemperatureUpdate();
    float getTemperature();

    long getUnixTime(Time t, Date d);

    void enable32KHz(bool enable);
    void setOutput(bool enable);
    void setSQWRate(int rate);

    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t decToBcd(uint8_t val);
    uint8_t bcdToDec(uint8_t val);
};
#endif
