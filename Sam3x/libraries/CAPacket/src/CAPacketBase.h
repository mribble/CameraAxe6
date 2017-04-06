#ifndef __CAPACKET_BASE_H__
#define __CAPACKET_BASE_H__

#include "CAUtility.h"

// These base classes exist because sometimes we need to store this data and we don't want that to be tied to
//  packet processing so we created base classes without any of the packet processing included

class CAPacketCamSettingsBase {
public:
    const uint8_t SEQ0    = 0x01;
    const uint8_t SEQ1    = 0x02;
    const uint8_t SEQ2    = 0x04;
    const uint8_t SEQ3    = 0x08;
    const uint8_t SEQ4    = 0x10;
    const uint8_t SEQ5    = 0x20;
    const uint8_t SEQ6    = 0x40;
    const uint8_t SEQ7    = 0x80;
    CAPacketCamSettingsBase();
    uint8_t getCamPortNumber() {return mCamPortNumber;};
    uint8_t getMode() {return mMode;};
    uint16_t getDelayHours() {return mDelayHours;};
    uint8_t getDelayMinutes() {return mDelayMinutes;};
    uint8_t getDelaySeconds() {return mDelaySeconds;};
    uint16_t getDelayMilliseconds() {return mDelayMilliseconds;};
    uint16_t getDelayMicroseconds() {return mDelayMicroseconds;};
    uint16_t getDurationHours() {return mDurationHours;};
    uint8_t getDurationMinutes() {return mDurationMinutes;};
    uint8_t getDurationSeconds() {return mDurationSeconds;};
    uint16_t getDurationMilliseconds() {return mDurationMilliseconds;};
    uint16_t getDurationMicroseconds() {return mDurationMicroseconds;};
    uint8_t getSequencer() {return mSequencer;};
    uint8_t getApplyIntervalometer() {return mApplyIntervalometer;};
    uint8_t getSmartPreview() {return mSmartPreview;};
    uint8_t getMirrorLockupEnable() {return mMirrorLockupEnable;};
    uint8_t getMirrorLockupMinutes() {return mMirrorLockupMinutes;};
    uint8_t getMirrorLockupSeconds() {return mMirrorLockupSeconds;};
    uint16_t getMirrorLockupMilliseconds() {return mMirrorLockupMilliseconds;};
    void set(uint8_t camPortNumber, uint8_t mode, uint16_t delayHours, uint8_t delayMinutes,
                uint8_t delaySeconds, uint16_t delayMilliseconds, uint16_t delayMicroseconds,
                uint16_t durationHours, uint8_t durationMinutes, uint8_t durationSeconds, 
                uint16_t durationMilliseconds, uint16_t durationMicroseconds, uint8_t sequencer,
                uint8_t applyIntervalometer, uint8_t smartPreview, uint8_t mirrorLockupEnable, 
                uint8_t mirrorLockupMinutes, uint8_t mirrorLockupSeconds, uint16_t mirrorLockupMilliseconds);
protected:
    uint8_t mCamPortNumber;
    uint8_t mMode;
    uint16_t mDelayHours;
    uint8_t mDelayMinutes;
    uint8_t mDelaySeconds;
    uint16_t mDelayMilliseconds;
    uint16_t mDelayMicroseconds;
    uint16_t mDurationHours;
    uint8_t mDurationMinutes;
    uint8_t mDurationSeconds;
    uint16_t mDurationMilliseconds;
    uint16_t mDurationMicroseconds;
    uint8_t mSequencer;
    uint8_t mApplyIntervalometer;
    uint8_t mSmartPreview;
    uint8_t mMirrorLockupEnable;
    uint8_t mMirrorLockupMinutes;
    uint8_t mMirrorLockupSeconds;
    uint16_t mMirrorLockupMilliseconds;
};

#endif // __CAPACKET_H__
