#include <CAPacket.h>

const uint8* CAPacket::getPacketSize(const uint8* inBuf, uint8 *packetSize)
{
    *packetSize = inBuf[0];
    return inBuf+1;
}

const uint8* CAPacket::getPacketType(const uint8* inBuf, uint8 *packetType)
{
    CA_ASSERT( inBuf[0] == PID_MENU_HEADER        ||
               inBuf[0] == PID_NEW_ROW            ||
               inBuf[0] == PID_NEW_CELL_LEFT      ||
               inBuf[0] == PID_NEW_CELL_RIGHT     ||
               inBuf[0] == PID_NEW_CELL_CENTER    ||
               inBuf[0] == PID_COND_START         ||
               inBuf[0] == PID_COND_END           ||
               inBuf[0] == PID_TEXT_STATIC        ||
               inBuf[0] == PID_TEXT_DYNAMIC       ||
               inBuf[0] == PID_BUTTON             ||
               inBuf[0] == PID_CHECK_BOX          ||
               inBuf[0] == PID_DROP_SELECT        ||
               inBuf[0] == PID_EDIT_NUMBER        ||
               inBuf[0] == PID_TIME_BOX           ||
               inBuf[0] == PID_SCRIPT_END         ||
               inBuf[0] == PID_ACTIVATE           ||
               inBuf[0] == PID_LOG                ||
               inBuf[0] == PID_CAM_STATE          ||
               inBuf[0] == PID_CAM_SETTINGS       ||
               inBuf[0] == PID_INTERVALOMETER     ||
               inBuf[0] == PID_INTER_MODULE_LOGIC ||
               inBuf[0] == PID_CONTROL_FLAGS      ||
               inBuf[0] == PID_RTC       , "Invalid packet type");
    *packetType = inBuf[0];
    return inBuf+1;
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
    oPacket->mod_attribute   = unpack(&inBuf, 4);
    oPacket->value           = unpack(&inBuf, 4);
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
    oPacket->type             = unpack(&inBuf, 4);
    oPacket->value            = unpack(&inBuf, 4);
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
    oPacket->digits_before_decimal = unpack(&inBuf, 4);
    oPacket->digits_after_decimal  = unpack(&inBuf, 4);
    oPacket->min_value             = unpack(&inBuf, 32);
    oPacket->max_value             = unpack(&inBuf, 32);
    oPacket->value                 = unpack(&inBuf, 32);
    return inBuf;
}

const uint8* CAPacket::unpackTimeBox(const uint8* inBuf, PacketTimeBox* oPacket)
{
    uint8 unused;
    
    oPacket->client_host_id       = (inBuf++)[0];
    oPacket->enable_hours         = unpack(&inBuf, 1);
    oPacket->enable_minutes       = unpack(&inBuf, 1);
    oPacket->enable_seconds       = unpack(&inBuf, 1);
    oPacket->enable_milliseconds  = unpack(&inBuf, 1);
    oPacket->enable_microseconds  = unpack(&inBuf, 1);
    oPacket->enable_nanoseconds   = unpack(&inBuf, 1);
    oPacket->hours                = unpack(&inBuf, 10);
    oPacket->minutes              = unpack(&inBuf, 6);
    oPacket->seconds              = unpack(&inBuf, 6);
    oPacket->milliseconds         = unpack(&inBuf, 10);
    oPacket->microseconds         = unpack(&inBuf, 10);
    oPacket->nanoseconds          = unpack(&inBuf, 10);
    unused                        = unpack(&inBuf, 6);
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
    oPacket->cam0_focus     = unpack(&inBuf, 1);
    oPacket->cam0_shutter   = unpack(&inBuf, 1);
    oPacket->cam1_focus     = unpack(&inBuf, 1);
    oPacket->cam1_shutter   = unpack(&inBuf, 1);
    oPacket->cam2_focus     = unpack(&inBuf, 1);
    oPacket->cam2_shutter   = unpack(&inBuf, 1);
    oPacket->cam3_focus     = unpack(&inBuf, 1);
    oPacket->cam3_shutter   = unpack(&inBuf, 1);
    oPacket->cam4_focus     = unpack(&inBuf, 1);
    oPacket->cam4_shutter   = unpack(&inBuf, 1);
    oPacket->cam5_focus     = unpack(&inBuf, 1);
    oPacket->cam5_shutter   = unpack(&inBuf, 1);
    oPacket->cam6_focus     = unpack(&inBuf, 1);
    oPacket->cam6_shutter   = unpack(&inBuf, 1);
    oPacket->cam7_focus     = unpack(&inBuf, 1);
    oPacket->cam7_shutter   = unpack(&inBuf, 1);
    return inBuf;
}

