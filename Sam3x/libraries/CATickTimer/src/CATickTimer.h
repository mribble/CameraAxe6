#ifndef __CATICKTIMER_H__
#define __CATICKTIMER_H__

#include "CATypes.h"
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
    CATickTimer(uint8 timerNum);
    void start(callbackFunc isr, uint64 ticks, uint8 repeat);
    static void stop(uint8 timerNum);
    void stop();
    uint64 convertTimeToTicks(uint16 hours, uint8 minutes, uint8 seconds, 
                         uint16 milliseconds, uint16 microseconds, uint16 nanoseconds);
    static void isrHandler(uint8 timerNum);

private:
    uint8 m_num; //timerNum
    
    static Tc *m_tc[NUM_TIMERS];
    static uint8 m_tcChannel[NUM_TIMERS];
    static IRQn_Type m_irq[NUM_TIMERS];
    static callbackFunc m_isr[NUM_TIMERS];
    static uint64 m_tcTicks[NUM_TIMERS];
    static uint64 m_tcTicksCur[NUM_TIMERS];
    static uint8 m_repeat[NUM_TIMERS];
};


#endif // __CATICKTIMER_H__