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
    mFlags = 0;
    mCAP = &caPacket;
}

void CAPacketString::set(uint8_t clientHostId, uint8_t flags, String str) {
    mClientHostId = clientHostId;
    mFlags = flags;
    mString = str;
    CA_ASSERT(mFlags <= 2,
        "Error in CAPacketString::set()");
}

void CAPacketString::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mFlags = mCAP->unpacker(8);
    mCAP->unpackerString(mString);
    mCAP->flushPacket();
    CA_ASSERT(mFlags <= 2,
        "Error in CAPacketString::unpack()");
}

uint16_t CAPacketString::pack() {
    uint16_t len = mString.length()+1;  // 1 for the null terminator
    uint16_t packetSize = 3 + 2 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_STRING, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mFlags, 8);
    mCAP->packerString(mString.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketString::packetToString(String& str) {
    str = (String)PID_STRING + '|' + mClientHostId + '|' + mFlags + '|' + mString + '|';
}

void CAPacketString::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mFlags = getUint32FromString(index, str);
    mString = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_STRING, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// Uint32 Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketUint32::CAPacketUint32(CAPacket& caPacket) {
    mClientHostId = 0;
    mFlags = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketUint32::set(uint8_t clientHostId, uint8_t flags, uint32_t value) {
    mClientHostId = clientHostId;
    mFlags = flags;
    mValue = value;
    CA_ASSERT(mFlags <= 2, "Error in CAPacketUint32::set()");
}

void CAPacketUint32::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mFlags = mCAP->unpacker(8);
    mValue = mCAP->unpacker(32);
    mCAP->flushPacket();
    CA_ASSERT(mFlags <= 2, "Error in CAPacketUint32::unpack()");
}

uint16_t CAPacketUint32::pack() {
    uint16_t packetSize = 3 + 6;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_UINT32, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mFlags, 8);
    mCAP->packer(mValue, 32);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketUint32::packetToString(String& str) {
    str = (String)PID_UINT32 + '|' + mClientHostId + '|' + mFlags + '|' + mValue + '|';
}

void CAPacketUint32::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mFlags = getUint32FromString(index, str);
    mValue = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_UINT32, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// TimeBox Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketTimeBox::CAPacketTimeBox(CAPacket& caPacket) {
    mClientHostId = 0;
    mFlags = 0;
    mHours = 0;
    mMinutes = 0;
    mSeconds = 0;
    mMilliseconds = 0;
    mMicroseconds = 0;
    mNanoseconds = 0;
    mCAP = &caPacket;
}

void CAPacketTimeBox::set(uint8_t clientHostId, uint8_t flags, uint16_t hours, uint8_t minutes,
                            uint8_t seconds, uint16_t milliseconds, uint16_t microseconds, uint16_t nanoseconds) {
    mClientHostId = clientHostId;
    mFlags = flags;
    mHours = hours;
    mMinutes = minutes;
    mSeconds = seconds;
    mMilliseconds = milliseconds;
    mMicroseconds = microseconds;
    mNanoseconds = nanoseconds;
    CA_ASSERT((mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) && (mMilliseconds <= 999) &&
                (mMicroseconds <= 999) && (mNanoseconds <= 999) && (mFlags <= 2), "Error in CAPacketTimeBox::set()");
}

void CAPacketTimeBox::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mFlags = mCAP->unpacker(8);
    mHours = mCAP->unpacker(10);
    mMinutes = mCAP->unpacker(6);
    mSeconds = mCAP->unpacker(6);
    mMilliseconds = mCAP->unpacker(10);
    mMicroseconds = mCAP->unpacker(10);
    mNanoseconds = mCAP->unpacker(10);
    mCAP->unpacker(4); // Unused
    mCAP->flushPacket();
    CA_ASSERT((mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) && (mMilliseconds <= 999) && 
                (mMicroseconds <= 999) && (mNanoseconds <= 999) && (mFlags <= 2),
                "Error in CAPacketTimeBox::set()");
}

