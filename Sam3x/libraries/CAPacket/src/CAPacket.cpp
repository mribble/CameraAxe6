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

uint8 CAPacket::unpackSize() {
    return (uint8)unpacker(8);
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
    uint8 len = strlen((char*)&mBuf[mBytesUsed]);
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
    CA_ASSERT((mBitsUsed == 0) && (mBitsVal == 0) && (mBytesUsed < mBufSize),
                "Error in flushPacket");
}

///////////////////////////////////////////////////////////////////////////////
// MenuHeader Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketMenuHeader::CAPacketMenuHeader(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketMenuHeader::set(uint8 majorVersion, uint8 minorVersion, String menuName) {
    mMajorVersion = majorVersion;
    mMinorVersion = minorVersion;
    mMenuName = menuName;
}

void CAPacketMenuHeader::unpack() {
    mMajorVersion = mCAP->unpacker(8);
    mMinorVersion = mCAP->unpacker(8);
    mCAP->unpackerString(mMenuName);
    mCAP->flushPacket();
}

uint8 CAPacketMenuHeader::pack() {
    uint8 len = mMenuName.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 2 + 2 + len;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_MENU_HEADER, 8);
    mCAP->packer(mMajorVersion, 8);
    mCAP->packer(mMinorVersion, 8);
    mCAP->packerString(mMenuName.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// NewRow Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketNewRow::CAPacketNewRow(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketNewRow::set() {
    CA_ASSERT(0, "NewRow::set never needs to be called");
}

void CAPacketNewRow::unpack() {
    CA_ASSERT(0, "NewRow::unpack never needs to be called");
}

uint8 CAPacketNewRow::pack() {
    uint8 packetSize = 2;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_NEW_ROW, 8);
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// NewCell Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketNewCell::CAPacketNewCell(CAPacket& caPacket) {
    mColumnPercentage = 0;
    mJustification = 0;
}

void CAPacketNewCell::set(uint8 columnPercentage, uint8 justification) {
    CA_ASSERT((columnPercentage <= 100) && (justification <= 2),
                "Error in CAPacketNewCell::set()");
    mColumnPercentage = columnPercentage;
    mJustification = justification;
}

void CAPacketNewCell::unpack() {
    mColumnPercentage = mCAP->unpacker(8);
    mJustification = mCAP->unpacker(8);
    mCAP->flushPacket();
}

uint8 CAPacketNewCell::pack() {
    uint8 packetSize = 2 + 2;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_NEW_CELL, 8);
    mCAP->packer(mColumnPercentage, 8);
    mCAP->packer(mJustification, 8);
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// TextStatic Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketTextStatic::CAPacketTextStatic(CAPacket& caPacket) {
    mCAP = &caPacket;
}

void CAPacketTextStatic::set(String text) {
    mText = text;
}

void CAPacketTextStatic::unpack() {
    mCAP->unpackerString(mText);
    mCAP->flushPacket();
}

uint8 CAPacketTextStatic::pack() {
    uint8 len = mText.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 2 + len;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_TEXT_STATIC, 8);
    mCAP->packerString(mText.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// TextDynamic Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketTextDynamic::CAPacketTextDynamic(CAPacket& caPacket) {
    mClientHostId = 0;
    mCAP = &caPacket;
}

void CAPacketTextDynamic::set(uint8 clientHostId, String text) {
    mClientHostId = clientHostId;
    mText = text;
}

void CAPacketTextDynamic::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mCAP->unpackerString(mText);
    mCAP->flushPacket();
}

uint8 CAPacketTextDynamic::pack() {
    uint8 len = mText.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 2 + 1 + len;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_TEXT_DYNAMIC, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packerString(mText.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// Button Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketButton::CAPacketButton(CAPacket& caPacket) {
    mClientHostId = 0;
    mType = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketButton::set(uint8 clientHostId, uint8 type, uint8 value, String text) {
    mClientHostId = clientHostId;
    mType = type;
    mValue = value;
    mText = text;
}

void CAPacketButton::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mType = mCAP->unpacker(4);
    mValue = mCAP->unpacker(4);
    mCAP->unpackerString(mText);
    mCAP->flushPacket();
}

uint8 CAPacketButton::pack() {
    uint8 len = mText.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 2 + 2 + len;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_BUTTON, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mType, 4);
    mCAP->packer(mValue, 4);
    mCAP->packerString(mText.c_str());
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// Check Box Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketCheckBox::CAPacketCheckBox(CAPacket& caPacket) {
    mClientHostId = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketCheckBox::set(uint8 clientHostId, uint8 value) {
    mClientHostId = clientHostId;
    mValue = value;
}

void CAPacketCheckBox::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mValue = mCAP->unpacker(8);
    mCAP->flushPacket();
}

