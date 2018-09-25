/*
 * LED.cpp
 *
 * Created: 15.04.2017 13:19:41
 *  Author: Maxi
 */

#include "LED.h"
#include "avr/io.h"
#include "bit_manipulation.h"
#include "math.h"
#include "timer0.h"

#define LED_UPDATETIME 15000 // every 15 seconds
#define LED_A1 PIND4         // D4
#define LED_A2 PIND5         // D5
#define LED_TURN_ON_OFF_DELAY 3

#define RED_PIN OCR1A   // 9 - B1 - gr�nes Kabel
#define GREEN_PIN OCR2A // 11 - B3 - blaues Kabel
#define BLUE_PIN OCR1B  // 10 - B2 - rotes Kabel

RGB_vector LED_currentColor;
RGB_vector LED_turningColor;
RGB_vector LED_lastColor;
RGB_vector firstRGB;

int diffRed;
int diffGreen;
int diffBlue;

uint16_t LED_speed = 1000;

uint8_t LED_timer;
uint8_t LED_percent = 100;
uint8_t LED_turnPercent = 1;

bool LED_Power = false;

LED_States LED_State = LED_States::NORMAL;
Color_States currentColor = Color_States::WHITE;

extern unsigned long prevTime2;
extern unsigned long prevTime3;
extern unsigned long snoozeStartTime;

extern uint8_t counter;
extern uint8_t seconds;

extern bool snoozeState;
extern bool blinkState;

extern AlarmStates alarmState;
extern States currentState;
extern Switching_States currentSwitch;

void LED(RGB_vector vector)
{
    LED(vector.red, vector.green, vector.blue);
}

void LED(uint8_t red, uint8_t green, uint8_t blue)
{
    if (red == 0)
    {
        TCCR1A &= ~(1 << COM1A1); // von PWM abkoppeln
        clear_bit(PORTB, 1);
    }
    else
    {
        TCCR1A |= (1 << COM1A1); // PWM aktivieren
        RED_PIN = red;
    }

    if (blue == 0)
    {
        TCCR1A &= ~(1 << COM1B1); // von PWM abkoppeln
        clear_bit(PORTB, 2);
    }
    else
    {
        TCCR1A |= (1 << COM1B1); // PWM aktivieren
        BLUE_PIN = blue;
    }

    if (green == 0)
    {
        TCCR2A &= ~(1 << COM1A1); // von PWM abkoppeln
        clear_bit(PORTB, 3);
    }
    else
    {
        TCCR2A |= (1 << COM1A1); // PWM aktivieren
        GREEN_PIN = green;
    }
}

void LED_with_factor(RGB_vector &vector)
{
    LED_with_factor(vector.red, vector.green, vector.blue);
}

void LED_with_factor(RGB_vector &vector, uint8_t currenPercent)
{
    LED_with_factor(vector.red, vector.green, vector.blue, currenPercent);
}

void LED_with_factor(uint8_t red, uint8_t green, uint8_t blue)
{
    LED_with_factor(red, green, blue, LED_percent);
}

void LED_with_factor(uint8_t red, uint8_t green, uint8_t blue, uint8_t currentPercent)
{
    LED(LED_with_factor_noChange(red, green, blue, currentPercent));
}

RGB_vector LED_with_factor_noChange(RGB_vector &rgb, uint8_t currentPercent)
{
    return LED_with_factor_noChange(rgb.red, rgb.green, rgb.blue, currentPercent);
}

RGB_vector LED_with_factor_noChange(uint8_t red, uint8_t green, uint8_t blue,
                                    uint8_t currentPercent)
{
    float factor;

    if (currentPercent == 0)
    {
        uint8_t smVal; // kleinster Wert
        if (red != 0)
        {
            smVal = red;
        }
        else if (green != 0)
        {
            smVal = green;
        }
        else if (blue != 0)
        {
            smVal = blue;
        }
        if (green < smVal && green != 0)
        {
            smVal = green;
        }
        if (blue < smVal && blue != 0)
        {
            smVal = blue;
        }

        factor = 1 / (float)smVal;
    }
    else
    {
        factor = ((currentPercent / 100.0f) * 255) / 255.0F;
    }

    return {round(red * factor), round(green * factor), round(blue * factor)};
}