uint16_t CAPacketTimeBox::pack() {
    uint8_t unused = 0;
    uint16_t packetSize = 3 + 9;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_TIME_BOX, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mFlags, 8);
    mCAP->packer(mHours, 10);
    mCAP->packer(mMinutes, 6);
    mCAP->packer(mSeconds, 6);
    mCAP->packer(mMilliseconds, 10);
    mCAP->packer(mMicroseconds, 10);
    mCAP->packer(mNanoseconds, 10);
    mCAP->packer(unused, 4);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketTimeBox::packetToString(String& str) {
    str = (String)PID_TIME_BOX + '|' + mClientHostId + '|' + mFlags + '|' + mHours + '|' + mMinutes + '|' + 
            mSeconds + '|' + mMilliseconds + '|' + mMicroseconds + '|' + mNanoseconds + '|';
}

void CAPacketTimeBox::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mClientHostId = getUint32FromString(index, str);
    mFlags = getUint32FromString(index, str);
    mHours = getUint32FromString(index, str);
    mMinutes = getUint32FromString(index, str);
    mSeconds = getUint32FromString(index, str);
    mMilliseconds = getUint32FromString(index, str);
    mMicroseconds = getUint32FromString(index, str);
    mNanoseconds = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_TIME_BOX, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// MenuSelect Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuSelect::CAPacketMenuSelect(CAPacket& caPacket) {
    mMode = 0;
    mMenuNumber = 0;
    mCAP = &caPacket;
}

void CAPacketMenuSelect::set(uint8_t mode, uint8_t menuNumber) {
    mMode = mode;
    mMenuNumber = menuNumber;
    CA_ASSERT((mMode <= 1), "Error in CAPacketMenuSelect::set()");
}

void CAPacketMenuSelect::unpack() {
    mMode = mCAP->unpacker(8);
    mMenuNumber = mCAP->unpacker(8);
    mCAP->flushPacket();
    CA_ASSERT((mMode <= 1), "Error in CAPacketMenuSelect::unpack()");
}

uint16_t CAPacketMenuSelect::pack() {
    uint16_t packetSize = 3 + 2;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MENU_SELECT, 8);
    mCAP->packer(mMode, 8);
    mCAP->packer(mMenuNumber, 8);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketMenuSelect::packetToString(String& str) {
    str = (String)PID_MENU_SELECT + '|' + mMode + '|' + mMenuNumber + '|';
}

void CAPacketMenuSelect::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMode = getUint32FromString(index, str);
    mMenuNumber = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_MENU_SELECT, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// MenuList Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuList::CAPacketMenuList(CAPacket& caPacket) {
    mMenuId = 0;
    mModuleId0 = 0;
    mModuleMask0 = 0;
    mModuleId1 = 0;
    mModuleMask1 = 0;
    mModuleId2 = 0;
    mModuleMask2 = 0;
    mModuleId3 = 0;
    mModuleMask3 = 0;
    mModuleTypeId0 = 0;
    mModuleTypeMask0 = 0;
    mModuleTypeId1 = 0;
    mModuleTypeMask1 = 0;
    mCAP = &caPacket;
}

void CAPacketMenuList::set(uint8_t menuId, uint8_t moduleId0, uint8_t moduleMask0,  uint8_t moduleId1,
                uint8_t moduleMask1, uint8_t moduleId2, uint8_t moduleMask2, uint8_t moduleId3, uint8_t moduleMask3,
                uint8_t moduleTypeId0, uint8_t moduleTypeMask0, uint8_t moduleTypeId1, uint8_t moduleTypeMask1,
                String menuName) {
    mMenuId = menuId;
    mModuleId0 = moduleId0;
    mModuleMask0 = moduleMask0;
    mModuleId1 = moduleId1;
    mModuleMask1 = moduleMask1;
    mModuleId2 = moduleId2;
    mModuleMask2 = moduleMask2;
    mModuleId3 = moduleId3;
    mModuleMask3 = moduleMask3;
    mModuleTypeId0 = moduleTypeId0;
    mModuleTypeMask0 = moduleTypeMask0;
    mModuleTypeId1 = moduleTypeId1;
    mModuleTypeMask1 = moduleTypeMask1;
    mMenuName = menuName;
    CA_ASSERT(mModuleMask0 <= 0xf && mModuleMask1 <= 0xf &&
        mModuleMask2 <= 0xf && mModuleMask3 <= 0xf &&
        mModuleTypeMask0 <= 0xf && mModuleTypeMask1 <= 0xf , "Error in MenuList::set()");
}

