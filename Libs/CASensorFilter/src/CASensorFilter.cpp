////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CASensorFilter.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This sets up the values
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CASensorFilter::init(hwPortPin pin, FilterMode mode, uint16_t visibleTime) {
    mPin = pin;
    mMode = mode;
    mCurValIndex = 0;
    mThresholdVal = 2048;
    mVisibleTime = visibleTime;
    mNextUpdateTime = millis() + (visibleTime/NUM_SEN_VALS);

    switch (mode) {
        case DIGITAL_MIN:
        case ANALOG_MIN:
            mDefaultVal = 0xffff;
            break;
        case DIGITAL_MAX:
        case ANALOG_MAX:
        case ANALOG_THRESHOLD:
            mDefaultVal = 0;
            break;
        default:
            mDefaultVal = 0;
            CA_ASSERT(0, "Invalid mode");
    }

    for(uint8_t i=0; i<NUM_SEN_VALS; ++i) {
        mVals[i] = mDefaultVal;
    }
}

void CASensorFilter::setThreshold(uint16_t val) {
    mThresholdVal = val;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This should be called in a loop as fast as possible
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16_t CASensorFilter::getSensorData() {
    uint8_t i;
    uint16_t val;
    
    switch (mMode) {
        case DIGITAL_MIN:
            val = CAU::digitalRead(mPin);
            mVals[mCurValIndex] = min(val, mVals[mCurValIndex]);
            for(i=0; i<NUM_SEN_VALS; ++i) {
                val = min(val, mVals[i]);
            }
            break;
        case DIGITAL_MAX:
            val = CAU::digitalRead(mPin);
            mVals[mCurValIndex] = max(val, mVals[mCurValIndex]);
            for(i=0; i<NUM_SEN_VALS; ++i) {
                val = max(val, mVals[i]);
            }
            break;
        case ANALOG_MIN:
            val = CAU::analogRead(mPin);
            mVals[mCurValIndex] = min(val, mVals[mCurValIndex]);
            for(i=0; i<NUM_SEN_VALS; ++i) {
                val = min(val, mVals[i]);
            }
            break;
        case ANALOG_MAX:
            val = CAU::analogRead(mPin);
            mVals[mCurValIndex] = max(val, mVals[mCurValIndex]);
            for(i=0; i<NUM_SEN_VALS; ++i) {
                val = max(val, mVals[i]);
            }
            break;
        case ANALOG_THRESHOLD:
            val = CAU::analogRead(mPin);
            val = (val >= mThresholdVal) ? (val-mThresholdVal) : (mThresholdVal-val);
            mVals[mCurValIndex] = max(val, mVals[mCurValIndex]);
            for(i=0; i<NUM_SEN_VALS; ++i) {
                val = max(val, mVals[i]);
            }
            break;
    }
    
    if (mNextUpdateTime <= millis()) {
        mCurValIndex = (mCurValIndex+1)%NUM_SEN_VALS;
        mVals[mCurValIndex] = mDefaultVal;
        mNextUpdateTime = millis() + (mVisibleTime/NUM_SEN_VALS);
    }
    return val;
}