////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
// 
// This class implements a binary protocol for sending the type of data data between sam3x and esp8266 via serial
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CAPacket.h>

CAPacket::CAPacket(uint8_t state, uint8_t *buf, uint16_t bufSize) {
    mBitsUsed = 0;
    mBitsVal = 0;
    mBytesUsed = 0;
    mState = state;
    mBuf = buf;
    mBufSize = bufSize;
}

void CAPacket::resetBuffer() {
    flushPacket();  // Check for errors
    mBitsUsed = 0;
    mBitsVal = 0;
    mBytesUsed = 0;
}

boolean CAPacket::unpackGuard() {
    boolean ret;
    if (unpacker(8) == GUARD_PACKET) {
        ret = true;
    } else {
        ret = false;
    }
    return ret;
}

uint16_t CAPacket::unpackSize() {
    return (uint16_t)unpacker(16);
}

uint8_t CAPacket::unpackType() {
    uint8_t val = (uint8_t)unpacker(8);
    CA_ASSERT( val > PID_START_SENTINEL  &&
               val < PID_END_SENTINEL,   "Invalid packet type");
    return val;
}
    
uint32_t CAPacket::unpacker(uint8_t unpackBits)
{
    CA_ASSERT(mState == STATE_UNPACKER, "Error in unpacker");
    uint8_t unpackBitsLeft = unpackBits;
    uint32_t ret = 0;
    uint8_t valShift = 0;

    // This loop shifts through the number of bytes you want to unpack in the src buffer
    // and puts them into an uint32
    do
    {
        uint8_t bitsInCurSrcByte = 8 - mBitsUsed;
        uint8_t bitsToUnpack = min(unpackBitsLeft, bitsInCurSrcByte);
        uint8_t unusedLeftBits = (unpackBitsLeft >= bitsInCurSrcByte) ? 
                                0 : (bitsInCurSrcByte - unpackBitsLeft);
        uint8_t rightShift = mBitsUsed + unusedLeftBits;
        uint8_t val = mBuf[mBytesUsed] << unusedLeftBits;  // Zero out left bits
        val = val >> rightShift;      // Shift bits to right most position for this byte
        ret |= (uint32_t(val) << valShift);
        valShift += bitsToUnpack;
        if (mBitsUsed + bitsToUnpack == 8)
        {
            mBitsUsed = 0;
            mBytesUsed++;
        }
        else
        {
            mBitsUsed += bitsToUnpack;
        }
        unpackBitsLeft -= bitsToUnpack;
    } while (unpackBitsLeft != 0);
    return ret;
}

void CAPacket::unpackerString(String& str) {
    CA_ASSERT(mState == STATE_UNPACKER, "Error in unpackerString");
    uint16_t len = strlen((char*)&mBuf[mBytesUsed]);
    str = (char*)(&mBuf[mBytesUsed]);
    mBytesUsed += len+1;  // +1 for the null terminator
}

void CAPacket::packer(uint32_t val, uint8_t packBits)
{
    CA_ASSERT(mState == STATE_PACKER, "Error in packer");
    uint8_t packBitsLeft = packBits;

    do
    {
        uint8_t bitsInCurDstByte = 8 - mBitsUsed;
        uint8_t bitsToPack = min(bitsInCurDstByte, packBitsLeft);
        mBitsVal |= (val << mBitsUsed);
        mBitsUsed += bitsToPack;
        val = val >> bitsToPack;
        if (mBitsUsed == 8) // When byte is full write it's value
        {
            //CAU::log("%d, ", mBitsVal);
            mBuf[mBytesUsed++] = mBitsVal;
            mBitsVal = 0;
            mBitsUsed = 0;
        }
        packBitsLeft -= bitsToPack;
    } while (packBitsLeft != 0);
}

void CAPacket::packerString(const char* src){
    CA_ASSERT(mState == STATE_PACKER, "Error in packerString");
    for(uint8_t val=0; val<strlen(src); val++) {
        (mBuf[mBytesUsed++]) = src[val];
    }
    mBuf[mBytesUsed++] = 0; // Add null terminator
}

void CAPacket::flushPacket() {
    CA_ASSERT((mBitsUsed == 0) && (mBitsVal == 0) && (mBytesUsed <= mBufSize),
                "Error in flushPacket");
}

///////////////////////////////////////////////////////////////////////////////
// String Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketString::CAPacketString(CAPacket& caPacket) {
    mClientHostId = 0;
    mCAP = &caPacket;
}

void CAPacketString::set(uint8_t clientHostId, String str) {
    CA_ASSERT(str.length() < MAX_STRING_SIZE, "String too long");
    mClientHostId = clientHostId;
    mString = str;
}

void CAPacketString::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mString = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_STRING, "Wrong PID ID");
    CA_ASSERT(mString.length() < MAX_STRING_SIZE, "String too long");
}