void CAPacketMenuList::unpack() {
    mMenuId = mCAP->unpacker(8);
    mModuleId0 = mCAP->unpacker(8);
    mModuleMask0 = mCAP->unpacker(4);
    mModuleId1 = mCAP->unpacker(8);
    mModuleMask1 = mCAP->unpacker(4);
    mModuleId2 = mCAP->unpacker(8);
    mModuleMask2 = mCAP->unpacker(4);
    mModuleId3 = mCAP->unpacker(8);
    mModuleMask3 = mCAP->unpacker(4);
    mModuleTypeId0 = mCAP->unpacker(8);
    mModuleTypeMask0 = mCAP->unpacker(4);
    mModuleTypeId1 = mCAP->unpacker(8);
    mModuleTypeMask1 = mCAP->unpacker(4);
    mCAP->unpackerString(mMenuName);
    mCAP->flushPacket();
    CA_ASSERT(mModuleMask0 <= 0xf && mModuleMask1 <= 0xf &&
            mModuleMask2 <= 0xf && mModuleMask3 <= 0xf &&
            mModuleTypeMask0 <= 0xf && mModuleTypeMask1 <= 0xf , "Error in MenuList::unpack()");
}

uint16_t CAPacketMenuList::pack() {
    uint16_t len = mMenuName.length() + 1;  // 1 for the null terminator
    uint16_t packetSize = 3 + 10 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MENU_LIST, 8);
    mCAP->packer(mMenuId, 8);
    mCAP->packer(mModuleId0, 8);
    mCAP->packer(mModuleMask0, 4);
    mCAP->packer(mModuleId1, 8);
    mCAP->packer(mModuleMask1, 4);
    mCAP->packer(mModuleId2, 8);
    mCAP->packer(mModuleMask2, 4);
    mCAP->packer(mModuleId3, 8);
    mCAP->packer(mModuleMask3, 4);
    mCAP->packer(mModuleTypeId0, 8);
    mCAP->packer(mModuleTypeMask0, 4);
    mCAP->packer(mModuleTypeId1, 8);
    mCAP->packer(mModuleTypeMask1, 4);
    mCAP->packerString(mMenuName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketMenuList::packetToString(String& str) {
    str = (String)PID_MENU_LIST + '|' + mMenuId + '|' + mModuleId0 + '|' + mModuleMask0 + '|' + 
            mModuleId1 + '|' + mModuleMask1 + '|' + mModuleId2 + '|' + mModuleMask2 + '|' +
            mModuleId3 + '|' + mModuleMask3 + '|' + mModuleTypeId0 + '|' + mModuleTypeMask0 + '|' +
            mModuleTypeId1 + '|' + mModuleTypeMask1 + '|' + mMenuName + '|';
}

void CAPacketMenuList::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMenuId = getUint32FromString(index, str);
    mModuleId0 = getUint32FromString(index, str);
    mModuleMask0 = getUint32FromString(index, str);
    mModuleId1 = getUint32FromString(index, str);
    mModuleMask1 = getUint32FromString(index, str);
    mModuleId2 = getUint32FromString(index, str);
    mModuleMask2 = getUint32FromString(index, str);
    mModuleId3 = getUint32FromString(index, str);
    mModuleMask3 = getUint32FromString(index, str);
    mModuleTypeId0 = getUint32FromString(index, str);
    mModuleTypeMask0 = getUint32FromString(index, str);
    mModuleTypeId1 = getUint32FromString(index, str);
    mModuleTypeMask1 = getUint32FromString(index, str);
    mMenuName = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_MENU_LIST, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// ModuleList Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketModuleList::CAPacketModuleList(CAPacket& caPacket) {
    mModuleId = 0;
    mModuleTypeId = 0;
    mCAP = &caPacket;
}

