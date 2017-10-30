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

//#include <Wire.h>
#include "DS3231.h"
#include "avr/io.h"
#include "i2cmaster.h"
#include "LED.h"

#define SDA PINC4
#define SCL	PINC5

static const uint8_t dim[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

/* Public */

Time::Time()
{
	this->hour = 0;
	this->min = 0;
	this->sec = 0;
}
Date::Date()
{
	this->year = 2017;
	this->month = 1;
	this->day = 1;
	this->dow = 1;
}
void DS3231::begin()
{
	i2c_init();
}

Time DS3231::getTime()
{
	Time t;
	
	i2c_start_wait(REG_I2C);			// set device address and write mode
	i2c_write(REG_SEC);					// write address = REG_SEC

	i2c_rep_start(REG_I2C+I2C_READ);	// set device address and read mode
	t.sec=bcdToDec(i2c_readAck());		// read one byte from address REG_SEC
	t.min=bcdToDec(i2c_readAck());		//  "    "    "    "     "    REG_SEC+1
	t.hour=bcdToDec(i2c_readNak());		//  "    "    "    "     "    REG_SEC+2

	i2c_stop();							// set stop condition = release bus

	return t;
}

Date DS3231::getDate()
{
	Date result;

	i2c_start_wait(REG_I2C);					// set device address and write mode
	i2c_write(REG_DOW);							// write address = REG_DOW

	i2c_rep_start(REG_I2C+I2C_READ);			// set device address and read mode
	result.dow=bcdToDec(i2c_readAck());			// read one byte from address REG_DOW
	result.day=bcdToDec(i2c_readAck());			//  "    "    "    "     "    REG_DOW+1
	result.month=bcdToDec(i2c_readAck());       //  "    "    "    "     "    REG_DOW+3
	result.year=2000+bcdToDec(i2c_readNak());	//  "    "    "    "     "    REG_DOW+3

	i2c_stop();									// set stop condition = release bus

	return result;
}

void DS3231::setTime(uint8_t hour, uint8_t min, uint8_t sec)
{
	if (((hour >= 0) && (hour < 24)) && ((min >= 0) && (min < 60)) && ((sec >= 0) && (sec < 60)))
	{
		i2c_start_wait(REG_I2C);	// set device address and write mode
		i2c_write(REG_SEC);			// write address = REG_SEC
		i2c_write(decToBcd(sec));	// write sec to address REG_SEC
		i2c_write(decToBcd(min));	// write min to address REG_SEC+1=REG_MIN
		i2c_write(decToBcd(hour));	// write hour to address REG_SEC+2=REG_HOUR
		i2c_stop();					// set stop condition = release bus
	}
}

void DS3231::setHour(uint8_t hour)
{
	if (((hour >= 0) && (hour < 24)))
	{
		writeRegister(REG_HOUR, decToBcd(hour));
	}
}

void DS3231::setDate(uint8_t day, uint8_t month, uint16_t year)
{
	if (((day >= 1) && (day <= 31)) && ((month >= 1) && (month <= 12)) && ((year >= 2000) && (year < 3000)))
	{
		year -= 2000;

		i2c_start_wait(REG_I2C);	// set device address and write mode
		i2c_write(REG_DAY);			// write address = REG_DAY
		i2c_write(decToBcd(day));	// write day to address REG_DAY
		i2c_write(decToBcd(month));	// write month to address REG_DAY+1=REG_MONTH
		i2c_write(decToBcd(year));	// write year to address REG_DAY+2=REG_YEAR
		i2c_stop();					// set stop condition = release bus
	}
}

void DS3231::setDOW()
{
	int dow;
	uint8_t mArr[12] = { 6,2,2,5,0,3,5,1,4,6,2,4 };
	Date _t = getDate();

	dow = (_t.year % 100);
	dow = dow*1.25;
	dow += _t.day;
	dow += mArr[_t.month - 1];
	if (((_t.year % 4) == 0) && (_t.month < 3))
	dow -= 1;
	while (dow > 7)
	dow -= 7;
	writeRegister(REG_DOW, dow);
}

void DS3231::setDOW(uint8_t dow)
{
	if ((dow > 0) && (dow < 8))
	writeRegister(REG_DOW, dow);
}

//Alarm1

void DS3231::setInternAlarm1(const uint8_t sec, const uint8_t min, const uint8_t hour)
{
	i2c_start_wait(REG_I2C);			// set device address and write mode
	i2c_write(REG_ALARM1);			// write address = REG_ALARM1
	i2c_write(decToBcd((sec)));		// write data to address REG_ALARM1
	i2c_write(decToBcd((min)));		// write data to address REG_ALARM1+1
	i2c_write(decToBcd((hour)));	// write data to address REG_ALARM1+2
	i2c_write(decToBcd((0x80)));	// write data to address REG_ALARM1+3
	i2c_stop();						// set stop condition = release bus
}
void DS3231::setShowAlarm1(uint8_t minute, uint8_t hour, uint8_t diff)
{
	if (minute - diff < 0)
	{
		if (hour != 0)
		hour--;
		else
		hour = 23;
		minute = 60 + minute - diff;
	}
	else
	minute -= diff;

	setInternAlarm1(0, minute, hour);
}

Time DS3231::getInternAlarm1()
{
	Time result;

	i2c_start_wait(REG_I2C);				// set device address and write mode
	i2c_write(REG_ALARM1);					// write address = REG_ALARM1

	i2c_rep_start(REG_I2C+I2C_READ);		// set device address and read mode
	result.sec=bcdToDec(i2c_readAck());		// read one byte from address REG_ALARM1
	result.min = bcdToDec(i2c_readAck());	// read one byte from address REG_ALARM1+1
	result.hour = bcdToDec(i2c_readNak());	// read one byte from address REG_ALARM1+2

	i2c_stop();								// set stop condition = release bus

	return result;
}

Time DS3231::getShowAlarm1(uint8_t diff)
{
	Time result = getInternAlarm1();
	if (result.min + diff >= 60)
	{
		if (result.hour != 23)
		result.hour++;
		else
		result.hour = 0;
		result.min = result.min + diff - 60;
	}
	else
	result.min += diff;

	return result;
}

uint8_t DS3231::triggeredAlarm1()
{
	return (readRegister(REG_STATUS) >> BIT_A1F) & 1;
}

void DS3231::clearAlarm1Flag()
{
	writeRegister(REG_STATUS, readRegister(REG_STATUS) & ~(1 << BIT_A1F));
}

void DS3231::setAlarm1Interrupt(bool enable)
{
	uint8_t _reg = readRegister(REG_CONTROL);
	_reg &= ~(1 << 0);
	_reg |= (enable << 0);
	writeRegister(REG_CONTROL, _reg);
}

//Alarm2

void DS3231::setInternAlarm2(const uint8_t min, const uint8_t hour)
{
	i2c_start_wait(REG_I2C);		// set device address and write mode
	i2c_write(REG_ALARM2);			// write address = REG_ALARM2
	i2c_write(decToBcd((min)));		// write data to address REG_ALARM2
	i2c_write(decToBcd((hour)));	// write data to address REG_ALARM2+1
	i2c_write(decToBcd((0x80)));	// write data to address REG_ALARM1+2
	i2c_stop();						// set stop condition = release bus
}

void DS3231::setShowAlarm2(uint8_t minute, uint8_t hour, uint8_t diff)
{
	if (minute - diff < 0)
	{
		if (hour != 0)
		hour--;
		else
		hour = 23;
		minute = 60 + minute - diff;
	}
	else
	minute -= diff;

	setInternAlarm2(minute, hour);
}

Time DS3231::getInternAlarm2()
{
	Time result;

	i2c_start_wait(REG_I2C);				// set device address and write mode
	i2c_write(REG_ALARM2);					// write address = REG_ALARM1

	i2c_rep_start(REG_I2C+I2C_READ);		// set device address and read mode
	result.min = bcdToDec(i2c_readAck());	// read one byte from address REG_ALARM1
	result.hour = bcdToDec(i2c_readNak());	// read one byte from address REG_ALARM1+1

	i2c_stop();								// set stop condition = release bus

	return result;
}

Time DS3231::getShowAlarm2(uint8_t diff)
{
	Time t = getInternAlarm2();
	if (t.min + diff >= 60)
	{
		if (t.hour != 23)
		t.hour++;
		else
		t.hour = 0;
		t.min = t.min + diff - 60;
	}
	else
	t.min += diff;

	return t;
}

uint8_t DS3231::triggeredAlarm2()
{
	return (readRegister(REG_STATUS) >> BIT_A2F) & 1;
}

void DS3231::clearAlarm2Flag()
{
	writeRegister(REG_STATUS, readRegister(REG_STATUS) & ~(1 << BIT_A2F));
}

void DS3231::setAlarm2Interrupt(bool enable)
{
	uint8_t _reg = readRegister(REG_CONTROL);
	_reg &= ~(1 << 1);
	_reg |= (enable << 1);
	writeRegister(REG_CONTROL, _reg);
}

/**
* Forces the device to update the temperature.
*
* This method forces the temperature sensor to convert the temperature into digital code and execute the TCXO
* algorithm to update the capacitance array of the oscillator.
*
* Since an update can only happen when a conversion is not already in progress
*/
void DS3231::forceTemperatureUpdate()
{
	writeRegister(REG_CONTROL, readRegister(REG_CONTROL) | REG_CONTROL_CONV);
}

/**
* Reads current temperature in degrees Celsius.
*
* This method reads the temperature from the temperature registers in degrees Celsius. Bear in mind that these
* registers are updated every 64-seconds or after forceTemperatureUpdate() is called. Therefore, if you try
* to call this method multiple times, within 64-seconds (without forcing temperature update), the result will be
* the same.
*
* The resolution of this device is 0.25ยบC.
*
* @return The temperature in degrees Celsius.
*/
float DS3231::getTemperature()
{
	uint8_t decimalPart, resolution;
	float temperature;

	i2c_start_wait(REG_I2C);			// set device address and write mode
	i2c_write(REG_TEMP);				// write address = REG_TEMP

	i2c_rep_start(REG_I2C+I2C_READ);	// set device address and read mode
	decimalPart = i2c_readAck();		// read one byte from address REG_TEMP
	resolution = i2c_readNak();			// read one byte from address REG_TEMP+1

	i2c_stop();							// set stop condition = release bus

	resolution = resolution >> 6;		//get only the upper nibble
	temperature = (decimalPart & 0x80)  //negative (two's complement)?
	? (decimalPart | ~((1 << 8) - 1))	//convert it to a signed int
	: decimalPart;						//positive number

	return temperature + (resolution * 0.25);
}


long DS3231::getUnixTime(Time t, Date d)
{
	uint16_t	dc;

	dc = d.day;
	for (uint8_t i = 0; i < (d.month - 1); i++)
	dc += dim[i];
	if ((d.month > 2) && (((d.year - 2000) % 4) == 0))
	++dc;
	dc = dc + (365 * (d.year - 2000)) + (((d.year - 2000) + 3) / 4) - 1;

	return ((((((dc * 24L) + t.hour) * 60) + t.min) * 60) + t.sec) + SEC_1970_TO_2000;

}

void DS3231::enable32KHz(bool enable)
{
	uint8_t _reg = readRegister(REG_STATUS);
	_reg &= ~(1 << 3);
	_reg |= (enable << 3);
	writeRegister(REG_STATUS, _reg);
}

void DS3231::setOutput(bool enable)
{
	uint8_t _reg = readRegister(REG_CONTROL);
	_reg &= ~(1 << 2);
	_reg |= (enable << 2);
	writeRegister(REG_CONTROL, _reg);
}

void DS3231::setSQWRate(int rate)
{
	uint8_t _reg = readRegister(REG_CONTROL);
	_reg &= ~(3 << 3);
	_reg |= (rate << 3);
	writeRegister(REG_CONTROL, _reg);
}


/* Private */

uint8_t DS3231::readRegister(uint8_t reg)
{
	uint8_t readValue = 0;
	
	i2c_start_wait(REG_I2C);			// set device address and write mode
	i2c_write(reg);						// write address = reg
	i2c_rep_start(REG_I2C+I2C_READ);	// set device address and read mode
	readValue=i2c_readNak();			// read one byte from address reg
	i2c_stop();							// set stop condition = release bus

	return readValue;
}

void DS3231::writeRegister(uint8_t reg, uint8_t value)
{
	i2c_start_wait(REG_I2C);	// set device address and write mode
	i2c_write(reg);				// write address = reg
	i2c_write(value);			// write value to address reg
	i2c_stop();					// set stop condition = release bus
}

uint8_t DS3231::decToBcd(uint8_t val)
{
	return ((val / 10 * 16) + (val % 10));
}

uint8_t DS3231::bcdToDec(uint8_t val)
{
	return ((val / 16 * 10) + (val % 16));
}