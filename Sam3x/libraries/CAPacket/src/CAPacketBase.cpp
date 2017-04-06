#include <CAPacket.h>

///////////////////////////////////////////////////////////////////////////////
// CamSettings Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketCamSettingsBase::CAPacketCamSettingsBase() {
    mCamPortNumber = 0;
    mMode = 0;
    mDelayHours = 0;
    mDelayMinutes = 0;
    mDelaySeconds = 0;
    mDelayMilliseconds = 0;
    mDelayMicroseconds = 0;
    mDurationHours = 0;
    mDurationMinutes = 0;
    mDurationSeconds = 0;
    mDurationMilliseconds = 0;
    mDurationMicroseconds = 0;
    mSequencer = 0;
    mApplyIntervalometer = 0;
    mSmartPreview = 0;
    mMirrorLockupEnable = 0;
    mMirrorLockupMinutes = 0;
    mMirrorLockupSeconds = 0;
    mMirrorLockupMilliseconds = 0;
}

void CAPacketCamSettingsBase::set(uint8_t camPortNumber, uint8_t mode, uint16_t delayHours, uint8_t delayMinutes,
                uint8_t delaySeconds, uint16_t delayMilliseconds, uint16_t delayMicroseconds,
                uint16_t durationHours, uint8_t durationMinutes, uint8_t durationSeconds, 
                uint16_t durationMilliseconds, uint16_t durationMicroseconds, uint8_t sequencer,
                uint8_t applyIntervalometer, uint8_t smartPreview, uint8_t mirrorLockupEnable, 
                uint8_t mirrorLockupMinutes, uint8_t mirrorLockupSeconds, uint16_t mirrorLockupMilliseconds) {
    mCamPortNumber = camPortNumber;
    mMode = mode;
    mDelayHours = delayHours;
    mDelayMinutes = delayMinutes;
    mDelaySeconds = delaySeconds;
    mDelayMilliseconds = delayMilliseconds;
    mDelayMicroseconds = delayMicroseconds;
    mDurationHours = durationHours;
    mDurationMinutes = durationMinutes;
    mDurationSeconds = durationSeconds;
    mDurationMilliseconds = durationMilliseconds;
    mDurationMicroseconds = durationMicroseconds;
    mSequencer = sequencer;
    mApplyIntervalometer = applyIntervalometer;
    mSmartPreview = smartPreview;
    mMirrorLockupEnable = mirrorLockupEnable;
    mMirrorLockupMinutes = mirrorLockupMinutes;
    mMirrorLockupSeconds = mirrorLockupSeconds;
    mMirrorLockupMilliseconds = mirrorLockupMilliseconds;
    CA_ASSERT((mMode <= 2) && (mDelayHours <= 999) && (mDelayMinutes <=59) &&
                (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) && (mDelayMicroseconds <= 999) &&
                (mDurationHours <= 999) && (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) && (mApplyIntervalometer <= 1) &&
                (mSmartPreview <= 59) && (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
                (mMirrorLockupSeconds <= 59) && (mMirrorLockupMilliseconds <= 999),
                "Error in CAPacketCamSettingsBase::set()");
}