uint8 CAPacketCheckBox::pack() {
    uint8 packetSize = 2 + 2;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_CHECK_BOX, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mValue, 8);
    mCAP->flushPacket();
    return packetSize;
}

///////////////////////////////////////////////////////////////////////////////
// Button Packet class
///////////////////////////////////////////////////////////////////////////////
CAPacketDropSelect::CAPacketDropSelect(CAPacket& caPacket) {
    mClientHostId = 0;
    mValue = 0;
    mCAP = &caPacket;
}

void CAPacketDropSelect::set(uint8 clientHostId, uint8 value, String text) {
    mClientHostId = clientHostId;
    mValue = value;
    mText = text;
}

void CAPacketDropSelect::unpack() {
    mClientHostId = mCAP->unpacker(8);
    mValue = mCAP->unpacker(8);
    mCAP->unpackerString(mText);
    mCAP->flushPacket();
}

uint8 CAPacketDropSelect::pack() {
    uint8 len = mText.length() + 1;  // 1 for the null terminator
    uint8 packetSize = 2 + 2 + len;
    mCAP->packer(packetSize, 8);
    mCAP->packer(PID_DROP_SELECT, 8);
    mCAP->packer(mClientHostId, 8);
    mCAP->packer(mValue, 8);
    mCAP->packerString(mText.c_str());
    mCAP->flushPacket();
    return packetSize;
}

