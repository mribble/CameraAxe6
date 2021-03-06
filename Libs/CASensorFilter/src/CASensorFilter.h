////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
// 
// Does time based filtering on data to make in friendly for flicker free display
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CASENSORFILTER_H__
#define __CASENSORFILTER_H__

#include <CAStd.h>
#include <CAUtility.h>
#define NUM_SEN_VALS 4


class CASensorFilter
{
public:
    enum FilterMode {DIGITAL_MIN, DIGITAL_MAX, ANALOG_MIN, ANALOG_MAX, ANALOG_THRESHOLD};
    
    CASensorFilter() {};
    
    void init(hwPortPin pin, FilterMode mode, uint16_t visibleTime);
    void setThreshold(uint16_t val);
    uint16_t getSensorData();


private:    
    hwPortPin mPin;                 // Pin that is being filtered
    FilterMode mMode;               // Know if the reading of the pin should be analog or digital
    uint8_t mCurValIndex;           // Current index for the rolling window
    uint16_t mVals[NUM_SEN_VALS];   // Rolling window of values
    uint16_t mDefaultVal;           // The default value for the current mode
    uint16_t mThresholdVal;         // The threshold value to compare against
    uint16_t mVisibleTime;          // Min time in milliseconds the value will be visible
    uint32_t mNextUpdateTime;       // The time in milliseconds when we update the curValIndex
};

#endif // __CALED_H__
