/*
 * LED.h
 *
 * Created: 15.04.2017 13:19:57
 *  Author: Maxi
 */

#ifndef LED_H_
#define LED_H_

#include "RGB_vector.h"
#include "weckerEnums.h"

// change directly the color of the LED_Strip
void LED(RGB_vector vector);
void LED(uint8_t red, uint8_t green, uint8_t blue);

// change the color of the LED-Srip with a brightness-factor
void LED_with_factor(RGB_vector &vector);
void LED_with_factor(RGB_vector &vector, uint8_t currenPercent);
void LED_with_factor(uint8_t red, uint8_t green, uint8_t blue);
void LED_with_factor(uint8_t red, uint8_t green, uint8_t blue, uint8_t currentPercent);

RGB_vector LED_with_factor_noChange(RGB_vector &rgb, uint8_t currentPercent);
RGB_vector LED_with_factor_noChange(uint8_t red, uint8_t green, uint8_t blue,
                                    uint8_t currentPercent);

RGB_vector LED_with_factor_NEW(uint8_t red, uint8_t green, uint8_t blue, uint8_t currentPercent);

// generate with LED_sequence and LED_sunset a sunset for wakeup on the
// LED-Strip
void LED_sequence();
void LED_sunset(uint8_t timer);

// indicate the current alarm mode
void showAlarmLED(AlarmModes aM);

// turn on/off LED-Strip with a fading
void turnOnLED();
void turnOffLED();

// help functions to fade LED on/off
void turningOn();
void turningOff();

// apply a fade to another color
void fadeToColor(RGB_vector &color);
void fadeToColor(uint8_t red, uint8_t green, uint8_t blue);

// help function to fade LED to another color
void fadingColor();

// function to apply effects on LED strip
void LED_effects();

#endif /* LED_H_ */
