/*********
Wecker.cpp
*********/

#include "main.h"
#include "TM1637Display.h"
#include "OneButton2.h"
#include <math.h>

#include <avr/eeprom.h>
#include <avr/power.h>
#include <avr/interrupt.h>

#include "display.h"
#include "timer0.h"
#include "LED.h"
#include "bit_manipulation.h"
#include "infrared.h"
#include "inits.h"

#define VIBR_PIN PINB4 //Vibrationskissen - B4
#define IR_PIN PIND2 //IR-Empfänger - D2

#define LEDSTART 30 //30min vorm Klingeln LED-Leiste anschalten
#define SNOOZETIME 300000 //5 min
#define LED_FINALCOLOR 255, 165, 165 //finale Farbe für den LED-Sonnenaufgang

TM1637Display tm;
DS3231 rtc;

OneButton snooze(PINC, 0); //C0
OneButton button1(PINC, 1); //C1
OneButton button2(PINC, 2); //C2

Time t;
Time alarmTime;
unsigned long prevTime = 0; //Anzeige
unsigned long prevTime2 = 0; //Änderungsgeschwindigkeit bei langem Tastendruck & Vibrieren-Sekundenzähler
unsigned long prevTime3 = 0; //LED & Vibrieren
unsigned long snoozeStartTime = 0;

extern RGB_vector LED_lastColor;
extern RGB_vector LED_currentColor;

extern uint8_t LED_timer;
extern uint8_t LED_percent;
extern uint16_t LED_speed;

uint8_t alarmHour;
uint8_t alarmMinute;
uint8_t alarmAttempts = 0;
uint8_t counter = 0;
uint8_t seconds = 0;

volatile uint8_t buttonStates = 0; //gespeicherte Zustände
volatile uint8_t interruptFlags = 0; //0-snooze		1-button1		2-button2		3-RTC

bool blinkState = true; //Doppelpunkt, Alarm-LED, Alarmzeit einstellen
bool snoozeState = false;
bool vibrState = true;
bool night = false;
extern bool LED_Power;

//Statistik
uint8_t snooze_clicks; //0-1
uint8_t snooze_longclicks; //2-3
uint8_t button1_clicks; //4-5
uint8_t button1_longclicks; //6-7
uint8_t button2_clicks; //8-9
uint8_t button2_longclicks; //10-11
uint8_t remote_received; //12-13
uint8_t A1_triggers; //14-15
uint8_t A2_triggers; //16-17

//States currentState = States::DEBUGMODE;
States currentState = States::CLOCK;
AlarmStates alarmState = AlarmStates::OFF;
AlarmModes alarmMode = AlarmModes::ALARM1_ALARM2_ACTIVE;
extern LED_States LED_State;
extern Color_States currentColor;
Switching_States currentSwitch = Switching_States::NO;

