#include <CAPacket.h>
#include <CAUtility.h>

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

void CAPacketCamSettingsBase::set(uint8 camPortNumber, uint8 mode, uint16 delayHours, uint8 delayMinutes,
                uint8 delaySeconds, uint16 delayMilliseconds, uint16 delayMicroseconds,
                uint16 durationHours, uint8 durationMinutes, uint8 durationSeconds, 
                uint16 durationMilliseconds, uint16 durationMicroseconds, uint8 sequencer,
                uint8 applyIntervalometer, uint8 smartPreview, uint8 mirrorLockupEnable, 
                uint8 mirrorLockupMinutes, uint8 mirrorLockupSeconds, uint16 mirrorLockupMilliseconds) {
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