void CAPacketString::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mCAP->unpackerString(mString);
    mCAP->flushPacket();
	CA_ASSERT(mString.length() < MAX_STRING_SIZE, "String too long");
}

uint16_t CAPacketString::pack() {
    uint16_t len = mString.length()+1;  // 1 for the null terminator
    uint16_t packetSize = PACK_TOTAL_SZ + 1 + len;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_STRING, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packerString(mString.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketString::packetToString(String& str) {
    str = (String)PID_STRING + '~' + mClientHostId + '~' + mString + '~';
}

///////////////////////////////////////////////////////////////////////////////
// Uint32 Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketUint32::CAPacketUint32(CAPacket& caPacket) {
    mClientHostId = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketUint32::set(uint8_t clientHostId, uint32_t value) {
    mClientHostId = clientHostId;
    mValue = value;
}

void CAPacketUint32::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mValue = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_UINT32, "Wrong PID ID");
}

void CAPacketUint32::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mValue = mCAP->unpacker(32);
    mCAP->flushPacket();
}

uint16_t CAPacketUint32::pack() {
    uint16_t packetSize = PACK_TOTAL_SZ + 5;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_UINT32, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mValue, 32);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketUint32::packetToString(String& str) {
    str = (String)PID_UINT32 + '~' + mClientHostId + '~' + mValue + '~';
}

///////////////////////////////////////////////////////////////////////////////
// TimeBox Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketTimeBox::CAPacketTimeBox(CAPacket& caPacket) {
    mClientHostId = 0;
    mSeconds = 0;
    mNanoseconds = 0;
    mCAP = &caPacket;
}

void CAPacketTimeBox::set(uint8_t clientHostId, uint32_t seconds, uint32_t nanoseconds) {
    mClientHostId = clientHostId;
    mSeconds = seconds;
    mNanoseconds = nanoseconds;
}

void CAPacketTimeBox::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mSeconds = getUint32FromString(index, str);
    mNanoseconds = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_TIME_BOX, "Wrong PID ID");
}

void CAPacketTimeBox::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mSeconds = mCAP->unpacker(32);
    mNanoseconds = mCAP->unpacker(32);
    mCAP->flushPacket();
}

uint16_t CAPacketTimeBox::pack() {
    uint16_t packetSize = PACK_TOTAL_SZ + 9;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_TIME_BOX, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mSeconds, 32);
    mCAP->packer(mNanoseconds, 32);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketTimeBox::packetToString(String& str) {
    str = (String)PID_TIME_BOX + '~' + mClientHostId + '~' + mSeconds + '~' + mNanoseconds + '~';
}

///////////////////////////////////////////////////////////////////////////////
// MenuSelect Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuSelect::CAPacketMenuSelect(CAPacket& caPacket) {
    mMenuMode = 1;
    mCAP = &caPacket;
}

void CAPacketMenuSelect::set(uint8_t menuMode, String menuName) {
    mMenuMode = menuMode;
    mMenuName = menuName;
    CA_ASSERT((mMenuMode <= 1), "Error in CAPacketMenuSelect::set()");
    CA_ASSERT(mMenuName.length() < MAX_STRING_SIZE, "String too long");
}

void CAPacketMenuSelect::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMenuMode = getUint32FromString(index, str);
    mMenuName = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_MENU_SELECT, "Wrong PID ID");
    CA_ASSERT((mMenuMode <= 1), "Error in CAPacketMenuSelect::set()");
    CA_ASSERT(mMenuName.length() < MAX_STRING_SIZE, "String too long");
}

void CAPacketMenuSelect::unpack() {
    mMenuMode = mCAP->unpacker(8);
    mCAP->unpackerString(mMenuName);
    mCAP->flushPacket();
    CA_ASSERT((mMenuMode <= 1), "Error in CAPacketMenuSelect::unpack)");
    CA_ASSERT(mMenuName.length() < MAX_STRING_SIZE, "String too long");
}

uint16_t CAPacketMenuSelect::pack() {
    uint16_t len = mMenuName.length()+1;  // 1 for the null terminator
    uint16_t packetSize = PACK_TOTAL_SZ + 1 + len;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MENU_SELECT, 8);
    mCAP->packer(mMenuMode, 8);
    mCAP->packerString(mMenuName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketMenuSelect::packetToString(String& str) {
    str = (String)PID_MENU_SELECT + '~' + mMenuMode + '~' + mMenuName + '~';
}

///////////////////////////////////////////////////////////////////////////////
// CamSettings Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketCamSettings::CAPacketCamSettings() {
    mCamPortNumber = 0;
    mMode = CA_MODE_CAMERA;
    mDelaySeconds = 0;
    mDelayNanoseconds = 0;
    mDurationSeconds = 0;
    mDurationNanoseconds = 0;
    mPostDelaySeconds = 0;
    mPostDelayNanoseconds = 0;
    mSequencer = 0;
    mMirrorLockup = 0;
    mCAP = NULL;
}

