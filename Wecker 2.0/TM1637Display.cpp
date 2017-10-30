
//  Author: avishorp@gmail.com
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/*
extern "C" {
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
}*/
//#include <Arduino.h>

#include "TM1637Display.h"
#include "avr/io.h"
#include "bit_manipulation.h"
#include <util/delay.h>

#define TM1637_I2C_COMM1 0x40
#define TM1637_I2C_COMM2 0xC0
#define TM1637_I2C_COMM3 0x80

// DIO Pin7 - D7
#define DIO_DDR DDRD
#define DIO_PORT PORTD
#define DIO_PIN PIND7

// CLK Pin8 - B0
#define CLK_DDR DDRB
#define CLK_PORT PORTB
#define CLK_PIN PINB0

//
//      A
//     ---
//  F |   | B    P - Doppelpunkt
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
    // xxPGFEDCBA
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001  // F
};

TM1637Display::TM1637Display()
{
  // Set the pin direction and default value.
  // Both pins are set as inputs, allowing the pull-up resistors to pull them up
  clear_bit(CLK_DDR, CLK_PIN); // input
  clear_bit(DIO_DDR, DIO_PIN); // input

  clear_bit(CLK_PORT, CLK_PIN); // low
  clear_bit(DIO_PORT, DIO_PIN); // low
}

void TM1637Display::setBrightness(uint8_t brightness) { m_brightness = brightness; }

void TM1637Display::setSegments(const uint8_t digit0, const uint8_t digit1, const uint8_t digit2,
                                const uint8_t digit3)
{
  // Write COMM1
  start();
  writeByte(TM1637_I2C_COMM1);
  stop();

  // Write COMM2 + first digit address
  start();
  writeByte(TM1637_I2C_COMM2);

  // Write the data bytes
  writeByte(digit0);
  writeByte(digit1);
  writeByte(digit2);
  writeByte(digit3);

  stop();

  // Write COMM3 + brightness
  start();
  writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
  stop();
}

void TM1637Display::bitDelay() { _delay_us(50); }

void TM1637Display::start()
{
  set_bit(DIO_DDR, DIO_PIN); // output
  bitDelay();
}

void TM1637Display::stop()
{
  set_bit(DIO_DDR, DIO_PIN); // output
  bitDelay();
  clear_bit(CLK_DDR, CLK_PIN); // input
  bitDelay();
  clear_bit(DIO_DDR, DIO_PIN); // input
  bitDelay();
}

bool TM1637Display::writeByte(uint8_t b)
{
  uint8_t data = b;

  // 8 Data Bits
  for (uint8_t i = 0; i < 8; i++)
  {
    // CLK low
    set_bit(CLK_DDR, CLK_PIN); // output
    bitDelay();

    // Set data bit
    if (data & 0x01)
    {
      clear_bit(DIO_DDR, DIO_PIN); // input
    }

    else
    {
      set_bit(DIO_DDR, DIO_PIN); // output
    }

    bitDelay();

    // CLK high
    clear_bit(CLK_DDR, CLK_PIN); // input
    bitDelay();
    data = data >> 1;
  }

  // Wait for acknowledge
  // CLK to zero
  set_bit(CLK_DDR, CLK_PIN);   // output
  clear_bit(DIO_DDR, DIO_PIN); // input
  bitDelay();

  // CLK to high
  clear_bit(CLK_DDR, CLK_PIN); // input
  bitDelay();
  uint8_t ack = check_bit(PIND, DIO_PIN); // digitalRead(m_pinDIO);
  if (ack == 0)
  {
    set_bit(DIO_DDR, DIO_PIN); // output
  }

  bitDelay();
  set_bit(CLK_DDR, CLK_PIN); // output
  bitDelay();

  return ack;
}

uint8_t TM1637Display::encodeDigit(uint8_t digit) { return digitToSegment[digit & 0x0f]; }