void LED_sequence()
{
    if ((millis() - prevTime3) >= LED_UPDATETIME) // LED aktualisieren
    {
        prevTime3 = millis();

        LED_sunset(LED_timer);
        LED_timer++;
    }
    if (LED_timer > 120)
    {
        currentState = States::CLOCK;
        counter = 0;
        snoozeState = false;
        prevTime2 = millis();
        prevTime3 = 0;
        snoozeStartTime = millis();
        seconds = 0;

        if (alarmState == AlarmStates::ALARM1_LED)
        {
            alarmState = AlarmStates::ALARM1_VIBR;
        }

        else if (alarmState == AlarmStates::ALARM2_LED)
        {
            alarmState = AlarmStates::ALARM2_VIBR;
        }
    }
}

void LED_sunset(uint8_t timer)
{
    if (timer < 10)
    {
        LED_currentColor.red++;
    }
    else if (timer < 15)
    {
        LED_currentColor.red += 2;
    }
    else if (timer < 21)
    {
        LED_currentColor.red += 2;
        LED_currentColor.green++;
    }
    else if (timer < 25)
    {
        LED_currentColor.red += 2;
        LED_currentColor.green++;
        LED_currentColor.blue++;
    }
    else if (timer < 30)
    {
        LED_currentColor.red += 5;
        LED_currentColor.green++;
        LED_currentColor.blue++;
    }
    else if (timer < 36)
    {
        LED_currentColor.red += 5;
        LED_currentColor.green += 5;
        LED_currentColor.blue++;
    }
    else if (timer < 52)
    {
        LED_currentColor.red += 5;
        LED_currentColor.green += 5;
        LED_currentColor.blue += 5;
    }
    else if (timer < 54)
    {
        LED_currentColor.red += 10;
        LED_currentColor.green += 5;
        LED_currentColor.blue += 5;
    }
    else if (timer < 60)
    {
        LED_currentColor.red += 10;
        LED_currentColor.green += 5;
        LED_currentColor.blue += 10;
    }
    else if (timer >= 60)
    {
        LED_currentColor.red = 255;
        LED_currentColor.green = 165;
        LED_currentColor.blue = 165;
    }
    LED(LED_currentColor);
}

void showAlarmLED(AlarmModes aM)
{
    switch (aM)
    {
        case AlarmModes::INACTIVE:
            PORTD &= ~(1 << LED_A1); // 0
            PORTD &= ~(1 << LED_A2); // 0
            break;

        case AlarmModes::ALARM1_ACTIVE:
            PORTD |= (1 << LED_A1);  // 1
            PORTD &= ~(1 << LED_A2); // 0
            break;

        case AlarmModes::ALARM2_ACTIVE:
            PORTD &= ~(1 << LED_A1); // 0
            PORTD |= (1 << LED_A2);  // 1
            break;

        case AlarmModes::ALARM1_ALARM2_ACTIVE:
            PORTD |= (1 << LED_A1); // 1
            PORTD |= (1 << LED_A2); // 1
            break;
    }
}

void turnOnLED()
{
    LED_turningColor = LED_with_factor_noChange(LED_lastColor, LED_percent);
    prevTime3 = 0;
    currentSwitch = Switching_States::TURNING_ON;
    LED_turnPercent = 1;

    LED_State = LED_States::NORMAL;
    if (currentState == States::STANDBY)
    {
        currentState = States::CLOCK;
        blinkState = false;
        counter = 5;
    }

    turningOn();
}

void turnOffLED()
{
    LED_turningColor = LED_with_factor_noChange(LED_lastColor, LED_percent);
    prevTime3 = 0;
    currentSwitch = Switching_States::TURNING_OFF;
    LED_turnPercent = 100;

    if (counter > 5)
        counter = 5;

    turningOff();
}

void turningOn()
{
    if ((millis() - prevTime3) >= LED_TURN_ON_OFF_DELAY)
    {
        prevTime3 = millis();
        LED(LED_with_factor_noChange(LED_turningColor.red, LED_turningColor.green,
                                     LED_turningColor.blue, LED_turnPercent));
        if (LED_turnPercent >= 100)
        {
            currentSwitch = Switching_States::NO;
            LED_Power = true;
        }
        else
            LED_turnPercent++;
    }
}

