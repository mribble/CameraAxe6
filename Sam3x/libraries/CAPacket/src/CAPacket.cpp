#include <CAPacket.h>
#include <CAUtility.h>

CAPacket::CAPacket(uint8 state, uint8 *buf, uint16 bufSize) {
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

uint16 CAPacket::unpackSize() {
    return (uint16)unpacker(16);
}

uint8 CAPacket::unpackType() {
    uint8 val = (uint8)unpacker(8);
    CA_ASSERT( val > PID_START_SENTINEL  &&
               val < PID_END_SENTINEL,   "Invalid packet type");
    return val;
}
    
uint32 CAPacket::unpacker(uint8 unpackBits)
{
    CA_ASSERT(mState == STATE_UNPACKER, "Error in unpacker");
    uint8 unpackBitsLeft = unpackBits;
    uint32 ret = 0;
    uint8 valShift = 0;

    // This loop shifts through the number of bytes you want to unpack in the src buffer
    // and puts them into an uint32
    do
    {
        uint8 bitsInCurSrcByte = 8 - mBitsUsed;
        uint8 bitsToUnpack = min(unpackBitsLeft, bitsInCurSrcByte);
        uint8 unusedLeftBits = (unpackBitsLeft >= bitsInCurSrcByte) ? 
                                0 : (bitsInCurSrcByte - unpackBitsLeft);
        uint8 rightShift = mBitsUsed + unusedLeftBits;
        uint8 val = mBuf[mBytesUsed] << unusedLeftBits;  // Zero out left bits
        val = val >> rightShift;      // Shift bits to right most position for this byte
        ret |= (uint32(val) << valShift);
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
    uint16 len = strlen((char*)&mBuf[mBytesUsed]);
    str = (char*)(&mBuf[mBytesUsed]);
    mBytesUsed += len+1;  // +1 for the null terminator
}

void CAPacket::packer(uint32 val, uint8 packBits)
{
    CA_ASSERT(mState == STATE_PACKER, "Error in packer");
    uint8 packBitsLeft = packBits;

    do
    {
        uint8 bitsInCurDstByte = 8 - mBitsUsed;
        uint8 bitsToPack = min(bitsInCurDstByte, packBitsLeft);
        mBitsVal |= (val << mBitsUsed);
        mBitsUsed += bitsToPack;
        val = val >> bitsToPack;
        if (mBitsUsed == 8) // When byte is full write it's value
        {
            mBuf[mBytesUsed++] = mBitsVal;
            mBitsVal = 0;
            mBitsUsed = 0;
        }
        packBitsLeft -= bitsToPack;
    } while (packBitsLeft != 0);
}

void CAPacket::packerString(const char* src){
    CA_ASSERT(mState == STATE_PACKER, "Error in packerString");
    for(uint8 val=0; val<strlen(src); val++) {
        (mBuf[mBytesUsed++]) = src[val];
    }
    mBuf[mBytesUsed++] = 0; // Add null terminator
}

void CAPacket::flushPacket() {
    CA_ASSERT((mBitsUsed == 0) && (mBitsVal == 0) && (mBytesUsed <= mBufSize),
                "Error in flushPacket");
}

///////////////////////////////////////////////////////////////////////////////
// MenuHeader Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuHeader::CAPacketMenuHeader(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketMenuHeader::set(uint16 majorVersion, uint16 minorVersion, String menuName) {
    mMajorVersion = majorVersion;
    mMinorVersion = minorVersion;
    mMenuName = menuName;
}

void CAPacketMenuHeader::unpack() {
    mMajorVersion = mCAP->unpacker(16);
    mMinorVersion = mCAP->unpacker(16);
    mCAP->unpackerString(mMenuName);
    mCAP->flushPacket();
}

uint8 CAPacketMenuHeader::pack() {
    uint8 len = mMenuName.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 4 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MENU_HEADER, 8);
    mCAP->packer(mMajorVersion, 16);
    mCAP->packer(mMinorVersion, 16);
    mCAP->packerString(mMenuName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// TextStatic Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketTextStatic::CAPacketTextStatic(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketTextStatic::set(String text0) {
    mText0 = text0;
}

void CAPacketTextStatic::unpack() {
    mCAP->unpackerString(mText0);
    mCAP->flushPacket();
}

uint8 CAPacketTextStatic::pack() {
    uint8 len = mText0.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_TEXT_STATIC, 8);
    mCAP->packerString(mText0.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// TextDynamic Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketTextDynamic::CAPacketTextDynamic(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mCAP = &caPacket;
}

void CAPacketTextDynamic::set(uint8 clientHostId, uint8 modAttribute, String text0, String text1) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mText0 = text0;
    mText1 = text1;
    CA_ASSERT(mModAttribute <= 2,
        "Error in CAPacketTextDynamic::set()");
}