int main(void)
{
	/******Eingänge******/
	DDRC &= 0b11110000; //C0:3
	DDRD &= ~(1 << IR_PIN);

	/**PullUp-Widerstand**/
	PORTC |= 0b00001111; //C0:3
	PORTD |= (1 << IR_PIN); //IR receiver


	/******Ausgänge******/
	DDRB |= 0b00011110; //B1:4

	/*****Interrupts*****/
	EICRA |= (1 << ISC00);
	EIMSK |= (1 << INT0);

	PCICR |= (1 << PCIE1); //PCINT1 aktivieren
	PCMSK1 |= (1 << PCINT8) | (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11); //PCINTs maskieren


	timer0_init(); //Timer0 initialisieren
	init_buttons();

	sei(); //globale Interrupts aktivieren
	init_rtc(); //DS3231
	init_PWM();
	

	/*
	Serial.begin(9600);
	showStartMessages();
	Serial.end();
	*/

	LED_lastColor.setVector(255, 255, 255);

	// Datums- und Zeiteinstellung
	//rtc.setDOW(3);     // Day-of-Week (1-Montag 7-Sonntag)
	//rtc.setTime(17, 53, 0);     // 24h Format hh::mm:ss
	//rtc.setHour(19);
	//rtc.setDate(15, 2, 2017);   // tt.mm.jjjj

	

	//DS3231
	

	//Powersaving
	ADCSRA &= 0b01111111; //Analog to Digital Converter abstellen
	ACSR |= 0b10000000; //Analog Comparator abstellen
	power_adc_disable(); // ADC Converter
	power_spi_disable(); // SPI

	

	/***
	LOOP
	***/
	while(1)
	{
		#pragma region Anzeige - aktueller Status
		switch (currentState)
		{
			case States::CLOCK:
			if ((millis() - prevTime) >= 1000)
			{
				prevTime = millis();
				blinkState = !blinkState;

				if (counter < 5)
				{
					autoselectBrightness();
				}
				else if (counter == 5)
				{
					showAlarmLED(AlarmModes::INACTIVE);
					tm.setBrightness(8);
				}
				else if (night && !LED_Power && counter == 15)
				{
					currentState = States::STANDBY;
					tm.setSegments(0, 0, 0, 0);
					break;
				}

				t = rtc.getTime();

				if (!night && t.hour >= 0 && t.hour <= 3) //Nacht-Modus inaktiv & Zeit zum Abschalten?
				{
					night = true;
					saveStatistics();

					if (!LED_Power)
					{
						currentState = States::STANDBY;
						tm.setSegments(0, 0, 0, 0);
						break;
					}
				}

				if (alarmState == AlarmStates::OFF)
				{
					if (counter < 5)
					{
						showAlarmLED(alarmMode);
					}
				}
				else if (blinkState)
				{
					showAlarmLED(alarmMode);
				}
				else
				{
					showAlarmLED(AlarmModes::INACTIVE);
				}

				if (counter <= 15 && alarmState != AlarmStates::ALARM1_VIBR && alarmState != AlarmStates::ALARM2_VIBR)
				{
					counter++;
				}

				clock(t, blinkState);

				//Serial.print("Uhrzeit:");
				//printClock(t);
				//Serial.println();
			}
			break;

			case States::STANDBY:
			if ((millis() - prevTime) >= 30000)
			{
				prevTime = millis();
				t = rtc.getTime();
				if (t.hour >= 8 && t.hour <= 23)
				{
					currentState = States::CLOCK;
					prevTime = 0;
					counter = 0;
					blinkState = false;
					night = false;
				}
			}
			break;

			case States::TEST:
			if ((millis() - prevTime) >= 1000)
			{
				prevTime = millis();
				tm.setBrightness(15);
				tm.setSegments(127, 255, 127, 127);
				showAlarmLED(AlarmModes::ALARM1_ALARM2_ACTIVE);

				switch (counter)
				{
					case 0:
					LED(255, 0, 0);
					vibrate(true);
					break;

					case 1:
					LED(0, 255, 0);
					break;

					case 2:
					LED(0, 0, 255);
					break;

					case 3:
					LED(255, 255, 255);
					vibrate(false);
					break;

					case 4:
					if (LED_Power)
					LED_with_factor(LED_lastColor);
					else
					LED(0, 0, 0);
					currentState = States::CLOCK;
					counter = 0;
					prevTime = 0;
					break;
				}
				if (currentState == States::TEST)
				counter++;
			}
			break;

			case States::TEMP:
			if ((millis() - prevTime) >= 1000)
			{
				prevTime = millis();

				if (counter >= 3)
				{
					counter = 0;
					prevTime = 0;
					currentState = States::CLOCK;
					break;
				}

				float temp = rtc.getTemperature();
				thermometer(trunc(temp));
				counter++;
			}
			break;

			case States::LED_PERCENT:
			if ((millis() - prevTime) >= 1000)
			{
				prevTime = millis();
				display_Percent(LED_percent);
				if (counter >= 2)
				{
					currentState = States::CLOCK;
					counter = 5;
					prevTime = 0;
					break;
				}
				counter++;
			}
			break;

			case States::DISPLAY_ALARM_STATUS:
			if ((millis() - prevTime) >= 1000)
			{
				prevTime = millis();
				showAlarmLED(alarmMode);

				if (counter >= 2)
				{
					counter = 0;
					prevTime = 0;
					currentState = States::CLOCK;
					break;
				}

				autoselectBrightness();

				switch (alarmMode)
				{
					case AlarmModes::ALARM1_ACTIVE:
					tm.setSegments(0, 0, 119, 6);
					break;

					case  AlarmModes::ALARM2_ACTIVE:
					tm.setSegments(0, 0, 119, 91);
					break;

					case  AlarmModes::ALARM1_ALARM2_ACTIVE:
					tm.setSegments(119, 134, 119, 91);
					break;

					case  AlarmModes::INACTIVE:
					tm.setSegments(0, 0, 119, 63);
					break;
				}
				counter++;
			}
			break;

			case States::DISPLAY_ALARM1:
			case States::DISPLAY_ALARM2:
			if ((millis() - prevTime) >= 500)
			{
				prevTime = millis();

				if (blinkState)
				{
					if (currentState == States::DISPLAY_ALARM1)
					showAlarmLED(AlarmModes::ALARM1_ACTIVE);
					else if (currentState == States::DISPLAY_ALARM2)
					showAlarmLED(AlarmModes::ALARM2_ACTIVE);
				}
				else
				showAlarmLED(AlarmModes::INACTIVE);

				blinkState = !blinkState;
				clock(alarmHour, alarmMinute, true);
			}
			break;

			case States::CHANGE_ALARM1_HOUR:
			case States::CHANGE_ALARM2_HOUR:
			if ((millis() - prevTime) >= 500)
			{
				prevTime = millis();

				if (blinkState)
				{
					clock(alarmHour, alarmMinute, true);

					if (currentState == States::CHANGE_ALARM1_HOUR)
					showAlarmLED(AlarmModes::ALARM1_ACTIVE);
					else if (currentState == States::CHANGE_ALARM2_HOUR)
					showAlarmLED(AlarmModes::ALARM2_ACTIVE);
				}
				else
				{
					clock(111, alarmMinute, true);
					showAlarmLED(AlarmModes::INACTIVE);
				}

				blinkState = !blinkState;

			}
			break;

			case States::CHANGE_ALARM1_MINUTE:
			case States::CHANGE_ALARM2_MINUTE:
			if ((millis() - prevTime) >= 500)
			{
				prevTime = millis();

				if (blinkState)
				{
					clock(alarmHour, alarmMinute, true);

					if (currentState == States::CHANGE_ALARM1_MINUTE)
					showAlarmLED(AlarmModes::ALARM1_ACTIVE);
					else if (currentState == States::CHANGE_ALARM2_MINUTE)
					showAlarmLED(AlarmModes::ALARM2_ACTIVE);
				}
				else
				{
					clock(alarmHour, 111, true);
					showAlarmLED(AlarmModes::INACTIVE);
				}

				blinkState = !blinkState;
			}
			break;
		}
		#pragma endregion

		#pragma region Alarmtriggers
		if (check_bit(interruptFlags, 3))
		{
			clear_bit(interruptFlags, 3);

			if (rtc.triggeredAlarm1())
			{
				t = rtc.getTime();
				//Serial.println("A1F");
				if (alarmMode == AlarmModes::ALARM1_ACTIVE || alarmMode == AlarmModes::ALARM1_ALARM2_ACTIVE)
				{
					A1_triggers++;
					//Serial.println("Alarm 1 ausgeloest");

					alarmState = AlarmStates::ALARM1_LED;
					currentState = States::CLOCK;
					LED_State = LED_States::NORMAL;

					night = false;
					counter = 5;
					prevTime3 = 0;
					LED_timer = 0;
					LED_currentColor.setVector(0, 0, 0);
					LED_Power = true;
				}
				rtc.clearAlarm1Flag();
			}

			if (rtc.triggeredAlarm2())
			{
				t = rtc.getTime();
				//Serial.println("A2F");
				if (alarmMode == AlarmModes::ALARM2_ACTIVE || alarmMode == AlarmModes::ALARM1_ALARM2_ACTIVE)
				{
					A2_triggers++;
					//Serial.println("Alarm 2 ausgeloest");

					alarmState = AlarmStates::ALARM2_LED;
					currentState = States::CLOCK;
					LED_State = LED_States::NORMAL;

					night = false;
					counter = 5;
					prevTime3 = 0;
					LED_timer = 0;
					LED_currentColor.setVector(0, 0, 0);
					LED_Power = true;
				}
				rtc.clearAlarm2Flag();
			}
		}

		#pragma endregion

		#pragma region AlarmZustand
		switch (alarmState)
		{
			case AlarmStates::ALARM1_LED:
			case AlarmStates::ALARM2_LED:

			LED_sequence();
			
			break;

			case AlarmStates::ALARM1_VIBR:
			case AlarmStates::ALARM2_VIBR:

			vibration_sequence();

			break;

		}
		#pragma endregion

		LED_effects();

		#pragma region Buttoneingänge überprüfen
		//snooze
		if (snooze.state != 0)
		{
			snooze.tick();
		}

		if (check_bit(interruptFlags, 0))
		{
			clear_bit(interruptFlags, 0);
			snooze.tick();
		}

		//button1
		if (button1.state != 0)
		{
			button1.tick();
		}

		if (check_bit(interruptFlags, 1))
		{
			clear_bit(interruptFlags, 1);
			button1.tick();
		}

		//button2
		if (button2.state != 0)
		{
			button2.tick();
		}

		if (check_bit(interruptFlags, 2))
		{
			clear_bit(interruptFlags, 2);
			button2.tick();
		}
		#pragma endregion

		poll_IR();
	}
} //Loop End