const uint8* CAPacket::unpackCamSettings(const uint8* inBuf, PacketCamSettings* oPacket)
{
    uint8 unused;

    oPacket->cam_port_number             = unpack(&inBuf, 5);
    oPacket->mode                        = unpack(&inBuf, 2);
    oPacket->delay_hours                 = unpack(&inBuf, 10);
    oPacket->delay_minutes               = unpack(&inBuf, 6);
    oPacket->delay_seconds               = unpack(&inBuf, 6);
    oPacket->delay_milliseconds          = unpack(&inBuf, 10);
    oPacket->delay_microseconds          = unpack(&inBuf, 10);
    oPacket->duration_hours              = unpack(&inBuf, 10);
    oPacket->duration_minutes            = unpack(&inBuf, 6);
    oPacket->duration_seconds            = unpack(&inBuf, 6);
    oPacket->duration_milliseconds       = unpack(&inBuf, 10);
    oPacket->duration_microseconds       = unpack(&inBuf, 10);
    oPacket->sequencer0                  = unpack(&inBuf, 1);
    oPacket->sequencer1                  = unpack(&inBuf, 1);
    oPacket->sequencer2                  = unpack(&inBuf, 1);
    oPacket->sequencer3                  = unpack(&inBuf, 1);
    oPacket->sequencer4                  = unpack(&inBuf, 1);
    oPacket->sequencer5                  = unpack(&inBuf, 1);
    oPacket->sequencer6                  = unpack(&inBuf, 1);
    oPacket->sequencer7                  = unpack(&inBuf, 1);
    oPacket->apply_intervalometer        = unpack(&inBuf, 1);
    oPacket->smart_preview               = unpack(&inBuf, 6);
    oPacket->mirror_lockup_enable        = unpack(&inBuf, 1);
    oPacket->mirror_lockup_minutes       = unpack(&inBuf, 6);
    oPacket->mirror_lockup_seconds       = unpack(&inBuf, 6);
    oPacket->mirror_lockup_milliseconds  = unpack(&inBuf, 10);
    unused                               = unpack(&inBuf, 7);
    return inBuf;
}

const uint8* CAPacket::unpackIntervalometer(const uint8* inBuf, PacketIntervalometer* oPacket)
{
    uint8 unused;

    oPacket->start_hours           = unpack(&inBuf, 10);
    oPacket->start_minutes         = unpack(&inBuf, 6);
    oPacket->start_seconds         = unpack(&inBuf, 6);
    oPacket->start_milliseconds    = unpack(&inBuf, 10);
    oPacket->start_microseconds    = unpack(&inBuf, 10);
    oPacket->interval_hours        = unpack(&inBuf, 10);
    oPacket->interval_minutes      = unpack(&inBuf, 6);
    oPacket->interval_seconds      = unpack(&inBuf, 6);
    oPacket->interval_milliseconds = unpack(&inBuf, 10);
    oPacket->interval_microseconds = unpack(&inBuf, 10);
    oPacket->repeats               = unpack(&inBuf, 16);
    unused                         = unpack(&inBuf, 4);
    return inBuf;
}

const uint8* CAPacket::unpackInterModuleLogic(const uint8* inBuf, PacketInterModuleLogic* oPacket)
{
    uint8 unused;
    
    oPacket->enable_latch = unpack(&inBuf, 1);
    oPacket->logic        = unpack(&inBuf, 4);
    unused                = unpack(&inBuf, 3);
    return inBuf;
}

const uint8* CAPacket::unpackControlFlags(const uint8* inBuf, PacketControlFlags* oPacket)
{
    uint8 unused;
    
    oPacket->enable_slave_mode      = unpack(&inBuf, 1);
    oPacket->enable_extra_messages  = unpack(&inBuf, 1);
    unused                          = unpack(&inBuf, 6);
    return inBuf;
}

///////////////////////////////////////////////////////////////////////////////
// Pack
///////////////////////////////////////////////////////////////////////////////