void turningOff()
{
    if ((millis() - prevTime3) >= LED_TURN_ON_OFF_DELAY)
    {
        prevTime3 = millis();
        LED(LED_with_factor_noChange(LED_turningColor.red, LED_turningColor.green,
                                     LED_turningColor.blue, LED_turnPercent));
        LED_turnPercent--;
        if (LED_turnPercent <= 0)
        {
            currentSwitch = Switching_States::NO;
            LED_Power = false;
            LED(0, 0, 0);
        }
    }
}
void fadeToColor(RGB_vector &color)
{
    fadeToColor(color.red, color.green, color.blue);
}

void fadeToColor(uint8_t red, uint8_t green, uint8_t blue)
{
    currentSwitch = Switching_States::FADING;

    LED_turningColor.setVector(red, green, blue);

    firstRGB = LED_with_factor_noChange(LED_lastColor, LED_percent);
    RGB_vector secondRGB = LED_with_factor_noChange(LED_turningColor, LED_percent);

    diffRed = firstRGB.red - secondRGB.red;
    diffGreen = firstRGB.green - secondRGB.green;
    diffBlue = firstRGB.blue - secondRGB.blue;

    LED_turnPercent = 1;

    fadingColor();
}

void fadingColor()
{
    if ((millis() - prevTime3) >= LED_TURN_ON_OFF_DELAY)
    {
        prevTime3 = millis();

        RGB_vector currentRGB;
        currentRGB.red = firstRGB.red - trunc((LED_turnPercent / 255.0F) * diffRed);
        currentRGB.green = firstRGB.green - trunc((LED_turnPercent / 255.0F) * diffGreen);
        currentRGB.blue = firstRGB.blue - trunc((LED_turnPercent / 255.0F) * diffBlue);

        LED(currentRGB);

        if (LED_turnPercent < 255)
            LED_turnPercent++;
        else
        {
            currentSwitch = Switching_States::NO;
            LED_lastColor = LED_turningColor;
            LED_with_factor(LED_lastColor);
        }
    }
}

void LED_effects()
{
    if (LED_State != LED_States::NORMAL && LED_Power)
    {
        switch (LED_State)
        {
            case LED_States::JUMP3:
                if ((millis() - prevTime3) >= LED_speed)
                {
                    prevTime3 = millis();

                    if (currentColor == Color_States::WHITE || currentColor == Color_States::BLUE)
                    {
                        currentColor = Color_States::RED;
                        LED_with_factor(255, 0, 0);
                    }
                    else if (currentColor == Color_States::RED)
                    {
                        currentColor = Color_States::GREEN;
                        LED_with_factor(0, 255, 0);
                    }
                    else if (currentColor == Color_States::GREEN)
                    {
                        currentColor = Color_States::BLUE;
                        LED_with_factor(0, 0, 255);
                    }
                }
                break;

            case LED_States::JUMP7:
                if ((millis() - prevTime3) >= LED_speed)
                {
                    prevTime3 = millis();

                    if (currentColor == Color_States::WHITE || currentColor == Color_States::PURPLE)
                    {
                        currentColor = Color_States::RED;
                        LED_with_factor(255, 0, 0);
                    }
                    else if (currentColor == Color_States::RED)
                    {
                        currentColor = Color_States::YELLOW;
                        LED_with_factor(255, 255, 0);
                    }
                    else if (currentColor == Color_States::YELLOW)
                    {
                        currentColor = Color_States::GREEN;
                        LED_with_factor(0, 255, 0);
                    }
                    else if (currentColor == Color_States::GREEN)
                    {
                        currentColor = Color_States::TUERKIS;
                        LED_with_factor(0, 255, 255);
                    }
                    else if (currentColor == Color_States::TUERKIS)
                    {
                        currentColor = Color_States::BLUE;
                        LED_with_factor(0, 0, 255);
                    }
                    else if (currentColor == Color_States::BLUE)
                    {
                        currentColor = Color_States::PURPLE;
                        LED_with_factor(255, 0, 255);
                    }
                }
                break;
        }
    }
}
