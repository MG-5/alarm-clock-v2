/*
 * display.cpp
 *
 * Created: 15.04.2017 14:35:40
 *  Author: Maxi
 */

#include "display.h"
#include "TM1637Display.h"
#include "math.h"

extern bool night;
extern TM1637Display tm;

void autoselectBrightness()
{
    if (!night)
        tm.setBrightness(15);
    else
        tm.setBrightness(10);
}

void clock(uint8_t hour, uint8_t minute, bool doublepoint)
{
    Time t;
    t.hour = hour;
    t.min = minute;
    clock(t, doublepoint);
}

void clock(Time &ttime, bool doublepoint)
{
    uint8_t value;
    if (doublepoint)
        value = 128;
    else
        value = 0;

    uint8_t hourFirstNumber;
    uint8_t hourSecondNumber;
    uint8_t minFirstNumber;
    uint8_t minSecondNumber;

    if (ttime.hour == 111)
    {
        hourFirstNumber = 0;
        hourSecondNumber = 0;
    }
    else if (ttime.hour > 9)
    {
        hourFirstNumber = tm.encodeDigit(ttime.hour / 10);
        hourSecondNumber = tm.encodeDigit(ttime.hour % 10);
    }
    else if (ttime.hour <= 9)
    {
        hourFirstNumber = 0;
        hourSecondNumber = tm.encodeDigit(ttime.hour);
    }

    if (ttime.min == 111)
    {
        minFirstNumber = 0;
        minSecondNumber = 0;
    }
    else if (ttime.min > 9)
    {
        minFirstNumber = tm.encodeDigit(ttime.min / 10);
        minSecondNumber = tm.encodeDigit(ttime.min % 10);
    }

    else if (ttime.min <= 9)
    {
        minFirstNumber = tm.encodeDigit(0);
        minSecondNumber = tm.encodeDigit(ttime.min);
    }

    tm.setSegments(hourFirstNumber, hourSecondNumber + value, minFirstNumber, minSecondNumber);
}

void display_Percent(uint8_t percent)
{
    autoselectBrightness();

    if (percent == 100)
        tm.setSegments(0, tm.encodeDigit(1), tm.encodeDigit(0), tm.encodeDigit(0));
    else if (percent > 9)
        tm.setSegments(0, 0, tm.encodeDigit(percent / 10), tm.encodeDigit(percent % 10));
    else if (percent == 0)
        tm.setSegments(0, 0, 0, tm.encodeDigit(1));
    else if (percent <= 9)
        tm.setSegments(0, 0, 0, tm.encodeDigit(percent));
}

void thermometer(uint8_t value)
{
    autoselectBrightness();

    if (value > 9)
        tm.setSegments(tm.encodeDigit(value / 10), tm.encodeDigit(value % 10), 99, 57);
    else if (value <= 9)
        tm.setSegments(0, tm.encodeDigit(value), 99, 57);
}