#pragma region Tastenfunktionen
void snooze_click()
{
	snooze_clicks++;
	switch (currentState)
	{
		case States::STANDBY:
		currentState = States::CLOCK;
		blinkState = false;
		counter = 0;
		prevTime = 0;
		break;

		case States::CLOCK:
		counter = 0;
		showAlarmLED(alarmMode);

		autoselectBrightness();

		clock(t, blinkState);
		break;

		default:
		if (currentState != States::CHANGE_ALARM1_MINUTE && currentState != States::CHANGE_ALARM2_MINUTE && currentState != States::CHANGE_ALARM1_HOUR && currentState != States::CHANGE_ALARM2_HOUR && currentState != States::TEST)
		{
			currentState = States::CLOCK;
			counter = 0;
			prevTime = 0;
		}
		break;
	}

	switch (alarmState)
	{
		case AlarmStates::ALARM1_VIBR:
		case AlarmStates::ALARM2_VIBR:
		if (!snoozeState)
		{
			prevTime3 = millis();
			snoozeState = true;
			vibrate(false); //abstellen
			vibrState = true; //für den nächsten Snooze
			LED(LED_FINALCOLOR);
			alarmAttempts = 0;
		}
		break;
	}
}

void snooze_longPressStart()
{
	snooze_longclicks++;
	if (alarmState == AlarmStates::OFF)
	{
		if (LED_Power)
		turnOffLED();

		else
		turnOnLED();
	}
}

