#ifndef __CALEDCONTROL_H__
#define __CALEDCONTROL_H__

#include <CAStd.h>

#define NO_PIN 0xFF

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CALedControl - This blinks some LEDs.  The way it works is if you set a maxBlinks of 1 then the LEDs are on
//                  for the onTime in ms and off for the offTime in ms.  If maxBlinks is greater than 1 then 
//                  the LEDs flash on/off/on... for the onTime and then stay off after the blinking for the offTime.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CALedControl
{
public:
    enum State { ALL_OFF, GREEN_ON, GREEN_BLINK, RED_ON, RED_BLINK, ORANGE_ON, ORANGE_BLINK};
    
    CALedControl(uint8_t greenPin, uint8_t redPin);
    void set(State state);
    void pollLeds();

private:    
    uint8_t mGreenPin;
    uint8_t mRedPin;
    uint8_t mState;
    uint16_t mOnTime;
    uint16_t mOffTime;
    uint8_t mWriteVal;
    uint32_t mNextTime;
};


#endif // __CALEDCONTROL_H__
