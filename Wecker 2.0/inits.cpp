/*
 * inits.cpp
 *
 * Created: 17.04.2017 00:41:35
 *  Author: Maxi
 */

#include "inits.h"
#include "DS3231.h"
#include "OneButton2.h"
#include "avr/io.h"
#include "main.h"
#include <stdint.h>

extern OneButton snooze;
extern OneButton button1;
extern OneButton button2;

extern DS3231 rtc;

void init_buttons()
{
    snooze.attachClick(snooze_click);
    snooze.attachLongPressStart(snooze_longPressStart);
    snooze.setPressTicks(500);

    button1.attachClick(button1_click);
    button1.attachLongPressStart(button1_longPressStart);

    button2.attachClick(button2_click);
    button2.attachLongPressStart(button2_longPressStart);
    button2.attachDuringLongPress(button2_duringLongPress);
}

void init_rtc()
{
    rtc.begin();
    rtc.enable32KHz(false);
    rtc.setOutput(OUTPUT_INT);
    rtc.clearAlarm1Flag();
    rtc.clearAlarm2Flag();
    rtc.setAlarm1Interrupt(true);
    rtc.setAlarm2Interrupt(true);
}

void init_PWM()
{
    TCCR1A = (1 << WGM10);              // 8-Bit 256 Schritte
    TCCR1B = (1 << CS01) | (0 << CS00); // Prescaler 8

    TCCR2A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10); // 8-Bit 256 Schritte
    TCCR2B = (1 << CS01) | (0 << CS00);                    // Prescaler 8
}