void button1_click()
{
	//turnOnLED();
	button1_clicks++;
	prevTime = 0;

	if (alarmState != AlarmStates::ALARM1_VIBR && alarmState != AlarmStates::ALARM2_VIBR)
	{
		switch (currentState)
		{
			case States::CLOCK:
			currentState = States::DISPLAY_ALARM1;

			alarmTime = rtc.getShowAlarm1(LEDSTART);
			alarmHour = alarmTime.hour;
			alarmMinute = alarmTime.min;

			blinkState = true;
			prevTime = 0;
			counter = 0;

			autoselectBrightness();

			break;

			case States::DISPLAY_ALARM1:
			currentState = States::DISPLAY_ALARM2;
			blinkState = true;
			prevTime = 0;

			alarmTime = rtc.getShowAlarm2(LEDSTART);
			alarmHour = alarmTime.hour;
			alarmMinute = alarmTime.min;
			break;

			case States::DISPLAY_ALARM2:
			currentState = States::CLOCK;
			blinkState = false;
			break;

			case States::CHANGE_ALARM1_HOUR:
			rtc.setShowAlarm1(alarmMinute, alarmHour, LEDSTART);
			alarmMode = AlarmModes::ALARM1_ACTIVE;
			currentState = States::CHANGE_ALARM1_MINUTE;
			blinkState = false;
			break;

			case States::CHANGE_ALARM2_HOUR:
			rtc.setShowAlarm2(alarmMinute, alarmHour, LEDSTART);
			alarmMode = AlarmModes::ALARM2_ACTIVE;
			currentState = States::CHANGE_ALARM2_MINUTE;
			blinkState = false;
			break;

			case States::CHANGE_ALARM1_MINUTE:
			rtc.setShowAlarm1(alarmMinute, alarmHour, LEDSTART);
			alarmMode = AlarmModes::ALARM1_ACTIVE;
			currentState = States::DISPLAY_ALARM1;
			break;

			case States::CHANGE_ALARM2_MINUTE:
			rtc.setShowAlarm2(alarmMinute, alarmHour, LEDSTART);
			alarmMode = AlarmModes::ALARM2_ACTIVE;
			currentState = States::DISPLAY_ALARM2;
			break;
		}
	}
	else switch (alarmState)
	{
		case AlarmStates::ALARM1_VIBR:
		case AlarmStates::ALARM2_VIBR:
		alarmState = AlarmStates::OFF; //Alarm abschalten
		alarmAttempts = 0;
		RGB_vector prevColor = LED_lastColor;
		LED_lastColor.setVector(LED_FINALCOLOR);
		LED(LED_FINALCOLOR);
		fadeToColor(prevColor);
		vibrate(false);
		vibrState = true;
		counter = 0;
		break;
	}
}