void CAPacketModuleList::set(uint8_t moduleId, uint8_t moduleTypeId, String moduleName) {
    mModuleId = moduleId;
    mModuleTypeId = moduleTypeId;
    mModuleName = moduleName;
}

void CAPacketModuleList::unpack() {
    mModuleId = mCAP->unpacker(8);
    mModuleTypeId = mCAP->unpacker(8);
    mCAP->unpackerString(mModuleName);
    mCAP->flushPacket();
}

uint16_t CAPacketModuleList::pack() {
    uint16_t len = mModuleName.length() + 1;  // 1 for the null terminator
    uint16_t packetSize = 3 + 2 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MODULE_LIST, 8);
    mCAP->packer(mModuleId, 8);
    mCAP->packer(mModuleTypeId, 8);
    mCAP->packerString(mModuleName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketModuleList::packetToString(String& str) {
    str = (String)PID_MODULE_LIST + '|' + mModuleId + '|' + mModuleTypeId + '|' + mModuleName + '|';
}

void CAPacketModuleList::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mModuleId = getUint32FromString(index, str);
    mModuleTypeId = getUint32FromString(index, str);
    mModuleName = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_MODULE_LIST, "Wrong PID ID");
}

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

///////////////////////////////////////////////////////////////////////////////
// CamState Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketCamState::CAPacketCamState(CAPacket& caPacket) {
    mMultiplier = 0;
    mFocus = 0;
    mShutter = 0;
    mCAP = &caPacket;
}

void CAPacketCamState::set(uint8_t multiplier, uint8_t focus, uint8_t shutter) {
    mMultiplier = multiplier;
    mFocus = focus;
    mShutter = shutter;
}

void CAPacketCamState::unpack() {
    mMultiplier = mCAP->unpacker(8);
    mFocus = mCAP->unpacker(8);
    mShutter = mCAP->unpacker(8);
    mCAP->flushPacket();
}

uint16_t CAPacketCamState::pack() {
    uint16_t packetSize = 3 + 3;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CAM_STATE, 8);
    mCAP->packer(mMultiplier, 8);
    mCAP->packer(mFocus, 8);
    mCAP->packer(mShutter, 8);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketCamState::packetToString(String& str) {
    str = (String)PID_CAM_STATE + '|' + mMultiplier + '|' + mFocus + '|' + mShutter + '|';
}

void CAPacketCamState::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMultiplier = getUint32FromString(index, str);
    mFocus = getUint32FromString(index, str);
    mShutter = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_CAM_STATE, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// CamSettings Packet Class ** Derived from CamSettingsBase
///////////////////////////////////////////////////////////////////////////////
CAPacketCamSettings::CAPacketCamSettings(CAPacket& caPacket) {
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
    mCAP = &caPacket;
}

void CAPacketCamSettings::unpack() {
    mCamPortNumber = mCAP->unpacker(8);
    mMode = mCAP->unpacker(2);
    mDelayHours = mCAP->unpacker(10);
    mDelayMinutes = mCAP->unpacker(6);
    mDelaySeconds = mCAP->unpacker(6);
    mDelayMilliseconds = mCAP->unpacker(10);
    mDelayMicroseconds = mCAP->unpacker(10);
    mDurationHours = mCAP->unpacker(10);
    mDurationMinutes = mCAP->unpacker(6);
    mDurationSeconds = mCAP->unpacker(6);
    mDurationMilliseconds = mCAP->unpacker(10);
    mDurationMicroseconds = mCAP->unpacker(10);
    mSequencer = mCAP->unpacker(8);
    mApplyIntervalometer = mCAP->unpacker(1);
    mSmartPreview = mCAP->unpacker(6);
    mMirrorLockupEnable = mCAP->unpacker(1);
    mMirrorLockupMinutes = mCAP->unpacker(6);
    mMirrorLockupSeconds = mCAP->unpacker(6);
    mMirrorLockupMilliseconds = mCAP->unpacker(10);
    mCAP->unpacker(4); // Unused
    mCAP->flushPacket();
    CA_ASSERT((mMode <= 2) && (mDelayHours <= 999) && (mDelayMinutes <=59) &&
                (mDelaySeconds <= 59) && (mDelayMilliseconds <= 999) && (mDelayMicroseconds <= 999) &&
                (mDurationHours <= 999) && (mDurationMinutes <= 59) && (mDurationSeconds <= 59) &&
                (mDurationMilliseconds <= 999) && (mDurationMicroseconds <= 999) && (mApplyIntervalometer <= 1) &&
                (mSmartPreview <= 59) && (mMirrorLockupEnable <= 1) && (mMirrorLockupMinutes <= 59) &&
                (mMirrorLockupSeconds <= 59) && (mMirrorLockupMilliseconds <= 999),
                "Error in CAPacketCamSettings::unpack()");
}