void CAPacketTextDynamic::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mCAP->unpackerString(mText0);
    mCAP->unpackerString(mText1);
    mCAP->flushPacket();
    CA_ASSERT(mModAttribute <= 2,
        "Error in CAPacketTextDynamic::unpack()");
}

uint8 CAPacketTextDynamic::pack() {
    uint8 len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
    uint8 packetSize = 3 + 2 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_TEXT_DYNAMIC, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packerString(mText0.c_str());
    mCAP->packerString(mText1.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// Button Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketButton::CAPacketButton(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mType = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketButton::set(uint8 clientHostId, uint8 modAttribute, uint8 type, uint8 value, String text0, String text1) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mType = type;
    mValue = value;
    mText0 = text0;
    mText1 = text1;
    CA_ASSERT((mType <= 1) && (mValue <= 1) && (mModAttribute <= 2),
                "Error in CAPacketButton::set()");
}

void CAPacketButton::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mType = mCAP->unpacker(4);
    mValue = mCAP->unpacker(4);
    mCAP->unpackerString(mText0);
    mCAP->unpackerString(mText1);
    mCAP->flushPacket();
    CA_ASSERT((mType <= 1) && (mValue <= 1) && (mModAttribute <= 2),
                "Error in CAPacketButton::unpack()");
}

uint8 CAPacketButton::pack() {
    uint8 len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
    uint8 packetSize = 3 + 3 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_BUTTON, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packer(mType, 4);
    mCAP->packer(mValue, 4);
    mCAP->packerString(mText0.c_str());
    mCAP->packerString(mText1.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// CheckBox Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketCheckBox::CAPacketCheckBox(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketCheckBox::set(uint8 clientHostId, uint8 modAttribute, uint8 value, String text0) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mValue = value;
    mText0 = text0;
    CA_ASSERT((mValue <= 1) && (mModAttribute <= 2), "Error in CAPacketCheckBox::set()");
}

void CAPacketCheckBox::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mValue = mCAP->unpacker(8);
    mCAP->unpackerString(mText0);
    mCAP->flushPacket();
    CA_ASSERT((mValue <= 1) && (mModAttribute <= 2), "Error in CAPacketCheckBox::unpack()");
}

uint8 CAPacketCheckBox::pack() {
    uint8 len = mText0.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 3 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CHECK_BOX, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packer(mValue, 8);
    mCAP->packerString(mText0.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// DropSelect Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketDropSelect::CAPacketDropSelect(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketDropSelect::set(uint8 clientHostId, uint8 modAttribute, uint8 value, String text0, String text1) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mValue = value;
    mText0 = text0;
    mText1 = text1;
    CA_ASSERT(mModAttribute <= 2,
        "Error in CAPacketDropSelect::set()");
}

void CAPacketDropSelect::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mValue = mCAP->unpacker(8);
    mCAP->unpackerString(mText0);
    mCAP->unpackerString(mText1);
    mCAP->flushPacket();
    CA_ASSERT(mModAttribute <= 2,
        "Error in CAPacketDropSelect::unpack()");
}

uint8 CAPacketDropSelect::pack() {
    uint8 len = mText0.length()+1+mText1.length()+1;  // 1 for the null terminator
    uint8 packetSize = 3 + 3 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_DROP_SELECT, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packer(mValue, 8);
    mCAP->packerString(mText0.c_str());
    mCAP->packerString(mText1.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// EditNumber Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketEditNumber::CAPacketEditNumber(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mDigitsBeforeDecimal = 0;
    mDigitsAfterDecimal = 0;
    mMinValue = 0;
    mMaxValue = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketEditNumber::set(uint8 clientHostId, uint8 modAttribute, uint8 digitsBeforeDecimal, 
                                uint8 digitsAfterDecimal, uint32 minValue, uint32 maxValue, uint32 value,
                                String text0 ) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mDigitsBeforeDecimal = digitsBeforeDecimal;
    mDigitsAfterDecimal = digitsAfterDecimal;
    mMinValue = minValue;
    mMaxValue = maxValue;
    mValue = value;
    mText0 = text0;
    CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                (mMinValue <= 99999999) && (mMaxValue <= 99999999) && (mModAttribute <= 2),
                "Error in CAPacketEditNumber::set()");
}

