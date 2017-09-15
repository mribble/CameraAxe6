#include <CATickTimer.h>

//Static members of the class
Tc* CATickTimer::m_tc[NUM_TIMERS] = {0};
uint8_t CATickTimer::m_tcChannel[NUM_TIMERS] = {0};
IRQn_Type CATickTimer::m_irq[NUM_TIMERS];
callbackFunc CATickTimer::m_isr[NUM_TIMERS] = {NULL};
uint64_t CATickTimer::m_tcTicks[NUM_TIMERS] = {0};
uint64_t CATickTimer::m_tcTicksCur[NUM_TIMERS] = {0};
uint8_t CATickTimer::m_repeat[NUM_TIMERS] = {0};

// Implement the timer callback
void TC0_Handler()
{
    TC_GetStatus(TC0, 0);  // Clears interrupt flag
    CATickTimer::isrHandler(0);
}
void TC1_Handler()
{
    TC_GetStatus(TC0, 1);  // Clears interrupt flag
    CATickTimer::isrHandler(1);
}
void TC2_Handler()
{
    TC_GetStatus(TC0, 2);  // Clears interrupt flag
    CATickTimer::isrHandler(2);
}
void TC3_Handler()
{
    TC_GetStatus(TC1, 0);  // Clears interrupt flag
    CATickTimer::isrHandler(3);
}
void TC4_Handler()
{
    TC_GetStatus(TC1, 1);  // Clears interrupt flag
    CATickTimer::isrHandler(4);
}
void TC5_Handler()
{
    TC_GetStatus(TC1, 2);  // Clears interrupt flag
    CATickTimer::isrHandler(5);
}
void TC6_Handler()
{
    TC_GetStatus(TC2, 0);  // Clears interrupt flag
    CATickTimer::isrHandler(6);
}
void TC7_Handler()
{
    TC_GetStatus(TC2, 1);  // Clears interrupt flag
    CATickTimer::isrHandler(7);
}
void TC8_Handler()
{
    TC_GetStatus(TC2, 2);  // Clears interrupt flag
    CATickTimer::isrHandler(8);
}

CATickTimer::CATickTimer(uint8_t timerNum)
{
    m_num = timerNum;
    m_tcChannel[m_num] = timerNum % 3;
    switch (timerNum)
    {
        case 0:
            m_irq[m_num] = TC0_IRQn;
            m_tc[m_num] = TC0;
            break;
        case 1:
            m_irq[m_num] = TC1_IRQn;
            m_tc[m_num] = TC0;
            break;
        case 2:
            m_irq[m_num] = TC2_IRQn;
            m_tc[m_num] = TC0;
            break;
        case 3:
            m_irq[m_num] = TC3_IRQn;
            m_tc[m_num] = TC1;
            break;
        case 4:
            m_irq[m_num] = TC4_IRQn;
            m_tc[m_num] = TC1;
            break;
        case 5:
            m_irq[m_num] = TC5_IRQn;
            m_tc[m_num] = TC1;
            break;
        case 6:
            m_irq[m_num] = TC6_IRQn;
            m_tc[m_num] = TC2;
            break;
        case 7:
            m_irq[m_num] = TC7_IRQn;
            m_tc[m_num] = TC2;
            break;
        case 8:
            m_irq[m_num] = TC8_IRQn;
            m_tc[m_num] = TC2;
            break;
        default:
            m_irq[m_num] = TC0_IRQn;
            m_tc[m_num] = TC0;
            CA_ASSERT(0, "CATickTimers timerNum out of range");
            break;
    }
}

