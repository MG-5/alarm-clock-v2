// -----
// OneButton.h - Library for detecting button clicks, doubleclicks and long press pattern on a
// single button. This class is implemented for use with the Arduino environment. Copyright (c) by
// Matthias Hertel, http://www.mathertel.de This work is licensed under a BSD style license. See
// http://www.mathertel.de/License.aspx More information on: http://www.mathertel.de/Arduino
// -----
// 02.10.2010 created by Matthias Hertel
// 21.04.2011 transformed into a library
// 01.12.2011 include file changed to work with the Arduino 1.0 environment
// 23.03.2014 Enhanced long press functionalities by adding longPressStart and longPressStop
// callbacks 21.09.2015 A simple way for debounce detection added.
// -----

#ifndef OneButton2_h
#define OneButton2_h

// ----- Callback function types -----

extern "C"
{
    typedef void (*callbackFunction)(void);
}

#include <stdint.h>

class OneButton
{
public:
    // ----- Constructor -----
    OneButton(volatile uint8_t *pinAddr, uint8_t pin);

    // ----- Set runtime parameters -----

    // set # millisec after press is assumed.
    void setPressTicks(int ticks);

    // attach functions that will be called when button was pressed in the specified way.
    void attachClick(callbackFunction newFunction);
    void attachLongPressStart(callbackFunction newFunction);
    void attachLongPressStop(callbackFunction newFunction);
    void attachDuringLongPress(callbackFunction newFunction);

    // ----- State machine functions -----

    // call this function every some milliseconds for handling button events.
    void tick();
    bool isLongPressed();
    uint8_t state;

private:
    volatile uint8_t *_pinAddr; // hardware pin address
    uint8_t _pin;               // hardware pin number

    uint16_t
        _pressTicks; // number of ticks that have to pass by before a long button press is detected
    const uint8_t _debounceTicks = 50; // number of ticks for debounce times.

    bool _buttonReleased;
    bool _buttonPressed;

    bool _isLongPressed;

    // These variables will hold functions acting as event source.
    callbackFunction _clickFunc;
    callbackFunction _longPressStartFunc;
    callbackFunction _longPressStopFunc;
    callbackFunction _duringLongPressFunc;

    // These variables that hold information across the upcoming tick calls.
    // They are initialized once on program start and are updated every time the tick function is
    // called.

    unsigned long _startTime; // will be set in state 1
};

#endif