void button1_longPressStart()
{
	button1_longclicks++;

	if (alarmState == AlarmStates::OFF)
	{
		switch (currentState)
		{
			case States::CLOCK:
			case States::STANDBY:
			currentState = States::TEMP;
			showAlarmLED(AlarmModes::INACTIVE);
			counter = 0;
			prevTime = 0;
			break;

			case States::DISPLAY_ALARM1:
			currentState = States::CHANGE_ALARM1_HOUR;
			blinkState = false;
			prevTime = 0;
			break;

			case States::DISPLAY_ALARM2:
			currentState = States::CHANGE_ALARM2_HOUR;
			blinkState = false;
			prevTime = 0;
			break;
		}
	}
	else
	{
		LED_Power = false;
		LED(0, 0, 0);
		alarmState = AlarmStates::OFF;
		vibrate(false);
		vibrState = true;
	}
}

void button2_click()
{
	button2_clicks++;
	switch (currentState)
	{
		case States::CLOCK:
		case States::DISPLAY_ALARM_STATUS:

		currentState = States::DISPLAY_ALARM_STATUS;

		counter = 0;
		prevTime = 0;
		switch (alarmMode)
		{
			case AlarmModes::INACTIVE:
			alarmMode = AlarmModes::ALARM1_ACTIVE;
			break;

			case AlarmModes::ALARM1_ACTIVE:
			alarmMode = AlarmModes::ALARM2_ACTIVE;
			break;

			case AlarmModes::ALARM2_ACTIVE:
			alarmMode = AlarmModes::ALARM1_ALARM2_ACTIVE;
			break;

			case AlarmModes::ALARM1_ALARM2_ACTIVE:
			alarmMode = AlarmModes::INACTIVE;
			break;
		}

		break;


		case States::CHANGE_ALARM1_HOUR:
		case States::CHANGE_ALARM2_HOUR:
		if ((millis() - prevTime2) >= 200)
		{
			prevTime = 0;
			prevTime2 = millis();
			alarmHour++;
			blinkState = true;
			alarmHour = adjustHour(alarmHour);
		}
		break;

		case States::CHANGE_ALARM1_MINUTE:
		case States::CHANGE_ALARM2_MINUTE:
		if ((millis() - prevTime2) >= 150)
		{
			prevTime = 0;
			prevTime2 = millis();
			alarmMinute += 5;
			blinkState = true;
			alarmMinute = adjustMinute(alarmMinute);
		}
		break;

		default:
		break;

	}
}