uint8* CAPacket::packTextDynamic(PacketTextDynamic* iPacket, uint8* dst)
{
    char* str = iPacket->text_string;
    uint8 packetSize = 3 + strlen(str) + 1;  // 1 for the null terminator
    pack(packetSize, &dst, 8);
    pack(PID_TEXT_DYNAMIC, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packButton(PacketButton* iPacket, uint8* dst)
{
    uint8 packetSize = 4;
    pack(packetSize, &dst, 8);
    pack(PID_BUTTON, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    pack(iPacket->type, &dst, 4);
    pack(iPacket->value, &dst, 4);
    return dst;
}

uint8* CAPacket::packCheckBox(PacketCheckBox* iPacket, uint8* dst)
{
    uint8 packetSize = 4;
    pack(packetSize, &dst, 8);
    pack(PID_CHECK_BOX, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    pack(iPacket->value, &dst, 8);
    return dst;
}

uint8* CAPacket::packDropSelect(PacketDropSelect* iPacket, uint8* dst)
{
    char* str = iPacket->drop_value_string;
    uint8 packetSize = 4 + strlen(str) + 1;  // 1 for the null terminator
    pack(packetSize, &dst, 8);
    pack(PID_DROP_SELECT, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    pack(iPacket->value, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packEditNumber(PacketEditNumber* iPacket, uint8* dst) 
{
    uint8 packetSize = 16;
    pack(packetSize, &dst, 8);
    pack(PID_EDIT_NUMBER, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    pack(iPacket->digits_before_decimal, &dst, 4);
    pack(iPacket->digits_after_decimal, &dst, 4);
    pack(iPacket->min_value, &dst, 32);
    pack(iPacket->max_value, &dst, 32);
    pack(iPacket->value, &dst, 32);
    return dst;
}

uint8* CAPacket::packTimeBox(PacketTimeBox* iPacket, uint8* dst)
{
    uint8 packetSize = 11;
    pack(packetSize, &dst, 8);
    pack(PID_TIME_BOX, &dst, 8);
    pack(iPacket->client_host_id, &dst, 8);
    pack(iPacket->enable_hours, &dst, 1);
    pack(iPacket->enable_minutes, &dst, 1);
    pack(iPacket->enable_seconds, &dst, 1);
    pack(iPacket->enable_milliseconds, &dst, 1);
    pack(iPacket->enable_microseconds, &dst, 1);
    pack(iPacket->enable_nanoseconds, &dst, 1);
    pack(iPacket->hours, &dst, 10);
    pack(iPacket->minutes, &dst, 6);
    pack(iPacket->seconds, &dst, 6);
    pack(iPacket->milliseconds, &dst, 10);
    pack(iPacket->microseconds, &dst, 10);
    pack(iPacket->nanoseconds, &dst, 10);
    pack(0, &dst, 6); // unused
    return dst;
}

uint8* CAPacket::packActivate(PacketActivate* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    pack(packetSize, &dst, 8);
    pack(PID_ACTIVATE, &dst, 8);
    pack(iPacket->active, &dst, 8);
    return dst;
}

uint8* CAPacket::packLog(PacketLog* iPacket, uint8* dst)
{
    char* str = iPacket->log_string;
    uint8 packetSize = 2 + strlen(str) + 1;  // 1 for the null terminator
    pack(packetSize, &dst, 8);
    pack(PID_LOG, &dst, 8);
    do
    {
        *(dst++) = *(str++);
    } while (dst[-1] != 0);
    return dst;
}

uint8* CAPacket::packCamState(PacketCamState* iPacket, uint8* dst)
{
    uint8 packetSize = 5;
    pack(packetSize, &dst, 8);
    pack(PID_CAM_STATE, &dst, 8);
    pack(iPacket->cam_multiplier, &dst, 8);
    pack(iPacket->cam0_focus, &dst, 1);
    pack(iPacket->cam0_shutter, &dst, 1);
    pack(iPacket->cam1_focus, &dst, 1);
    pack(iPacket->cam1_shutter, &dst, 1);
    pack(iPacket->cam2_focus, &dst, 1);
    pack(iPacket->cam2_shutter, &dst, 1);
    pack(iPacket->cam3_focus, &dst, 1);
    pack(iPacket->cam3_shutter, &dst, 1);
    pack(iPacket->cam4_focus, &dst, 1);
    pack(iPacket->cam4_shutter, &dst, 1);
    pack(iPacket->cam5_focus, &dst, 1);
    pack(iPacket->cam5_shutter, &dst, 1);
    pack(iPacket->cam6_focus, &dst, 1);
    pack(iPacket->cam6_shutter, &dst, 1);
    pack(iPacket->cam7_focus, &dst, 1);
    pack(iPacket->cam7_shutter, &dst, 1);
    return dst;
}

uint8* CAPacket::packCamSettings(PacketCamSettings* iPacket, uint8* dst)
{
    uint8 packetSize = 19;
    pack(packetSize, &dst, 8);
    pack(PID_CAM_SETTINGS, &dst, 8);
    pack(iPacket->cam_port_number, &dst, 5);
    pack(iPacket->mode, &dst, 2);
    pack(iPacket->delay_hours, &dst, 10);
    pack(iPacket->delay_minutes, &dst, 6);
    pack(iPacket->delay_seconds, &dst, 6);    
    pack(iPacket->delay_milliseconds, &dst, 10);
    pack(iPacket->delay_microseconds, &dst, 10);
    pack(iPacket->duration_hours, &dst, 10);
    pack(iPacket->duration_minutes, &dst, 6);
    pack(iPacket->duration_seconds, &dst, 6);
    pack(iPacket->duration_milliseconds, &dst, 10);
    pack(iPacket->duration_microseconds, &dst, 10);
    pack(iPacket->sequencer0, &dst, 1);
    pack(iPacket->sequencer1, &dst, 1);
    pack(iPacket->sequencer2, &dst, 1);
    pack(iPacket->sequencer3, &dst, 1);
    pack(iPacket->sequencer4, &dst, 1);
    pack(iPacket->sequencer5, &dst, 1);
    pack(iPacket->sequencer6, &dst, 1);
    pack(iPacket->sequencer7, &dst, 1);
    pack(iPacket->apply_intervalometer, &dst, 1);
    pack(iPacket->smart_preview, &dst, 6);
    pack(iPacket->mirror_lockup_enable, &dst, 1);
    pack(iPacket->mirror_lockup_minutes, &dst, 6);
    pack(iPacket->mirror_lockup_seconds, &dst, 6);
    pack(iPacket->mirror_lockup_milliseconds, &dst, 10);
    pack(0, &dst, 7); // unused
    return dst;
}

uint8* CAPacket::packIntervalometer(PacketIntervalometer* iPacket, uint8* dst)
{
    uint8 packetSize = 15;
    pack(packetSize, &dst, 8);
    pack(PID_INTERVALOMETER, &dst, 8);
    pack(iPacket->start_hours, &dst, 10);
    pack(iPacket->start_minutes, &dst, 6);
    pack(iPacket->start_seconds, &dst, 6);
    pack(iPacket->start_milliseconds, &dst, 10);
    pack(iPacket->start_microseconds, &dst, 10);
    pack(iPacket->interval_hours, &dst, 10);
    pack(iPacket->interval_minutes, &dst, 6);
    pack(iPacket->interval_seconds, &dst, 6);
    pack(iPacket->interval_milliseconds, &dst, 10);
    pack(iPacket->interval_microseconds, &dst, 10);
    pack(iPacket->repeats, &dst, 16);
    pack(0, &dst, 4);  // unused
    return dst;
}

uint8* CAPacket::packInterModuleLogic(PacketInterModuleLogic* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    pack(packetSize, &dst, 8);
    pack(PID_INTER_MODULE_LOGIC, &dst, 8);
    pack(iPacket->enable_latch, &dst, 1);
    pack(iPacket->logic, &dst, 4);
    pack(0, &dst, 3);
    return dst;
}

uint8* CAPacket::packControlFlags(PacketControlFlags* iPacket, uint8* dst)
{
    uint8 packetSize = 3;
    pack(packetSize, &dst, 8);
    pack(PID_CONTROL_FLAGS, &dst, 8);
    pack(iPacket->enable_slave_mode, &dst, 1);
    pack(iPacket->enable_extra_messages, &dst, 1);
    pack(0, &dst, 6);
    return dst;
}
///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

uint32 CAPacket::unpack(const uint8** src, uint8 unpackBits)
{
    uint8 unpackBitsLeft = unpackBits;
    uint32 ret = 0;
    uint8 valShift = 0;

    do
    {
        uint8 bitsInCurSrcByte = 8 - m_bitsUsed;
        uint8 bitsToUnpack = min(unpackBitsLeft, bitsInCurSrcByte);
        uint8 unusedLeftBits = (unpackBitsLeft >= bitsInCurSrcByte) ? 
                                0 : (bitsInCurSrcByte - unpackBitsLeft);
        uint8 rightShift = m_bitsUsed + unusedLeftBits;
        uint8 val = (**src) << unusedLeftBits;  // Zero out left bits
        val = val >> rightShift;      // Shift bits to right most position for this byte
        ret |= (uint32(val) << valShift);
        valShift += bitsToUnpack;
        if (m_bitsUsed + bitsToUnpack == 8)
        {
            m_bitsUsed = 0;
            (*src)++;
        }
        else
        {
            m_bitsUsed += bitsToUnpack;
        }
        unpackBitsLeft -= bitsToUnpack;
        //CAU::log("%d %d %d %d %d %d %d\n", bitsInCurSrcByte, bitsToUnpack, unusedLeftBits, rightShift, val, ret, unpackBitsLeft);
    } while (unpackBitsLeft != 0);
    return ret;
}

void CAPacket::pack(uint32 val, uint8** dst, uint8 packBits)
{
    uint8 packBitsLeft = packBits;
    uint8 wVal = 0;
    
    do
    {
        uint8 bitsInCurDstByte = 8 - m_bitsUsed;
        uint8 bitsToPack = min(bitsInCurDstByte, packBitsLeft);
        m_val |= (val << m_bitsUsed);
        m_bitsUsed += bitsToPack;
        val = val >> bitsToPack;
        if (m_bitsUsed == 8) // When byte is full write it's value
        {
            **dst = m_val;
            m_val = 0;
            m_bitsUsed = 0;
            (*dst)++;
        }
        packBitsLeft -= bitsToPack;
    } while (packBitsLeft != 0);
}