/*

///////////////////////////////////////////////////////////////////////////////
// Unpack
///////////////////////////////////////////////////////////////////////////////

const uint8* CAPacket::unpackCondStart(const uint8* inBuf, PacketCondStart* oPacket)
{
    oPacket->client_host_id  = (inBuf++)[0];
    oPacket->mod_attribute   = unpacker(&inBuf, 4);
    oPacket->value           = unpacker(&inBuf, 4);
    return inBuf;
}

const uint8* CAPacket::unpackEditNumber(const uint8* inBuf, PacketEditNumber* oPacket)
{
    oPacket->client_host_id        = (inBuf++)[0];
    oPacket->digits_before_decimal = unpacker(&inBuf, 4);
    oPacket->digits_after_decimal  = unpacker(&inBuf, 4);
    oPacket->min_value             = unpacker(&inBuf, 32);
    oPacket->max_value             = unpacker(&inBuf, 32);
    oPacket->value                 = unpacker(&inBuf, 32);
    return inBuf;
}

const uint8* CAPacket::unpackTimeBox(const uint8* inBuf, PacketTimeBox* oPacket)
{
    uint8 unused;
    
    oPacket->client_host_id       = (inBuf++)[0];
    oPacket->enable_hours         = unpacker(&inBuf, 1);
    oPacket->enable_minutes       = unpacker(&inBuf, 1);
    oPacket->enable_seconds       = unpacker(&inBuf, 1);
    oPacket->enable_milliseconds  = unpacker(&inBuf, 1);
    oPacket->enable_microseconds  = unpacker(&inBuf, 1);
    oPacket->enable_nanoseconds   = unpacker(&inBuf, 1);
    oPacket->hours                = unpacker(&inBuf, 10);
    oPacket->minutes              = unpacker(&inBuf, 6);
    oPacket->seconds              = unpacker(&inBuf, 6);
    oPacket->milliseconds         = unpacker(&inBuf, 10);
    oPacket->microseconds         = unpacker(&inBuf, 10);
    oPacket->nanoseconds          = unpacker(&inBuf, 10);
    unused                        = unpacker(&inBuf, 6);
    return inBuf;
}

const uint8* CAPacket::unpackActivate(const uint8* inBuf, PacketActivate* oPacket)
{
    oPacket->active = (inBuf++)[0];
    return inBuf;
}

const uint8* CAPacket::unpackLog(const uint8* inBuf, PacketLog* oPacket, char* oBuf)
{
    oPacket->log_string = oBuf;
    do
    {
        *(oBuf++) = (inBuf++)[0];
    } while (oBuf[-1] != 0);
    return inBuf;
}

const uint8* CAPacket::unpackCamState(const uint8* inBuf, PacketCamState* oPacket)
{
    oPacket->cam_multiplier = (inBuf++)[0];
    oPacket->cam0_focus     = unpacker(&inBuf, 1);
    oPacket->cam0_shutter   = unpacker(&inBuf, 1);
    oPacket->cam1_focus     = unpacker(&inBuf, 1);
    oPacket->cam1_shutter   = unpacker(&inBuf, 1);
    oPacket->cam2_focus     = unpacker(&inBuf, 1);
    oPacket->cam2_shutter   = unpacker(&inBuf, 1);
    oPacket->cam3_focus     = unpacker(&inBuf, 1);
    oPacket->cam3_shutter   = unpacker(&inBuf, 1);
    oPacket->cam4_focus     = unpacker(&inBuf, 1);
    oPacket->cam4_shutter   = unpacker(&inBuf, 1);
    oPacket->cam5_focus     = unpacker(&inBuf, 1);
    oPacket->cam5_shutter   = unpacker(&inBuf, 1);
    oPacket->cam6_focus     = unpacker(&inBuf, 1);
    oPacket->cam6_shutter   = unpacker(&inBuf, 1);
    oPacket->cam7_focus     = unpacker(&inBuf, 1);
    oPacket->cam7_shutter   = unpacker(&inBuf, 1);
    return inBuf;
}

const uint8* CAPacket::unpackCamSettings(const uint8* inBuf, PacketCamSettings* oPacket)
{
    uint8 unused;

    oPacket->cam_port_number             = unpacker(&inBuf, 5);
    oPacket->mode                        = unpacker(&inBuf, 2);
    oPacket->delay_hours                 = unpacker(&inBuf, 10);
    oPacket->delay_minutes               = unpacker(&inBuf, 6);
    oPacket->delay_seconds               = unpacker(&inBuf, 6);
    oPacket->delay_milliseconds          = unpacker(&inBuf, 10);
    oPacket->delay_microseconds          = unpacker(&inBuf, 10);
    oPacket->duration_hours              = unpacker(&inBuf, 10);
    oPacket->duration_minutes            = unpacker(&inBuf, 6);
    oPacket->duration_seconds            = unpacker(&inBuf, 6);
    oPacket->duration_milliseconds       = unpacker(&inBuf, 10);
    oPacket->duration_microseconds       = unpacker(&inBuf, 10);
    oPacket->sequencer0                  = unpacker(&inBuf, 1);
    oPacket->sequencer1                  = unpacker(&inBuf, 1);
    oPacket->sequencer2                  = unpacker(&inBuf, 1);
    oPacket->sequencer3                  = unpacker(&inBuf, 1);
    oPacket->sequencer4                  = unpacker(&inBuf, 1);
    oPacket->sequencer5                  = unpacker(&inBuf, 1);
    oPacket->sequencer6                  = unpacker(&inBuf, 1);
    oPacket->sequencer7                  = unpacker(&inBuf, 1);
    oPacket->apply_intervalometer        = unpacker(&inBuf, 1);
    oPacket->smart_preview               = unpacker(&inBuf, 6);
    oPacket->mirror_lockup_enable        = unpacker(&inBuf, 1);
    oPacket->mirror_lockup_minutes       = unpacker(&inBuf, 6);
    oPacket->mirror_lockup_seconds       = unpacker(&inBuf, 6);
    oPacket->mirror_lockup_milliseconds  = unpacker(&inBuf, 10);
    unused                               = unpacker(&inBuf, 7);
    return inBuf;
}

const uint8* CAPacket::unpackIntervalometer(const uint8* inBuf, PacketIntervalometer* oPacket)
{
    uint8 unused;

    oPacket->start_hours           = unpacker(&inBuf, 10);
    oPacket->start_minutes         = unpacker(&inBuf, 6);
    oPacket->start_seconds         = unpacker(&inBuf, 6);
    oPacket->start_milliseconds    = unpacker(&inBuf, 10);
    oPacket->start_microseconds    = unpacker(&inBuf, 10);
    oPacket->interval_hours        = unpacker(&inBuf, 10);
    oPacket->interval_minutes      = unpacker(&inBuf, 6);
    oPacket->interval_seconds      = unpacker(&inBuf, 6);
    oPacket->interval_milliseconds = unpacker(&inBuf, 10);
    oPacket->interval_microseconds = unpacker(&inBuf, 10);
    oPacket->repeats               = unpacker(&inBuf, 16);
    unused                         = unpacker(&inBuf, 4);
    return inBuf;
}

const uint8* CAPacket::unpackInterModuleLogic(const uint8* inBuf, PacketInterModuleLogic* oPacket)
{
    uint8 unused;
    
    oPacket->enable_latch = unpacker(&inBuf, 1);
    oPacket->logic        = unpacker(&inBuf, 4);
    unused                = unpacker(&inBuf, 3);
    return inBuf;
}

const uint8* CAPacket::unpackControlFlags(const uint8* inBuf, PacketControlFlags* oPacket)
{
    uint8 unused;
    
    oPacket->enable_slave_mode      = unpacker(&inBuf, 1);
    oPacket->enable_extra_messages  = unpacker(&inBuf, 1);
    unused                          = unpacker(&inBuf, 6);
    return inBuf;
}

///////////////////////////////////////////////////////////////////////////////
// Pack
///////////////////////////////////////////////////////////////////////////////

uint8* CAPacket::packTextDynamic(PacketTextDynamic* iPacket, uint8* dst)
{
    char* str = iPacket->text_string;
    uint8 packetSize = 3 + strlen(str) + 1;  // 1 for the null terminator
    packer(packetSize, &dst, 8);
    packer(PID_TEXT_DYNAMIC, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packButton(PacketButton* iPacket, uint8* dst)
{
    uint8 packetSize = 4;
    packer(packetSize, &dst, 8);
    packer(PID_BUTTON, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    packer(iPacket->type, &dst, 4);
    packer(iPacket->value, &dst, 4);
    return dst;
}

uint8* CAPacket::packCheckBox(PacketCheckBox* iPacket, uint8* dst)
{
    uint8 packetSize = 4;
    packer(packetSize, &dst, 8);
    packer(PID_CHECK_BOX, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    packer(iPacket->value, &dst, 8);
    return dst;
}

uint8* CAPacket::packDropSelect(PacketDropSelect* iPacket, uint8* dst)
{
    char* str = iPacket->drop_value_string;
    uint8 packetSize = 4 + strlen(str) + 1;  // 1 for the null terminator
    packer(packetSize, &dst, 8);
    packer(PID_DROP_SELECT, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    packer(iPacket->value, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packEditNumber(PacketEditNumber* iPacket, uint8* dst) 
{
    uint8 packetSize = 16;
    packer(packetSize, &dst, 8);
    packer(PID_EDIT_NUMBER, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    packer(iPacket->digits_before_decimal, &dst, 4);
    packer(iPacket->digits_after_decimal, &dst, 4);
    packer(iPacket->min_value, &dst, 32);
    packer(iPacket->max_value, &dst, 32);
    packer(iPacket->value, &dst, 32);
    return dst;
}

uint8* CAPacket::packTimeBox(PacketTimeBox* iPacket, uint8* dst)
{
    uint8 packetSize = 11;
    packer(packetSize, &dst, 8);
    packer(PID_TIME_BOX, &dst, 8);
    packer(iPacket->client_host_id, &dst, 8);
    packer(iPacket->enable_hours, &dst, 1);
    packer(iPacket->enable_minutes, &dst, 1);
    packer(iPacket->enable_seconds, &dst, 1);
    packer(iPacket->enable_milliseconds, &dst, 1);
    packer(iPacket->enable_microseconds, &dst, 1);
    packer(iPacket->enable_nanoseconds, &dst, 1);
    packer(iPacket->hours, &dst, 10);
    packer(iPacket->minutes, &dst, 6);
    packer(iPacket->seconds, &dst, 6);
    packer(iPacket->milliseconds, &dst, 10);
    packer(iPacket->microseconds, &dst, 10);
    packer(iPacket->nanoseconds, &dst, 10);
    packer(0, &dst, 6); // unused
    return dst;
}

uint8* CAPacket::packActivate(PacketActivate* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    packer(packetSize, &dst, 8);
    packer(PID_ACTIVATE, &dst, 8);
    packer(iPacket->active, &dst, 8);
    return dst;
}

uint8* CAPacket::packLog(PacketLog* iPacket, uint8* dst)
{
    char* str = iPacket->log_string;
    uint8 packetSize = 2 + strlen(str) + 1;  // 1 for the null terminator
    packer(packetSize, &dst, 8);
    packer(PID_LOG, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packCamState(PacketCamState* iPacket, uint8* dst)
{
    uint8 packetSize = 5;
    packer(packetSize, &dst, 8);
    packer(PID_CAM_STATE, &dst, 8);
    packer(iPacket->cam_multiplier, &dst, 8);
    packer(iPacket->cam0_focus, &dst, 1);
    packer(iPacket->cam0_shutter, &dst, 1);
    packer(iPacket->cam1_focus, &dst, 1);
    packer(iPacket->cam1_shutter, &dst, 1);
    packer(iPacket->cam2_focus, &dst, 1);
    packer(iPacket->cam2_shutter, &dst, 1);
    packer(iPacket->cam3_focus, &dst, 1);
    packer(iPacket->cam3_shutter, &dst, 1);
    packer(iPacket->cam4_focus, &dst, 1);
    packer(iPacket->cam4_shutter, &dst, 1);
    packer(iPacket->cam5_focus, &dst, 1);
    packer(iPacket->cam5_shutter, &dst, 1);
    packer(iPacket->cam6_focus, &dst, 1);
    packer(iPacket->cam6_shutter, &dst, 1);
    packer(iPacket->cam7_focus, &dst, 1);
    packer(iPacket->cam7_shutter, &dst, 1);
    return dst;
}

uint8* CAPacket::packCamSettings(PacketCamSettings* iPacket, uint8* dst)
{
    uint8 packetSize = 19;
    packer(packetSize, &dst, 8);
    packer(PID_CAM_SETTINGS, &dst, 8);
    packer(iPacket->cam_port_number, &dst, 5);
    packer(iPacket->mode, &dst, 2);
    packer(iPacket->delay_hours, &dst, 10);
    packer(iPacket->delay_minutes, &dst, 6);
    packer(iPacket->delay_seconds, &dst, 6);    
    packer(iPacket->delay_milliseconds, &dst, 10);
    packer(iPacket->delay_microseconds, &dst, 10);
    packer(iPacket->duration_hours, &dst, 10);
    packer(iPacket->duration_minutes, &dst, 6);
    packer(iPacket->duration_seconds, &dst, 6);
    packer(iPacket->duration_milliseconds, &dst, 10);
    packer(iPacket->duration_microseconds, &dst, 10);
    packer(iPacket->sequencer0, &dst, 1);
    packer(iPacket->sequencer1, &dst, 1);
    packer(iPacket->sequencer2, &dst, 1);
    packer(iPacket->sequencer3, &dst, 1);
    packer(iPacket->sequencer4, &dst, 1);
    packer(iPacket->sequencer5, &dst, 1);
    packer(iPacket->sequencer6, &dst, 1);
    packer(iPacket->sequencer7, &dst, 1);
    packer(iPacket->apply_intervalometer, &dst, 1);
    packer(iPacket->smart_preview, &dst, 6);
    packer(iPacket->mirror_lockup_enable, &dst, 1);
    packer(iPacket->mirror_lockup_minutes, &dst, 6);
    packer(iPacket->mirror_lockup_seconds, &dst, 6);
    packer(iPacket->mirror_lockup_milliseconds, &dst, 10);
    packer(0, &dst, 7); // unused
    return dst;
}

uint8* CAPacket::packIntervalometer(PacketIntervalometer* iPacket, uint8* dst)
{
    uint8 packetSize = 15;
    packer(packetSize, &dst, 8);
    packer(PID_INTERVALOMETER, &dst, 8);
    packer(iPacket->start_hours, &dst, 10);
    packer(iPacket->start_minutes, &dst, 6);
    packer(iPacket->start_seconds, &dst, 6);
    packer(iPacket->start_milliseconds, &dst, 10);
    packer(iPacket->start_microseconds, &dst, 10);
    packer(iPacket->interval_hours, &dst, 10);
    packer(iPacket->interval_minutes, &dst, 6);
    packer(iPacket->interval_seconds, &dst, 6);
    packer(iPacket->interval_milliseconds, &dst, 10);
    packer(iPacket->interval_microseconds, &dst, 10);
    packer(iPacket->repeats, &dst, 16);
    packer(0, &dst, 4);  // unused
    return dst;
}

uint8* CAPacket::packInterModuleLogic(PacketInterModuleLogic* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    packer(packetSize, &dst, 8);
    packer(PID_INTER_MODULE_LOGIC, &dst, 8);
    packer(iPacket->enable_latch, &dst, 1);
    packer(iPacket->logic, &dst, 4);
    packer(0, &dst, 3);
    return dst;
}

uint8* CAPacket::packControlFlags(PacketControlFlags* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    packer(packetSize, &dst, 8);
    packer(PID_CONTROL_FLAGS, &dst, 8);
    packer(iPacket->enable_slave_mode, &dst, 1);
    packer(iPacket->enable_extra_messages, &dst, 1);
    packer(0, &dst, 6);
    return dst;
}

*/