void button2_longPressStart()
{
	button2_longclicks++;
	if (currentState == States::CLOCK || currentState == States::STANDBY)
	{
		currentState = States::TEST;
		counter = 0;
		prevTime = 0;
	}
}

void button2_duringLongPress()
{
	if (currentState != States::CLOCK && currentState != States::DISPLAY_ALARM_STATUS)
	{
		if (button2_clicks > 0)
		button2_clicks--;
		button2_click();
	}
}
#pragma endregion


#pragma region Hilfsfunktionen

void vibrate(bool vibr)
{
	if (vibr)
	{
		PORTB |= (1 << VIBR_PIN); //1
	}
	else
	{
		PORTB &= ~(1 << VIBR_PIN); //0
	}
}

void vibration_sequence()
{
	if (((millis() - prevTime2) >= 1000) && !snoozeState)
	{
		seconds++;
		prevTime2 = millis();
	}
	if (snoozeState && ((millis() - snoozeStartTime) >= SNOOZETIME))
	{
		snoozeState = false;
		prevTime3 = 0;
		snoozeStartTime = millis();
		seconds = 0;
	}
	else if (!snoozeState)
	{
		uint8_t intervall;
		if (seconds <= 30)
		intervall = 20; //2 sec
		else if (seconds <= 60)
		intervall = 5; //0.5 sec
		else if (seconds <= 120) //2 min
		intervall = 0;
		else
		{
			alarmAttempts++;
			snoozeState = true;
			LED(LED_FINALCOLOR);
			vibrate(false);
			//Serial.println("Snooze");
			//Serial.println(alarmAttempts);
			if (alarmAttempts > 2)
			{
				alarmState = AlarmStates::OFF;
				LED(0, 0, 0);
				LED_Power = false;
				counter = 0;
			}
		}
		if (!snoozeState)
		{
			if (intervall == 0)
			{
				vibrate(true);
				if ((millis() - prevTime3) >= 250)
				{
					if (vibrState)
					LED(LED_FINALCOLOR);
					else
					LED(0, 0, 0);

					vibrState = !vibrState;
					prevTime3 = millis();
				}
			}
			else if ((millis() - prevTime3) / 100 >= intervall)
			{
				prevTime3 = millis();
				vibrate(vibrState);
				vibrState = !vibrState;
			}
		}
	}
}

uint8_t adjustHour(uint8_t hour)
{
	if (hour >= 24)
	return hour - 24;
	else
	return hour;
}

uint8_t adjustMinute(uint8_t min)
{
	if (min >= 60)
	return min - 60;
	else
	return min;
}

/*
void printClock(Time t)
{
Serial.print(t.hour);
Serial.print(":");
Serial.print(t.min);
Serial.print(":");
Serial.print(t.sec);
}

void printDate(Date t)
{
Serial.print(t.day);
Serial.print(".");
Serial.print(t.month);
Serial.print(".");
Serial.print(t.year);
Serial.print(" - ");
Serial.print(t.dow);
Serial.print(".Tag der Woche");
}
*/