uint16_t CAPacketCamSettings::pack() {
    uint8_t unused = 0;
    uint16_t packetSize = 3 + 17;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CAM_SETTINGS, 8);
    mCAP->packer(mCamPortNumber, 8);
    mCAP->packer(mMode, 2);
    mCAP->packer(mDelayHours, 10);
    mCAP->packer(mDelayMinutes, 6);
    mCAP->packer(mDelaySeconds, 6);
    mCAP->packer(mDelayMilliseconds, 10);
    mCAP->packer(mDelayMicroseconds, 10);
    mCAP->packer(mDurationHours, 10);
    mCAP->packer(mDurationMinutes, 6);
    mCAP->packer(mDurationSeconds, 6);
    mCAP->packer(mDurationMilliseconds, 10);
    mCAP->packer(mDurationMicroseconds, 10);
    mCAP->packer(mSequencer, 8);
    mCAP->packer(mApplyIntervalometer, 1);
    mCAP->packer(mSmartPreview, 6);
    mCAP->packer(mMirrorLockupEnable, 1);
    mCAP->packer(mMirrorLockupMinutes, 6);
    mCAP->packer(mMirrorLockupSeconds, 6);
    mCAP->packer(mMirrorLockupMilliseconds, 10);
    mCAP->packer(unused, 4);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketCamSettings::packetToString(String& str) {
    str = (String)PID_CAM_SETTINGS + '|' + mCamPortNumber + '|' + mMode + '|' + mDelayHours + '|' + mDelayMinutes +
            '|' + mDelaySeconds + '|' + mDelayMilliseconds + '|' + mDelayMicroseconds + '|' + mDurationHours + '|' +
            mDurationMinutes + '|' + mDurationSeconds + '|' + mDurationMilliseconds + '|' + mDurationMicroseconds +
            '|' + mSequencer + '|' + mApplyIntervalometer + '|' + mSmartPreview + '|' + mMirrorLockupEnable + '|' +
            mMirrorLockupMinutes + '|' + mMirrorLockupSeconds + '|' + mMirrorLockupMilliseconds + '|';
}

void CAPacketCamSettings::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mCamPortNumber = getUint32FromString(index, str);
    mMode = getUint32FromString(index, str);
    mDelayHours = getUint32FromString(index, str);
    mDelayMinutes = getUint32FromString(index, str);
    mDelaySeconds = getUint32FromString(index, str);
    mDelayMilliseconds = getUint32FromString(index, str);
    mDelayMicroseconds = getUint32FromString(index, str);
    mDurationHours = getUint32FromString(index, str);
    mDurationMinutes = getUint32FromString(index, str);
    mDurationSeconds = getUint32FromString(index, str);
    mDurationMilliseconds = getUint32FromString(index, str);
    mDurationMicroseconds = getUint32FromString(index, str);
    mSequencer = getUint32FromString(index, str);
    mApplyIntervalometer = getUint32FromString(index, str);
    mSmartPreview = getUint32FromString(index, str);
    mMirrorLockupEnable = getUint32FromString(index, str);
    mMirrorLockupMinutes = getUint32FromString(index, str);
    mMirrorLockupSeconds = getUint32FromString(index, str);
    mMirrorLockupMilliseconds = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_CAM_SETTINGS, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// Intervalometer Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketIntervalometer::CAPacketIntervalometer(CAPacket& caPacket) {
    mStartHours = 0;
    mStartMinutes = 0;
    mStartSeconds = 0;
    mStartMilliseconds = 0;
    mStartMicroseconds = 0;
    mIntervalHours = 0;
    mIntervalMinutes = 0;
    mIntervalSeconds = 0;
    mIntervalMilliseconds = 0;
    mIntervalMicroseconds = 0;
    mRepeats = 0;
    mCAP = &caPacket;
}

