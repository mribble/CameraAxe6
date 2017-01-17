#ifndef __CAPACKET_BASE_H__
#define __CAPACKET_BASE_H__

#include "CATypes.h"

class CAPacketCamSettingsBase {
public:
    const uint8 SEQ0    = 0x01;
    const uint8 SEQ1    = 0x02;
    const uint8 SEQ2    = 0x04;
    const uint8 SEQ3    = 0x08;
    const uint8 SEQ4    = 0x10;
    const uint8 SEQ5    = 0x20;
    const uint8 SEQ6    = 0x40;
    const uint8 SEQ7    = 0x80;
    CAPacketCamSettingsBase();
    uint8 getCamPortNumber() {return mCamPortNumber;};
    uint8 getMode() {return mMode;};
    uint16 getDelayHours() {return mDelayHours;};
    uint8 getDelayMinutes() {return mDelayMinutes;};
    uint8 getDelaySeconds() {return mDelaySeconds;};
    uint16 getDelayMilliseconds() {return mDelayMilliseconds;};
    uint16 getDelayMicroseconds() {return mDelayMicroseconds;};
    uint16 getDurationHours() {return mDurationHours;};
    uint8 getDurationMinutes() {return mDurationMinutes;};
    uint8 getDurationSeconds() {return mDurationSeconds;};
    uint16 getDurationMilliseconds() {return mDurationMilliseconds;};
    uint16 getDurationMicroseconds() {return mDurationMicroseconds;};
    uint8 getSequencer() {return mSequencer;};
    uint8 getApplyIntervalometer() {return mApplyIntervalometer;};
    uint8 getSmartPreview() {return mSmartPreview;};
    uint8 getMirrorLockupEnable() {return mMirrorLockupEnable;};
    uint8 getMirrorLockupMinutes() {return mMirrorLockupMinutes;};
    uint8 getMirrorLockupSeconds() {return mMirrorLockupSeconds;};
    uint16 getMirrorLockupMilliseconds() {return mMirrorLockupMilliseconds;};
    void set(uint8 camPortNumber, uint8 mode, uint16 delayHours, uint8 delayMinutes,
                uint8 delaySeconds, uint16 delayMilliseconds, uint16 delayMicroseconds,
                uint16 durationHours, uint8 durationMinutes, uint8 durationSeconds, 
                uint16 durationMilliseconds, uint16 durationMicroseconds, uint8 sequencer,
                uint8 applyIntervalometer, uint8 smartPreview, uint8 mirrorLockupEnable, 
                uint8 mirrorLockupMinutes, uint8 mirrorLockupSeconds, uint16 mirrorLockupMilliseconds);
protected:
    uint8 mCamPortNumber;
    uint8 mMode;
    uint16 mDelayHours;
    uint8 mDelayMinutes;
    uint8 mDelaySeconds;
    uint16 mDelayMilliseconds;
    uint16 mDelayMicroseconds;
    uint16 mDurationHours;
    uint8 mDurationMinutes;
    uint8 mDurationSeconds;
    uint16 mDurationMilliseconds;
    uint16 mDurationMicroseconds;
    uint8 mSequencer;
    uint8 mApplyIntervalometer;
    uint8 mSmartPreview;
    uint8 mMirrorLockupEnable;
    uint8 mMirrorLockupMinutes;
    uint8 mMirrorLockupSeconds;
    uint16 mMirrorLockupMilliseconds;
};

class CAPacketInterModuleLogicBase {
public:
    CAPacketInterModuleLogicBase();
    uint8 getLatchEnable() {return mLatchEnable;};
    uint8 getLogic() {return mLogic;};
    void set(uint8 latchEnable, uint8 logic);
protected:
    uint8 mLatchEnable;
    uint8 mLogic;
};

#endif // __CAPACKET_H__