CAPacketCamSettings::CAPacketCamSettings(CAPacket& caPacket) {
    CAPacketCamSettings();
    mCAP = &caPacket;
}

void CAPacketCamSettings::set(uint8_t camPortNumber, uint8_t mode, uint32_t delaySeconds, uint32_t delayNanoSeconds,
                uint32_t durationSeconds, uint32_t durationNanoseconds, uint32_t postDelaySeconds,
                uint32_t postDelayNanoseconds, uint8_t sequencer, uint8_t mirrorLockup)
 {
    mCamPortNumber = camPortNumber;
    mMode = mode;
    mDelaySeconds = delaySeconds;
    mDelayNanoseconds = delayNanoSeconds;
    mDurationSeconds = durationSeconds;
    mDurationNanoseconds = durationNanoseconds;
    mPostDelaySeconds = postDelaySeconds;
    mPostDelayNanoseconds = postDelayNanoseconds;
    mSequencer = sequencer;
    mMirrorLockup = mirrorLockup;
    CA_ASSERT((mMode <= CA_MODE_FLASH) && (mMirrorLockup <= 1),
                "Error in CAPacketCamSettings::set()");
}

void CAPacketCamSettings::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mCamPortNumber = getUint32FromString(index, str);
    mMode = getUint32FromString(index, str);
    mDelaySeconds = getUint32FromString(index, str);
    mDelayNanoseconds = getUint32FromString(index, str);
    mDurationSeconds = getUint32FromString(index, str);
    mDurationNanoseconds = getUint32FromString(index, str);
    mPostDelaySeconds = getUint32FromString(index, str);
    mPostDelayNanoseconds = getUint32FromString(index, str);
    mSequencer = getUint32FromString(index, str);
    mMirrorLockup = getUint32FromString(index, str);
    CA_ASSERT((mMode <= CA_MODE_FLASH) && (mMirrorLockup <= 1),
                "Error in CAPacketCamSettings::set()");
    CA_ASSERT(index==str.length(), str.c_str());
    CA_ASSERT(id==PID_CAM_SETTINGS, "Wrong PID ID");
}

void CAPacketCamSettings::unpack() {
    mCamPortNumber = mCAP->unpacker(3);
    mMode = mCAP->unpacker(3);
    mDelaySeconds = mCAP->unpacker(32);
    mDelayNanoseconds = mCAP->unpacker(32);
    mDurationSeconds = mCAP->unpacker(32);
    mDurationNanoseconds = mCAP->unpacker(32);
    mPostDelaySeconds = mCAP->unpacker(32);
    mPostDelayNanoseconds = mCAP->unpacker(32);
    mSequencer = mCAP->unpacker(8);
    mMirrorLockup = mCAP->unpacker(1);
    mCAP->unpacker(1); // Unused
    mCAP->flushPacket();
    CA_ASSERT((mMode <= CA_MODE_FLASH) && (mMirrorLockup <= 1),
                "Error in CAPacketCamSettings::unpack()");
}

uint16_t CAPacketCamSettings::pack() {
    uint8_t unused = 0;
    uint16_t packetSize = PACK_TOTAL_SZ + 26;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CAM_SETTINGS, 8);
    mCAP->packer(mCamPortNumber, 3);
    mCAP->packer(mMode, 3);
    mCAP->packer(mDelaySeconds, 32);
    mCAP->packer(mDelayNanoseconds, 32);
    mCAP->packer(mDurationSeconds, 32);
    mCAP->packer(mDurationNanoseconds, 32);
    mCAP->packer(mPostDelaySeconds, 32);
    mCAP->packer(mPostDelayNanoseconds, 32);
    mCAP->packer(mSequencer, 8);
    mCAP->packer(mMirrorLockup, 1);
    mCAP->packer(unused, 1);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketCamSettings::packetToString(String& str) {
    str = (String)PID_CAM_SETTINGS + '~' + mCamPortNumber + '~' + mMode + '~' + mDelaySeconds + '~' + 
            mDelayNanoseconds + '~' + mDurationSeconds + '~' + mDurationNanoseconds + '~' + mPostDelaySeconds + 
            '~' + mPostDelayNanoseconds + '~' + mSequencer + '~' + mMirrorLockup + '~';
}

///////////////////////////////////////////////////////////////////////////////
// Intervalometer Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketIntervalometer::CAPacketIntervalometer(CAPacket& caPacket) {
    mEnable = 0;
    mStartSeconds = 0;
    mStartNanoseconds = 0;
    mIntervalSeconds = 0;
    mIntervalNanoseconds = 0;
    mRepeats = 0;
    mCAP = &caPacket;
}

