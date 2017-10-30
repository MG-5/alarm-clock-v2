/*
 * display.h
 *
 * Created: 15.04.2017 14:36:40
 *  Author: Maxi
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "DS3231.h"

//select automatically the right brightness in dependence of night state
void autoselectBrightness();

//display the actual time
void clock(uint8_t hour, uint8_t minute, bool doublepoint);
void clock(Time &ttime, bool doublepoint);

//display any number between 0-100
void display_Percent(uint8_t percent);

//display the temperatur in Celcius °C
void thermometer(uint8_t value);


#endif /* DISPLAY_H_ */