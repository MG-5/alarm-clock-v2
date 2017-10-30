/*
* IR_receiver.cpp
*
* Created: 16.04.2017 23:59:45
*  Author: Maxi
*/

#include "LED.h"
#include "avr/interrupt.h"
#include "IRLremote.h"

uint8_t IRProtocol = 0;
uint16_t IRAddress = 0;
uint16_t IRCommand = 0;
uint16_t lastCommand = 0;

extern uint8_t remote_received;
extern bool LED_Power;
extern uint8_t LED_percent;
extern uint8_t counter;
extern unsigned long prevTime;
extern States currentState;
extern LED_States LED_State;
extern AlarmStates alarmState;
extern Switching_States currentSwitch;

extern Color_States currentColor;
extern unsigned long prevTime3;

extern uint16_t LED_speed;
extern RGB_vector LED_lastColor;


void poll_IR()
{
	if (IRProtocol == 3 && alarmState == AlarmStates::OFF && currentSwitch == Switching_States::NO)
	{
		cli();

		if (IRAddress == 0xFF00 && IRCommand < 0xFFFF)
		{
			lastCommand = IRCommand;
			remote_received++;
			bool noBlock = true; //LED-Aktualisierung

			if (LED_Power)
			{
				switch (IRCommand)
				{
					case 0xBF40: //AN-AUS
					turnOffLED();
					break;

					case 0xA35C: //HELLER
					if (LED_percent < 100)
					LED_percent += 5;

					currentState = States::LED_PERCENT;
					counter = 0;
					prevTime = 0;

					noBlock = false;
					if (LED_State == LED_States::NORMAL)
					LED_with_factor(LED_lastColor);
					break;

					case 0xA25D: //DUNKLER
					if (LED_percent > 0)
					LED_percent -= 5;

					currentState = States::LED_PERCENT;
					counter = 0;
					prevTime = 0;

					noBlock = false;
					if (LED_State == LED_States::NORMAL)
					LED_with_factor(LED_lastColor);
					break;

					case 0xBE41: //Testroutine
					currentState = States::TEST;
					counter = 0;
					prevTime = 0;
					noBlock = false;
					break;	

					case 0xA758: //ROT
					fadeToColor(255, 0, 0);
					break;

					case 0xA659: //GRÜN
					fadeToColor(0, 255, 0);
					break;

					case 0xBA45: //BLAU
					fadeToColor(0, 0, 255);
					break;

					case 0xBB44: //WEIß
					fadeToColor(255, 80, 80);
					break;

					case 0xAB54: //A3
					fadeToColor(255, 10, 0);
					break;

					case 0xAA55: //B3
					fadeToColor(0, 255, 20);
					break;

					case 0xB649: //C3
					fadeToColor(0, 100, 255);
					break;

					case 0xB748: //D3
					fadeToColor(255, 60, 20);
					break;

					case 0xAF50: //A4
					fadeToColor(255, 20, 0);
					break;

					case 0xAE51: //B4
					fadeToColor(0, 255, 60);
					break;

					case 0xB24D: //C4
					break;

					case 0xB34C: //D4
					fadeToColor(255, 60, 20);
					break;

					case 0xE31C: //A5
					fadeToColor(255, 30, 0);
					break;

					case 0xE21D: //B5
					fadeToColor(0, 255, 150);
					break;

					case 0xE11E: //C5
					break;

					case 0xE01F: //D5
					fadeToColor(255, 165, 165);
					break;

					case 0xE718: //A6
					fadeToColor(255, 40, 0);
					break;

					case 0xE619: //B6
					fadeToColor(0, 255, 255);
					break;

					case 0xE51A: //C6
					fadeToColor(255, 0, 30);
					break;

					case 0xE41B: //D6
					fadeToColor(255, 255, 255);
					break;

					case 0xFB04: //JUMP3
					if (LED_State != LED_States::JUMP3)
					{
						LED_State = LED_States::JUMP3;
						currentColor = Color_States::WHITE;
						prevTime3 = 0;
						noBlock = false;
					}
					break;

					case 0xFA05: //JUMP7
					if (LED_State != LED_States::JUMP7)
					{
						LED_State = LED_States::JUMP7;
						currentColor = Color_States::WHITE;
						prevTime3 = 0;
						noBlock = false;
					}
					break;

					case 0xE817: //QUICK
					if (LED_speed > 50)
					LED_speed -= 50;

					noBlock = false;
					break;

					case 0xEC13: //SLOW
					if (LED_speed < 1000)
					LED_speed += 50;

					noBlock = false;
					break;
				}
				if (LED_Power && noBlock && LED_State != LED_States::FLASH)
				{
					LED_with_factor(LED_lastColor);
					LED_State = LED_States::NORMAL;
				}
			}
			else if (IRCommand == 0xBF40)
			{
				turnOnLED();
			}
			// reset variable to not read the same value twice
			IRProtocol = 0;

		}
		else if (IRAddress == 0 && IRCommand == 0xFFFF)
		{
			switch (lastCommand)
			{
				case 0xA35C: //HELLER
				if (LED_percent < 100)
				LED_percent += 5;

				if (LED_State == LED_States::NORMAL)
				LED_with_factor(LED_lastColor);
				currentState = States::LED_PERCENT;
				counter = 0;
				prevTime = 0;
				break;

				case 0xA25D: //DUNKLER
				if (LED_percent > 0)
				LED_percent -= 5;

				if (LED_State == LED_States::NORMAL)
				LED_with_factor(LED_lastColor);
				currentState = States::LED_PERCENT;
				counter = 0;
				prevTime = 0;
				break;

				case 0xE817: //QUICK
				if (LED_speed > 50)
				LED_speed -= 50;

				break;

				case 0xEC13: //SLOW
				if (LED_speed < 1000)
				LED_speed += 50;

				break;

			}
			// reset variable to not read the same value twice
			IRProtocol = 0;
		}
		sei();
	}

	switch (currentSwitch)
	{
		case Switching_States::TURNING_ON:
		turningOn();
		break;

		case Switching_States::TURNING_OFF:
		turningOff();
		break;

		case Switching_States::FADING:
		fadingColor();
		break;
	}
}

void IREvent(uint8_t protocol, uint16_t address, uint16_t command)
{
	if (protocol == 3) {
		// update the values to the newest valid input
		IRProtocol = protocol;
		IRAddress = address;
		IRCommand = command;
	}
}

ISR(INT0_vect)
{
	IRLinterrupt<IR_NEC>();
}