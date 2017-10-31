// -----
// OneButton.cpp - Library for detecting button clicks, doubleclicks and long press pattern on a
// single button. This class is implemented for use with the Arduino environment. Copyright (c) by
// Matthias Hertel, http://www.mathertel.de This work is licensed under a BSD style license. See
// http://www.mathertel.de/License.aspx More information on: http://www.mathertel.de/Arduino
// -----
// Changelog: see OneButton.h
// -----

#include "OneButton2.h"
#include "timer0.h"
#include <stdint.h>

// ----- Initialization and Default Values -----

OneButton::OneButton(volatile uint8_t *pinAddr, uint8_t pin)
{
  _pinAddr = pinAddr;
  _pin = pin;

  _pressTicks =
      750; // number of millisec that have to pass by before a long button press is detected.

  state = 0;              // starting with state 0: waiting for button to be pressed
  _isLongPressed = false; // Keep track of long press state

  _buttonReleased = true; // notPressed
  _buttonPressed = false;

  _longPressStartFunc = 0;
  _longPressStopFunc = 0;
  _duringLongPressFunc = 0;
} // OneButton

// explicitly set the number of millisec that have to pass by before a long button press is
// detected.
void OneButton::setPressTicks(int ticks) { _pressTicks = ticks; }

// save function for click event
void OneButton::attachClick(callbackFunction newFunction) { _clickFunc = newFunction; }

// save function for longPressStart event
void OneButton::attachLongPressStart(callbackFunction newFunction)
{
  _longPressStartFunc = newFunction;
}

// save function for longPressStop event
void OneButton::attachLongPressStop(callbackFunction newFunction)
{
  _longPressStopFunc = newFunction;
}

// save function for during longPress event
void OneButton::attachDuringLongPress(callbackFunction newFunction)
{
  _duringLongPressFunc = newFunction;
}

// function to get the current long pressed state
bool OneButton::isLongPressed() { return _isLongPressed; }

void OneButton::tick()
{

  bool buttonLevel = (*_pinAddr >> (_pin)) & 1; // digitalRead(_pin); // current button signal.

  // Implementation of the state machine
  if (state == 0)
  {
    if (buttonLevel == _buttonPressed)
    {
      state = 1;             // step to state 1
      _startTime = millis(); // remember starting time
    }
  }
  else if (state == 1)
  {
    if ((buttonLevel == _buttonReleased) && ((millis() - _startTime) < _debounceTicks))
    {
      // button was released to quickly so I assume some debouncing.
      // go back to state 0 without calling a function.
      state = 0;
    }
    else if (buttonLevel == _buttonReleased)
    {
      if (_clickFunc)
        _clickFunc();
      state = 0; // restart.
    }
    else if ((buttonLevel == _buttonPressed) && ((millis() - _startTime) > _pressTicks))
    {
      _isLongPressed = true; // Keep track of long press state
      if (_longPressStartFunc)
        _longPressStartFunc();
      if (_duringLongPressFunc)
        _duringLongPressFunc();
      state = 6;
    }
    else
    {
      // wait. Stay in this state.
    }
  }
  else if (state == 6)
  {
    if (buttonLevel == _buttonReleased)
    {
      _isLongPressed = false; // Keep track of long press state
      if (_longPressStopFunc)
        _longPressStopFunc();
      state = 0;
    }
    else
    {
      // button is being long pressed
      if (_duringLongPressFunc)
        _duringLongPressFunc();
    }
  }
}
