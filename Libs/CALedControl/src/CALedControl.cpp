#include <CALedControl.h>


CALedControl::CALedControl(uint8_t greenPin, uint8_t redPin) {
    mGreenPin = greenPin;
    mRedPin = redPin;
    mState = ALL_OFF;
    mOnTime = 100;
    mOffTime = 900;
    
    pinMode(mGreenPin, OUTPUT);
    digitalWrite(mGreenPin, LOW);
    pinMode(mRedPin, OUTPUT);
    digitalWrite(mRedPin, LOW);
}

void CALedControl::set(State state) {
    mState = state;
    mNextTime = 0;
    mWriteVal = LOW;
    pollLeds();
}

void CALedControl::pollLeds() {
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
        default:
            green = LOW;
            red = LOW;
            break;
    }

    digitalWrite(mGreenPin, green);
    digitalWrite(mRedPin, red);
}