/*
Version 2.1 - 
-change thermometer parameter from float to uint8_t
-fix blink state (DISPLAY_ALARM2 -> CLOCK)

Version 2.0 - 16. April 2017
-button interupts
-alarm interrupt
-int main & while(1)
-set alarm minute steps from 1min to 5mins
-fix late start up
-without Arduion-libs (Arduino.h & Wire.h)
-all variable & function names in english

----------------------------------------------------------------------

Version 1.8 - 13. November 2016
-LED fade when alarm will be turning off
-LED_PROZENT counter=5
-refractoring most german words to english words
-change alarmMode automatically, when alarmtime is changed

Version 1.7 - 18. September 2016
-fix snooze, alarmAttemps

Version 1.6 - 11. September 2016
-fix display doesnt go inactive, when its 0:00
-add turnOn & turnOff fades
-add fading Colors

Version 1.5 - 21. August 2016
-big code optimization (switch-case)
-fix night-switching
-display is now active, if the LED strip is turned on by remote control

Version 1.4 - 20. August 2016
-fix doublepoints, when get off from standby
-fix LED-Brightness steps
-fix display inactive, when LED Strip is on and its 0:00

Version 1.3 - 07. Juli 2016
-set snooze long press ticks to 500
-fix blink doublepoints
-change LED-Brightness steps from 2 to 5
-display is now active, if the LED strip is turned on

Version 1.2 - 14. Juni 2016
-add statistics

Version 1.1 -  01. Juni 2016
-fix Standby-Helligkeit
-change long press ticks from 1000 to 750
-change datatypes in OneButton-Library
-add JUMP3 & JUMP7 & QUICK & SLOW keys
-change LED final color

Version 1.0 - 29. Mai 2016
*/


#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>

void snooze_click();
void snooze_longPressStart();
void button1_click();
void button1_longPressStart();
void button2_click();
void button2_longPressStart();
void button2_duringLongPress();
void vibrate(bool vibr);
void vibration_sequence();
uint8_t adjustHour(uint8_t hour);
uint8_t adjustMinute(uint8_t min);
//void printClock(Time t);
//void printDate(Date t);

void saveStatistics();
void updateEEPROM(uint8_t index, uint8_t value);
uint16_t readSavedValue(uint8_t index);

//void showStartMessages();
void IREvent(uint8_t protocol, uint16_t address, uint32_t command);


#endif /* MAIN_H_ */