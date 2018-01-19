////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
//  CALed - This blinks some LEDs.  The way it works is if you set a maxBlinks of 1 then the LEDs are on
//          for the onTime in ms and off for the offTime in ms.  If maxBlinks is greater than 1 then 
//          the LEDs flash on/off/on... for the onTime and then stay off after the blinking for the offTime.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CALed.h>

CALed::CALed(uint8_t greenPin, uint8_t redPin) {
    mGreenPin = greenPin;
    mRedPin = redPin;
    mState = ALL_OFF;
    mOnTime = 250;
    mOffTime = 250;
    mNextTime = 0;
    mWriteVal = LOW;
    
    pinMode(mGreenPin, OUTPUT);
    digitalWrite(mGreenPin, LOW);
    pinMode(mRedPin, OUTPUT);
    digitalWrite(mRedPin, LOW);
}

void CALed::set(State state) {
    mState = state;
    pollLeds();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This should be called in a loop around every 50 ms or faster (slower mostly just affects blink times)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CALed::pollLeds() {
    uint8_t red;
    uint8_t green;
    
    if (millis() < mNextTime) {
        return;
    }
    
    mWriteVal = (mWriteVal == LOW) ? HIGH : LOW;    // Toggle value

    if (mWriteVal == HIGH) {
        mNextTime = millis() + mOnTime;
    } else {
        mNextTime = millis() + mOffTime;
    }

    switch (mState) {
        case ALL_OFF:
            green = LOW;
            red = LOW;
            break;
        case GREEN_ON:
            green = HIGH;
            red = LOW;
            break;
        case GREEN_BLINK:
            green = mWriteVal;
            red = LOW;
            break;
        case RED_ON:
            green = LOW;
            red = HIGH;
            break;
        case RED_BLINK:
            green = LOW;
            red = mWriteVal;
            break;
        case ORANGE_ON:
            green = HIGH;
            red = HIGH;
            break;
        case ORANGE_BLINK:
            green = mWriteVal;
            red = mWriteVal;
            break;
        case GREEN_RED_BLINK:
            green = mWriteVal;
            red = (mWriteVal == LOW) ? HIGH : LOW;
            break;
        default:
            green = LOW;
            red = LOW;
            break;
    }

    digitalWrite(mGreenPin, green);
    digitalWrite(mRedPin, red);
}