////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CALed - This blinks some LEDs.  The way it works is if you set a maxBlinks of 1 then the LEDs are on
//          for the onTime in ms and off for the offTime in ms.  If maxBlinks is greater than 1 then 
//          the LEDs flash on/off/on... for the onTime and then stay off after the blinking for the offTime.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CALED_H__
#define __CALED_H__

#include <CAStd.h>

#define NO_PIN 0xFF

class CALed
{
public:
    enum State { ALL_OFF, GREEN_ON, GREEN_BLINK, RED_ON, RED_BLINK, ORANGE_ON, ORANGE_BLINK, GREEN_RED_BLINK};
    
    CALed(uint8_t greenPin, uint8_t redPin);
    void set(State state);
    void pollLeds();

private:    
    uint8_t mGreenPin;      // IO pin for green led
    uint8_t mRedPin;        // IO pin for red led
    uint8_t mState;         // The state of the led mode (see enum State)
    uint16_t mOnTime;       // Time in ms that the LED is on when blinking
    uint16_t mOffTime;      // Time in ms that the LED is off when blinking
    uint8_t mWriteVal;      // Holds the current write value (LOW or HIGH) when blinking
    uint32_t mNextTime;     // Holds the next time when LED toggles when blinking
};

#endif // __CALED_H__