void CAPacketIntervalometer::set(uint8_t enable, uint32_t startSeconds, uint32_t startNanoseconds,
                                    uint32_t intervalSeconds, uint32_t intervalNanoseconds, uint16_t repeats) {
    mEnable = enable;
    mStartSeconds = startSeconds;
    mStartNanoseconds = startNanoseconds;
    mIntervalSeconds = intervalSeconds;
    mIntervalNanoseconds = intervalNanoseconds;
    mRepeats = repeats;
}

void CAPacketIntervalometer::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mEnable = getUint32FromString(index, str);
    mStartSeconds = getUint32FromString(index, str);
    mStartNanoseconds = getUint32FromString(index, str);
    mIntervalSeconds = getUint32FromString(index, str);
    mIntervalNanoseconds = getUint32FromString(index, str);
    mRepeats = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_INTERVALOMETER, "Wrong PID ID");
}

void CAPacketIntervalometer::unpack() {
    mEnable = mCAP->unpacker(8);
    mStartSeconds = mCAP->unpacker(32);
    mStartNanoseconds = mCAP->unpacker(32);
    mIntervalSeconds = mCAP->unpacker(32);
    mIntervalNanoseconds = mCAP->unpacker(32);
    mRepeats = mCAP->unpacker(16);
    mCAP->flushPacket();
    CA_ASSERT((mEnable <= 1), "Error in CAPacketIntervalometer::unpack()");
}

uint16_t CAPacketIntervalometer::pack() {
    uint16_t packetSize = PACK_TOTAL_SZ + 19;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_INTERVALOMETER, 8);
    mCAP->packer(mEnable, 8);
    mCAP->packer(mStartSeconds, 32);
    mCAP->packer(mStartNanoseconds, 32);
    mCAP->packer(mIntervalSeconds, 32);
    mCAP->packer(mIntervalNanoseconds, 32);
    mCAP->packer(mRepeats, 16);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketIntervalometer::packetToString(String& str) {
    str = (String)PID_INTERVALOMETER + '~' + mEnable + '~' + mStartSeconds + '~' + mStartNanoseconds + '~' +
                mIntervalSeconds + '~' + mIntervalNanoseconds + '~' + mRepeats + '~';
}

///////////////////////////////////////////////////////////////////////////////
// CamTrigger Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketCamTrigger::CAPacketCamTrigger(CAPacket& caPacket) {
    mMode = 0;
    mFocus = 0;
    mShutter = 0;
    mCAP = &caPacket;
}

void CAPacketCamTrigger::set(uint8_t mode, uint32_t focus, uint32_t shutter) {
    mMode = mode;
    mFocus = focus;
    mShutter = shutter;
}

void CAPacketCamTrigger::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMode = getUint32FromString(index, str);
    mFocus = getUint32FromString(index, str);
    mShutter = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_CAM_TRIGGER, "Wrong PID ID");
}

void CAPacketCamTrigger::unpack() {
    mMode = mCAP->unpacker(8);
    mFocus = mCAP->unpacker(8);
    mShutter = mCAP->unpacker(8);
    mCAP->flushPacket();
    CA_ASSERT((mFocus==0) && (mShutter==0), "Error in CAPacketCamTrigger::unpack()");
}

uint16_t CAPacketCamTrigger::pack() {
    uint16_t packetSize = PACK_TOTAL_SZ + 3;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CAM_TRIGGER, 8);
    mCAP->packer(mMode, 8);
    mCAP->packer(mFocus, 8);
    mCAP->packer(mShutter, 8);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketCamTrigger::packetToString(String& str) {
    str = (String)PID_CAM_TRIGGER + '~' + mMode + '~' + mFocus + '~' + mShutter + '~';
}

///////////////////////////////////////////////////////////////////////////////
// PeriodicData Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketPeriodicData::CAPacketPeriodicData(CAPacket& caPacket) {
    mVoltage = 0;
    mCAP = &caPacket;
}

void CAPacketPeriodicData::set(uint16_t voltage) {
    mVoltage = voltage;
}

void CAPacketPeriodicData::set(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mVoltage = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_PERIODIC_DATA, "Wrong PID ID");
}

void CAPacketPeriodicData::unpack() {
    mVoltage = mCAP->unpacker(16);
    mCAP->flushPacket();
}

uint16_t CAPacketPeriodicData::pack() {
    uint16_t packetSize = PACK_TOTAL_SZ + 2;
    mCAP->packer(GUARD_PACKET, 8);
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_PERIODIC_DATA, 8);
    mCAP->packer(mVoltage, 16);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketPeriodicData::packetToString(String& str) {
    str = (String)PID_PERIODIC_DATA + '~' + mVoltage + '~';
}