void CAPacketEditNumber::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mDigitsBeforeDecimal = mCAP->unpacker(4);
    mDigitsAfterDecimal = mCAP->unpacker(4);
    mMinValue = mCAP->unpacker(32);
    mMaxValue = mCAP->unpacker(32);
    mValue = mCAP->unpacker(32);
    mCAP->unpackerString(mText0);
    mCAP->flushPacket();
    CA_ASSERT((mDigitsBeforeDecimal <= 8) && (mDigitsAfterDecimal <= 8) &&
                (mDigitsBeforeDecimal+mDigitsAfterDecimal <= 8) &&
                (mMinValue <= 99999999) && (mMaxValue <= 99999999) && (mModAttribute <= 2),
                "Error in CAPacketEditNumber::unpack()");
}

uint8 CAPacketEditNumber::pack() {
    uint8 len = mText0.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 15 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_EDIT_NUMBER, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packer(mDigitsBeforeDecimal, 4);
    mCAP->packer(mDigitsAfterDecimal, 4);
    mCAP->packer(mMinValue, 32);
    mCAP->packer(mMaxValue, 32);
    mCAP->packer(mValue, 32);
    mCAP->packerString(mText0.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// TimeBox Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketTimeBox::CAPacketTimeBox(CAPacket& caPacket) {
    mClientHostId = 0;
    mModAttribute = 0;
    mEnableMask = 0;
    mHours = 0;
    mMinutes = 0;
    mSeconds = 0;
    mMilliseconds = 0;
    mMicroseconds = 0;
    mNanoseconds = 0;
    mCAP = &caPacket;
}

void CAPacketTimeBox::set(uint8 clientHostId, uint8 modAttribute, uint8 enableMask, uint16 hours, uint8 minutes,
                            uint8 seconds, uint16 milliseconds, uint16 microseconds, uint16 nanoseconds,
                            String text0 ) {
    mClientHostId = clientHostId;
    mModAttribute = modAttribute;
    mEnableMask = enableMask;
    mHours = hours;
    mMinutes = minutes;
    mSeconds = seconds;
    mMilliseconds = milliseconds;
    mMicroseconds = microseconds;
    mNanoseconds = nanoseconds;
    mText0 = text0;
    CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) &&
                (mMilliseconds <= 999) && (mMicroseconds <= 999) && (mNanoseconds <= 999) && (mModAttribute <= 2),
                "Error in CAPacketTimeBox::set()");
}

void CAPacketTimeBox::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mModAttribute = mCAP->unpacker(8);
    mEnableMask = mCAP->unpacker(6);
    mHours = mCAP->unpacker(10);
    mMinutes = mCAP->unpacker(6);
    mSeconds = mCAP->unpacker(6);
    mMilliseconds = mCAP->unpacker(10);
    mMicroseconds = mCAP->unpacker(10);
    mNanoseconds = mCAP->unpacker(10);
    mCAP->unpacker(6); // Unused
    mCAP->unpackerString(mText0);
    mCAP->flushPacket();
    CA_ASSERT((mEnableMask <= 0x3F) && (mHours <= 999) && (mMinutes <= 59) && (mSeconds <=59) &&
                (mMilliseconds <= 999) && (mMicroseconds <= 999) && (mNanoseconds <= 999) && (mModAttribute <= 2),
                "Error in CAPacketTimeBox::set()");
}

uint8 CAPacketTimeBox::pack() {
    uint8 unused;
    uint8 len = mText0.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 10 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_TIME_BOX, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mModAttribute, 8);
    mCAP->packer(mEnableMask, 6);
    mCAP->packer(mHours, 10);
    mCAP->packer(mMinutes, 6);
    mCAP->packer(mSeconds, 6);
    mCAP->packer(mMilliseconds, 10);
    mCAP->packer(mMicroseconds, 10);
    mCAP->packer(mNanoseconds, 10);
    mCAP->packer(unused, 6);
    mCAP->packerString(mText0.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// ScriptEnd Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketScriptEnd::CAPacketScriptEnd(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketScriptEnd::set() {
    CA_ASSERT(0, "ScriptEnd::set never needs to be called");
}