void CAPacketIntervalometer::set(uint16_t startHours, uint8_t startMinutes, uint8_t startSeconds, uint16_t startMilliseconds,
                uint16_t startMicroseconds, uint16_t intervalHours, uint8_t intervalMinutes,
                uint8_t intervalSeconds, uint16_t intervalMilliseconds, uint16_t intervalMicroseconds,
                uint16_t repeats) {
    mStartHours = startHours;
    mStartMinutes = startMinutes;
    mStartSeconds = startSeconds;
    mStartMilliseconds = startMilliseconds;
    mStartMicroseconds = startMicroseconds;
    mIntervalHours = intervalHours;
    mIntervalMinutes = intervalMinutes;
    mIntervalSeconds = intervalSeconds;
    mIntervalMilliseconds = intervalMilliseconds;
    mIntervalMicroseconds = intervalMicroseconds;
    mRepeats = repeats;
    CA_ASSERT((mStartHours <= 999) && (mStartMinutes <= 59) && (mStartSeconds <=59) &&
                (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) && (mIntervalHours <= 999) &&
                (mIntervalMinutes <= 59) && (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::set()");
}

void CAPacketIntervalometer::unpack() {
    mStartHours = mCAP->unpacker(10);
    mStartMinutes = mCAP->unpacker(6);
    mStartSeconds = mCAP->unpacker(6);
    mStartMilliseconds = mCAP->unpacker(10);
    mStartMicroseconds = mCAP->unpacker(10);
    mIntervalHours = mCAP->unpacker(10);
    mIntervalMinutes = mCAP->unpacker(6);
    mIntervalSeconds = mCAP->unpacker(6);
    mIntervalMilliseconds = mCAP->unpacker(10);
    mIntervalMicroseconds = mCAP->unpacker(10);
    mRepeats = mCAP->unpacker(16);
    mCAP->unpacker(4);  // Unused
    mCAP->flushPacket();
    CA_ASSERT((mStartHours <= 999) && (mStartMinutes <= 59) && (mStartSeconds <=59) &&
                (mStartMilliseconds <= 999) && (mStartMicroseconds <= 999) && (mIntervalHours <= 999) &&
                (mIntervalMinutes <= 59) && (mIntervalSeconds <= 59) && (mIntervalMilliseconds <= 999) &&
                (mIntervalMicroseconds <= 999), "Error in CAPacketIntervalometer::unpack()");
}

uint16_t CAPacketIntervalometer::pack() {
    uint8_t unused = 0;
    uint16_t packetSize = 3 + 13;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_INTERVALOMETER, 8);
    mCAP->packer(mStartHours, 10);
    mCAP->packer(mStartMinutes, 6);
    mCAP->packer(mStartSeconds, 6);
    mCAP->packer(mStartMilliseconds, 10);
    mCAP->packer(mStartMicroseconds, 10);
    mCAP->packer(mIntervalHours, 10);
    mCAP->packer(mIntervalMinutes, 6);
    mCAP->packer(mIntervalSeconds, 6);
    mCAP->packer(mIntervalMilliseconds, 10);
    mCAP->packer(mIntervalMicroseconds, 10);
    mCAP->packer(mRepeats, 16);
    mCAP->packer(unused, 4);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketIntervalometer::packetToString(String& str) {
    str = (String)PID_INTERVALOMETER + '|' + mStartHours + '|' + mStartMinutes + '|' + mStartSeconds + '|' +
                mStartMilliseconds + '|' + mStartMicroseconds + '|' + mIntervalHours + '|' + mIntervalMinutes + '|'+
                mIntervalSeconds + '|' + mIntervalMilliseconds + '|' + mIntervalMicroseconds + '|' + mRepeats + '|';
}

void CAPacketIntervalometer::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mStartHours = getUint32FromString(index, str);
    mStartMinutes = getUint32FromString(index, str);
    mStartSeconds = getUint32FromString(index, str);
    mStartMilliseconds = getUint32FromString(index, str);
    mStartMicroseconds = getUint32FromString(index, str);
    mIntervalHours = getUint32FromString(index, str);
    mIntervalMinutes = getUint32FromString(index, str);
    mIntervalSeconds = getUint32FromString(index, str);
    mIntervalMilliseconds = getUint32FromString(index, str);
    mIntervalMicroseconds = getUint32FromString(index, str);
    mRepeats = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_INTERVALOMETER, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// ControlFlags Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketControlFlags::CAPacketControlFlags(CAPacket& caPacket) {
    mSlaveModeEnable = 0;
    mExtraMessagesEnable = 0;
    mCAP = &caPacket;
}

void CAPacketControlFlags::set(uint8_t slaveModeEnable, uint8_t extraMessagesEnable) {
    mSlaveModeEnable = slaveModeEnable;
    mExtraMessagesEnable = extraMessagesEnable;
    CA_ASSERT((mSlaveModeEnable <= 1) && (mExtraMessagesEnable <= 1), 
                "Error in CAPacketControlFlags::set()");
}
    
void CAPacketControlFlags::unpack() {
    mSlaveModeEnable = mCAP->unpacker(1);
    mExtraMessagesEnable = mCAP->unpacker(1);
    mCAP->unpacker(6); // Unused
    mCAP->flushPacket();
    CA_ASSERT((mSlaveModeEnable <= 1) && (mExtraMessagesEnable <= 1), 
                "Error in CAPacketControlFlags::unpack()");

}

uint16_t CAPacketControlFlags::pack() {
    uint8_t unused = 0;
    uint16_t packetSize = 3 + 1;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CONTROL_FLAGS, 8);
    mCAP->packer(mSlaveModeEnable, 1);
    mCAP->packer(mExtraMessagesEnable, 1);
    mCAP->packer(unused, 6);
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketControlFlags::packetToString(String& str) {
    str = (String)PID_CONTROL_FLAGS + '|' + mSlaveModeEnable + '|' + mExtraMessagesEnable + '|';
}

void CAPacketControlFlags::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mSlaveModeEnable = getUint32FromString(index, str);
    mExtraMessagesEnable = getUint32FromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_CONTROL_FLAGS, "Wrong PID ID");
}