void CATickTimer::start(callbackFunc isr, uint64_t ticks, uint8_t repeat)
{
    uint8_t clockFlag;
    uint8_t clockDivisor;
    
    CA_ASSERT(m_irq[m_num], "Callback registration conflict");
    m_isr[m_num] = isr;
    m_repeat[m_num] = repeat;
    pmc_set_writeprotect(false); // Disable write protection of the timer/counter registers
    pmc_enable_periph_clk(m_irq[m_num]);
    
    if (ticks < (0x1ffffffff-1))        // TIMER_CLOCK1 is MCK/2
    {
        clockFlag = TC_CMR_TCCLKS_TIMER_CLOCK1;
        clockDivisor = 2;
        
    }
    else if (ticks < (0x7ffffffff-8))   // TIMER_CLOCK2 is MCK/8
    {
        clockFlag = TC_CMR_TCCLKS_TIMER_CLOCK2;
        clockDivisor = 8;
    }
    else if (ticks < (0x31ffffffff-16)) // TIMER_CLOCK3 is MCK/32
    {
        clockFlag = TC_CMR_TCCLKS_TIMER_CLOCK3;
        clockDivisor = 32;
    }
    else                                // TIMER_CLOCK4 is MCK/128
    {
        clockFlag = TC_CMR_TCCLKS_TIMER_CLOCK4;
        clockDivisor = 128;
    }
    
    m_tcTicks[m_num] = ticks/clockDivisor;
    m_tcTicksCur[m_num] = m_tcTicks[m_num];

    
    // Configure timer to count up and automatic interrupt when desired value is reached
    TC_Configure(m_tc[m_num], m_tcChannel[m_num], TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | clockFlag);
    
    if (m_tcTicks[m_num] > 0xffffffff)
    {
        TC_SetRC(m_tc[m_num], m_tcChannel[m_num], 0xffffffff);
    }
    else
    {
        TC_SetRC(m_tc[m_num], m_tcChannel[m_num], m_tcTicks[m_num]);
    }
    
    (m_tc[m_num])->TC_CHANNEL[m_tcChannel[m_num]].TC_IER = TC_IER_CPCS;   // Enable the rc compare interrupt
    (m_tc[m_num])->TC_CHANNEL[m_tcChannel[m_num]].TC_IDR = ~TC_IER_CPCS;  // Disable all the other interrupts
    
    NVIC_ClearPendingIRQ(m_irq[m_num]);
    NVIC_EnableIRQ(m_irq[m_num]);
    TC_Start(m_tc[m_num], m_tcChannel[m_num]);
}

void CATickTimer::stop(uint8_t timerNum)
{
    NVIC_DisableIRQ(m_irq[timerNum]);
    TC_Stop(m_tc[timerNum], m_tcChannel[timerNum]);
    CA_ASSERT(m_isr[timerNum], "Don't unregister a null callback");
    m_isr[timerNum] = NULL;
}

void CATickTimer::stop()
{
    stop(m_num);
}

uint64_t CATickTimer::convertTimeToTicks(uint32_t seconds, uint32_t nanoseconds)
{
    uint64_t ret;
    uint64_t ticksPerMicroSec = 84; //Sam8 running at 84 mhz

    ret = ((uint64_t)seconds*1000*1000*1000 + nanoseconds)* ticksPerMicroSec / 1000;

    return ret;
}

void CATickTimer::isrHandler(uint8_t timerNum)
{
    uint8_t num = timerNum;
    if (m_tcTicks[num] <= 0xffffffff)
    {
        (*m_isr[num])();
    }
    else
    {
        // Handle multiple interrupts to decrement counter since ticks were too large to handle
        // with a single pass of the clock timer
        uint32_t ticks;
        if (m_tcTicksCur[num] > 0xffffffff)
        {
            // Need to wait more
            m_tcTicksCur[num] -= 0xffffffff;
            TC_SetRC(m_tc[num], m_tcChannel[num], (uint32_t)min(0xffffffff, m_tcTicksCur[num]));
        }
        else if (m_tcTicksCur[num] > 0)
        {
            // Last iteration
            TC_SetRC(m_tc[num], m_tcChannel[num], (uint32_t)m_tcTicksCur[num]);
            m_tcTicksCur[num] = 0;
        }
        else
        {
            // Done
            TC_SetRC(m_tc[num], m_tcChannel[num], 0xffffffff);
            m_tcTicksCur[num] = m_tcTicks[num];
            (*m_isr[num])();
        }
    }

    if (m_repeat[num] == false)
    {
        stop(num);
    }
}