void saveStatistics()
{
	//snooze clicks
	updateEEPROM(0, snooze_clicks);
	snooze_clicks = 0;

	//snooze long clicks
	updateEEPROM(2, snooze_longclicks);
	snooze_longclicks = 0;

	//taste1 clicks
	updateEEPROM(4, button1_clicks);
	button1_clicks = 0;

	//taste1 long clicks
	updateEEPROM(6, button1_longclicks);
	button1_longclicks = 0;

	//taste2 clicks
	updateEEPROM(8, button2_clicks);
	button2_clicks = 0;

	//taste2 long clicks
	updateEEPROM(10, button2_longclicks);
	button2_longclicks = 0;

	//remote received
	updateEEPROM(12, remote_received);
	remote_received = 0;

	//A1 triggers
	updateEEPROM(14, A1_triggers);
	A1_triggers = 0;

	//A2 triggers
	updateEEPROM(16, A2_triggers);
	A2_triggers = 0;
}

void updateEEPROM(uint8_t index, uint8_t value)
{

	uint16_t saved_value = readSavedValue(index);
	saved_value += value;
	eeprom_update_byte((uint8_t*)index, (saved_value >> 8));
	eeprom_update_byte((uint8_t*)(index+1), (saved_value & (0b11111111)));
}

uint16_t readSavedValue(uint8_t index)
{
	return ((eeprom_read_byte((uint8_t*)index) << 8) | eeprom_read_byte((uint8_t*)(index+1)));
}
/*
void showStartMessages()
{
Serial.print("Uhrzeit: ");
printClock(rtc.getTime());
Serial.println();
Serial.print("Datum: ");
printDate(rtc.getDate());
Serial.println();
Serial.print("Alarm1: ");
printClock(rtc.getShowAlarm1(LEDSTART));
Serial.println();
Serial.print("Alarm2: ");
printClock(rtc.getShowAlarm2(LEDSTART));
Serial.println();
Serial.print("Alarmmodus:");
Serial.println(alarmMode);
Serial.println();

Serial.print("Snooze-Klicks: ");
Serial.println(readSavedValue(0), DEC);
Serial.print("Snooze-Longklicks: ");
Serial.println(readSavedValue(2), DEC);
Serial.print("Taste1-Klicks: ");
Serial.println(readSavedValue(4), DEC);
Serial.print("Taste1-Longklicks: ");
Serial.println(readSavedValue(6), DEC);
Serial.print("Taste2-Klicks: ");
Serial.println(readSavedValue(8), DEC);
Serial.print("Taste2-Longklicks: ");
Serial.println(readSavedValue(10), DEC);
Serial.print("Remote-Kommandos: ");
Serial.println(readSavedValue(12), DEC);
Serial.print("Alarm1 ausgeloest: ");
Serial.println(readSavedValue(14), DEC);
Serial.print("Alarm2 ausgeloest: ");
Serial.println(readSavedValue(16), DEC);
}
*/

#pragma endregion

#pragma region Interruptfunktionen
ISR(PCINT1_vect)
{
	if (check_bit(PINC, 0) != check_bit(buttonStates, 0))
	{
		toggle_bit(buttonStates, 0);
		set_bit(interruptFlags, 0);

	}
	else if (check_bit(PINC, 1) != check_bit(buttonStates, 1))
	{
		toggle_bit(buttonStates, 1);
		set_bit(interruptFlags, 1);
	}
	else if (check_bit(PINC, 2) != check_bit(buttonStates, 2))
	{
		toggle_bit(buttonStates, 2);
		set_bit(interruptFlags, 2);
	}
	else if (!check_bit(PINC, 3))
	{
		set_bit(interruptFlags, 3);
	}

}
#pragma endregion



