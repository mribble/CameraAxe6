#include <CAPacket.h>

const uint8* CAPacket::getPacketSize(const uint8* inBuf, uint8 *packetSize)
{
    *packetSize = inBuf[0];
    return inBuf+1;
}

const uint8* CAPacket::getPacketType(const uint8* inBuf, uint8 *packetType)
{
    CA_ASSERT( inBuf[0] > PID_START_SENTINEL  &&
               inBuf[0] < PID_END_SENTINEL,   "Invalid packet type");
    *packetType = inBuf[0];
    return inBuf+1;
}

///////////////////////////////////////////////////////////////////////////////
// MenuHeader Packet class
///////////////////////////////////////////////////////////////////////////////
const uint8* CAPacketMenuHeader::unpack(const uint8* src, char* strBuf) {
    mMajorVersion = unpacker(&src, 8);
    mMinorVersion = unpacker(&src, 8);
    mMenuName = unpackerString(&src, strBuf);
    return src;
}

uint8* CAPacketMenuHeader::pack(uint8* dst) {
    uint8 len = strlen(mMenuName);
    uint8 packetSize = 2 + len + 1;  // 1 for the null terminator
    packer(packetSize, &dst, 8);
    packer(PID_MENU_HEADER, &dst, 8);
    packer(mMajorVersion, &dst, 8);
    packer(mMinorVersion, &dst, 8);
    packerString(mMenuName, &dst, len);
    return dst;
}

void CAPacketMenuHeader::load(uint8 majorVersion, uint8 minorVersion, char* menuName) {
    mMajorVersion = majorVersion;
    mMinorVersion = minorVersion;
    mMenuName = menuName;
}

///////////////////////////////////////////////////////////////////////////////
// Unpack
///////////////////////////////////////////////////////////////////////////////

const uint8* CAPacket::unpackMenuHeader(const uint8* inBuf, PacketMenuHeader* oPacket, char* oBuf)
{
    oPacket->major_version    = (inBuf++)[0];
    oPacket->minor_version    = (inBuf++)[0];
    oPacket->menu_string      = oBuf;
    do
    {
        *(oBuf++) = (inBuf++)[0];
    } while (oBuf[-1] != 0);
    return inBuf;
}

const uint8* CAPacket::unpackNewCell(const uint8* inBuf, PacketNewCell* oPacket)
{
    oPacket->column_percentage = (inBuf++)[0];
    return inBuf;
}

const uint8* CAPacket::unpackCondStart(const uint8* inBuf, PacketCondStart* oPacket)
{
    oPacket->client_host_id  = (inBuf++)[0];
    oPacket->mod_attribute   = unpacker(&inBuf, 4);
    oPacket->value           = unpacker(&inBuf, 4);
    return inBuf;
}

const uint8* CAPacket::unpackTextStatic(const uint8* inBuf, PacketTextStatic* oPacket, char* oBuf)
{
    oPacket->text_string = oBuf;
    do
    {
        *(oBuf++) = (inBuf++)[0];
    } while (oBuf[-1] != 0);
    return inBuf;
}

const uint8* CAPacket::unpackTextDynamic(const uint8* inBuf, PacketTextDynamic* oPacket, char* oBuf)
{
    oPacket->client_host_id    = (inBuf++)[0];
    oPacket->text_string       = oBuf;
    do
    {
        *(oBuf++) = (inBuf++)[0];
    } while (oBuf[-1] != 0);
    return inBuf;
}

const uint8* CAPacket::unpackButton(const uint8* inBuf, PacketButton* oPacket)
{
    oPacket->client_host_id   = (inBuf++)[0];
    oPacket->type             = unpacker(&inBuf, 4);
    oPacket->value            = unpacker(&inBuf, 4);
    return inBuf;
}

const uint8* CAPacket::unpackCheckBox(const uint8* inBuf, PacketCheckBox* oPacket)
{
    oPacket->client_host_id   = (inBuf++)[0];
    oPacket->value            = (inBuf++)[0];
    return inBuf;
}

const uint8* CAPacket::unpackDropSelect(const uint8* inBuf, PacketDropSelect* oPacket, char* oBuf)
{
    oPacket->client_host_id    = (inBuf++)[0];
    oPacket->value             = (inBuf++)[0];
    oPacket->drop_value_string = oBuf;
    do
    {
        *(oBuf++) = (inBuf++)[0];
    } while (oBuf[-1] != 0);
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
///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

uint32 CAPacket::unpacker(const uint8** src, uint8 unpackBits)
{
    uint8 unpackBitsLeft = unpackBits;
    uint32 ret = 0;
    uint8 valShift = 0;

    do
    {
        uint8 bitsInCurSrcByte = 8 - mBitsUsed;
        uint8 bitsToUnpack = min(unpackBitsLeft, bitsInCurSrcByte);
        uint8 unusedLeftBits = (unpackBitsLeft >= bitsInCurSrcByte) ? 
                                0 : (bitsInCurSrcByte - unpackBitsLeft);
        uint8 rightShift = mBitsUsed + unusedLeftBits;
        uint8 val = (**src) << unusedLeftBits;  // Zero out left bits
        val = val >> rightShift;      // Shift bits to right most position for this byte
        ret |= (uint32(val) << valShift);
        valShift += bitsToUnpack;
        if (mBitsUsed + bitsToUnpack == 8)
        {
            mBitsUsed = 0;
            (*src)++;
        }
        else
        {
            mBitsUsed += bitsToUnpack;
        }
        unpackBitsLeft -= bitsToUnpack;
        //CAU::log("%d %d %d %d %d %d %d\n", bitsInCurSrcByte, bitsToUnpack, unusedLeftBits, rightShift, val, ret, unpackBitsLeft);
    } while (unpackBitsLeft != 0);
    return ret;
}

char* CAPacket::unpackerString(const uint8** src, char* dst) {
    uint8 len = strlen((char*)*src);
    strcpy(dst, (char*)*src);
    *src = *src+len;
    return dst;
}

void CAPacket::packer(uint32 val, uint8** dst, uint8 packBits)
{
    uint8 packBitsLeft = packBits;
    uint8 wVal = 0;

    do
    {
        uint8 bitsInCurDstByte = 8 - mBitsUsed;
        uint8 bitsToPack = min(bitsInCurDstByte, packBitsLeft);
        mBitsVal |= (val << mBitsUsed);
        mBitsUsed += bitsToPack;
        val = val >> bitsToPack;
        if (mBitsUsed == 8) // When byte is full write it's value
        {
            **dst = mBitsVal;
            mBitsVal = 0;
            mBitsUsed = 0;
            (*dst)++;
        }
        packBitsLeft -= bitsToPack;
    } while (packBitsLeft != 0);

}

void CAPacket::packerString(const char* src, uint8** dst, uint8 packBytes){
    strcpy((char*)*dst, src);
    *dst = *dst+packBytes;
}

