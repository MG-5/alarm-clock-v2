#ifndef MAIN_H_
#define MAIN_H_

#include "DS3231.h"
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
void printClock(Time t);
void printDate(Date t);

void saveStatistics();
void updateEEPROM(uint8_t index, uint8_t value);
uint16_t readSavedValue(uint8_t index);

void showStartMessages();
void setNightmode(bool value);
void IREvent(uint8_t protocol, uint16_t address, uint32_t command);

#endif /* MAIN_H_ */
