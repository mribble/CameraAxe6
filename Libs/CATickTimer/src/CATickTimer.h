#ifndef __CATICKTIMER_H__
#define __CATICKTIMER_H__

#include "CAUtility.h"

// There are 9 timers on the SAM3X8E.  They are group in 3 Timer Counters (TC)
// modules and each module has 3 channels.

// Servo Library uses TC0-0, TC0-2, TC1-0, TC1-1, TC1-2 so you can't use those
// in this library if the servo library is also being used.

#define NUM_TIMERS 9

typedef void (*callbackFunc)(void);

class CATickTimer
{
public:
    CATickTimer(uint8_t timerNum);
    void start(callbackFunc isr, uint64_t ticks, uint8_t repeat);
    static void stop(uint8_t timerNum);
    void stop();
    uint64_t convertTimeToTicks(uint16_t hours, uint8_t minutes, uint8_t seconds, 
                         uint16_t milliseconds, uint16_t microseconds, uint16_t nanoseconds);
    static void isrHandler(uint8_t timerNum);

private:
    uint8_t m_num; //timerNum
    
    static Tc *m_tc[NUM_TIMERS];
    static uint8_t m_tcChannel[NUM_TIMERS];
    static IRQn_Type m_irq[NUM_TIMERS];
    static callbackFunc m_isr[NUM_TIMERS];
    static uint64_t m_tcTicks[NUM_TIMERS];
    static uint64_t m_tcTicksCur[NUM_TIMERS];
    static uint8_t m_repeat[NUM_TIMERS];
};


#endif // __CATICKTIMER_H__