///////////////////////////////////////////////////////////////////////////////
// Echo Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketEcho::CAPacketEcho(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketEcho::set(uint8_t mode, String str) {
    mMode = mode;
    mString = str;
    CA_ASSERT((mMode <= 1), "Error in CAPacketEcho::set()");
}

void CAPacketEcho::unpack() {
    mMode = mCAP->unpacker(8);
    mCAP->unpackerString(mString);
    mCAP->flushPacket();
    CA_ASSERT((mMode <= 1), "Error in CAPacketEcho::unpack()");

}

uint16_t CAPacketEcho::pack() {
    uint16_t len = mString.length() + 1;  // 1 for the null terminator
    uint16_t packetSize = 3 + 1 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_ECHO, 8);
    mCAP->packer(mMode, 8);
    mCAP->packerString(mString.c_str());
    mCAP->flushPacket();
    return packetSize;
}

void CAPacketEcho::packetToString(String& str) {
    str = (String)PID_ECHO + '|' + mMode + '|' + mString + '|';
}

void CAPacketEcho::packetFromString(const String& str) {
    uint16_t index = 0;
    uint8_t id; 

    id = getUint32FromString(index, str);
    mMode = getUint32FromString(index, str);
    mString = getStringFromString(index, str);
    CA_ASSERT(index==str.length(), "Failed end check");
    CA_ASSERT(id==PID_ECHO, "Wrong PID ID");
}