void CAPacketScriptEnd::unpack() {
    CA_ASSERT(0, "ScriptEnd::unpack never needs to be called");
}

uint8 CAPacketScriptEnd::pack() {
    uint8 packetSize = 3;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_SCRIPT_END, 8);
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// MenuSelect Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuSelect::CAPacketMenuSelect(CAPacket& caPacket) {
    mMode = 0;
    mMenuNumber = 0;
    mCAP = &caPacket;
}

void CAPacketMenuSelect::set(uint8 mode, uint8 menuNumber) {
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

uint8 CAPacketMenuSelect::pack() {
    uint8 packetSize = 3 + 2;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MENU_SELECT, 8);
    mCAP->packer(mMode, 8);
    mCAP->packer(mMenuNumber, 8);
    mCAP->flushPacket();
    return packetSize;
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

void CAPacketMenuList::set(uint8 menuId, uint8 moduleId0, uint8 moduleMask0,  uint8 moduleId1,
                uint8 moduleMask1, uint8 moduleId2, uint8 moduleMask2, uint8 moduleId3, uint8 moduleMask3,
                uint8 moduleTypeId0, uint8 moduleTypeMask0, uint8 moduleTypeId1, uint8 moduleTypeMask1,
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

uint8 CAPacketMenuList::pack() {
    int len = mMenuName.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 10 + len;
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

///////////////////////////////////////////////////////////////////////////////
// ModuleList Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketModuleList::CAPacketModuleList(CAPacket& caPacket) {
    mModuleId = 0;
    mModuleTypeId = 0;
    mCAP = &caPacket;
}

void CAPacketModuleList::set(uint8 moduleId, uint8 moduleTypeId, String moduleName) {
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

uint8 CAPacketModuleList::pack() {
    int len = mModuleName.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + 2 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_MODULE_LIST, 8);
    mCAP->packer(mModuleId, 8);
    mCAP->packer(mModuleTypeId, 8);
    mCAP->packerString(mModuleName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// Logger Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketLogger::CAPacketLogger(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketLogger::set(String log) {
    mLog = log;
}

void CAPacketLogger::unpack() {
    mCAP->unpackerString(mLog);
    mCAP->flushPacket();
}

uint8 CAPacketLogger::pack() {
    uint8 len = mLog.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 3 + len;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_LOGGER, 8);
    mCAP->packerString(mLog.c_str());
    mCAP->flushPacket();
    return packetSize;
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

void CAPacketCamState::set(uint8 multiplier, uint8 focus, uint8 shutter) {
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

uint8 CAPacketCamState::pack() {
    uint8 packetSize = 3 + 3;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CAM_STATE, 8);
    mCAP->packer(mMultiplier, 8);
    mCAP->packer(mFocus, 8);
    mCAP->packer(mShutter, 8);
    mCAP->flushPacket();
    return packetSize;
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

uint8 CAPacketCamSettings::pack() {
    uint8 unused;
    uint8 packetSize = 3 + 17;
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

void CAPacketIntervalometer::set(uint16 startHours, uint8 startMinutes, uint8 startSeconds, uint16 startMilliseconds,
                uint16 startMicroseconds, uint16 intervalHours, uint8 intervalMinutes,
                uint8 intervalSeconds, uint16 intervalMilliseconds, uint16 intervalMicroseconds,
                uint16 repeats) {
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

uint8 CAPacketIntervalometer::pack() {
    uint8 unused;
    uint8 packetSize = 3 + 13;
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

///////////////////////////////////////////////////////////////////////////////
// ControlFlags Packet Class
///////////////////////////////////////////////////////////////////////////////
CAPacketControlFlags::CAPacketControlFlags(CAPacket& caPacket) {
    mSlaveModeEnable = 0;
    mExtraMessagesEnable = 0;
    mCAP = &caPacket;
}

void CAPacketControlFlags::set(uint8 slaveModeEnable, uint8 extraMessagesEnable) {
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

uint8 CAPacketControlFlags::pack() {
    uint8 unused = 0;
    uint8 packetSize = 3 + 1;
    mCAP->packer(packetSize, 16);
    mCAP->packer(PID_CONTROL_FLAGS, 8);
    mCAP->packer(mSlaveModeEnable, 1);
    mCAP->packer(mExtraMessagesEnable, 1);
    mCAP->packer(unused, 6);
    mCAP->flushPacket();
    return packetSize;
}

