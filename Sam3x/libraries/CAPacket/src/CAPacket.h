#ifndef __CAPACKET_H__
#define __CAPACKET_H__

#include "CATypes.h"

enum packetId  {PID_START_SENTINEL      =  0,  // Assumed to be first
                PID_MENU_HEADER         =  1,
                PID_NEW_ROW             =  2,
                PID_NEW_CELL_LEFT       =  3,
                PID_NEW_CELL_RIGHT      =  4, 
                PID_NEW_CELL_CENTER     =  5,
                PID_COND_START          =  6,
                PID_COND_END            =  7,
                PID_TEXT_STATIC         =  8,
                PID_TEXT_DYNAMIC        =  9,
                PID_BUTTON              = 10,
                PID_CHECK_BOX           = 11,
                PID_DROP_SELECT         = 12,
                PID_EDIT_NUMBER         = 13,
                PID_TIME_BOX            = 14,
                PID_SCRIPT_END          = 15,
                PID_ACTIVATE            = 16,
                PID_LOG                 = 17,
                PID_CAM_STATE           = 18,
                PID_CAM_SETTINGS        = 19,
                PID_INTERVALOMETER      = 20,
                PID_INTER_MODULE_LOGIC  = 21,
                PID_CONTROL_FLAGS       = 22,
                PID_END_SENTINEL        = 24, // Assumed to be last
               };

enum packetState { STATE_PACKER=1, STATE_UNPACKER=2 };

class CAPacket
{
public:
    CAPacket(uint8 state, uint8 *buf, uint16 bufSize);
    uint8 unpackSize();
    uint8 unpackType();
    uint32 unpacker(uint8 unpackBits);
    void unpackerString(String& str);
    void packer(uint32 val, uint8 packBits);
    void packerString(const char* src);
    void flushPacket();

private:    
    uint8 mBitsUsed;
    uint8 mBitsVal;
    uint16 mBytesUsed;
    uint8 mState;
    uint8* mBuf;
    uint16 mBufSize;
};

class CAPacketMenuHeader {
public:
    CAPacketMenuHeader(CAPacket& caPacket);
    uint8 getMajorVersion() {return mMajorVersion;};
    uint8 getMinorVersion() {return mMinorVersion;};
    String getMenuName() { return mMenuName;};

    void set(uint8 majorVersion, uint8 minorVersion, String menuName);
    void unpack();
    uint8 pack();

private:
    CAPacket* mCAP;
    uint8 mMajorVersion;
    uint8 mMinorVersion;
    String mMenuName;
};

/*
typedef struct {
    uint8 major_version;
    uint8 minor_version;
    char* menu_string;
} PacketMenuHeader;

typedef struct {
    uint8 column_percentage;
} PacketNewCell;

typedef struct {
    uint8 client_host_id;
    uint8 mod_attribute    : 4;
    uint8 value            : 4;
} PacketCondStart;

typedef struct {
    char* text_string;
} PacketTextStatic;

typedef struct {
    uint8 client_host_id;    
    char* text_string;
} PacketTextDynamic;

typedef struct {
    uint8 client_host_id;
    uint8 type   : 4;
    uint8 value  : 4;
} PacketButton;

typedef struct {
    uint8 client_host_id;
    uint8 value;
} PacketCheckBox;

typedef struct {
    uint8 client_host_id;
    uint8 value;
    char* drop_value_string;
} PacketDropSelect;

typedef struct {
    uint8 client_host_id;
    uint8 digits_before_decimal : 4;
    uint8 digits_after_decimal  : 4;
    uint32 min_value;
    uint32 max_value;
    uint32 value;
} PacketEditNumber;

typedef struct {
    uint8 client_host_id;
    uint8 enable_hours         : 1;
    uint8 enable_minutes       : 1;
    uint8 enable_seconds       : 1;
    uint8 enable_milliseconds  : 1;
    uint8 enable_microseconds  : 1;
    uint8 enable_nanoseconds   : 1;
    uint8 unused0              : 2;
    uint16 hours               : 10;
    uint16 minutes             : 6;
    uint16 seconds             : 6;
    uint16 milliseconds        : 10;
    uint16 microseconds;       // 10
    uint16 nanoseconds;        // 10
} PacketTimeBox;

typedef struct {
    uint8 active;
} PacketActivate;

typedef struct {
    char* log_string;
} PacketLog;

typedef struct {
    uint8 cam_multiplier;
    uint8 cam0_focus    : 1;
    uint8 cam0_shutter  : 1;
    uint8 cam1_focus    : 1;
    uint8 cam1_shutter  : 1;
    uint8 cam2_focus    : 1;
    uint8 cam2_shutter  : 1;
    uint8 cam3_focus    : 1;
    uint8 cam3_shutter  : 1;
    uint8 cam4_focus    : 1;
    uint8 cam4_shutter  : 1;
    uint8 cam5_focus    : 1;
    uint8 cam5_shutter  : 1;
    uint8 cam6_focus    : 1;
    uint8 cam6_shutter  : 1;
    uint8 cam7_focus    : 1;
    uint8 cam7_shutter  : 1;
} PacketCamState;

typedef struct {
    uint32 cam_port_number   : 5;
    uint32 mode              : 2;
    uint32 delay_hours       : 10;
    uint32 delay_minutes     : 6;
    uint32 delay_seconds     : 6;
    uint32 unused0           : 3;
    uint32 delay_milliseconds : 10;
    uint32 delay_microseconds : 10;
    uint32 duration_hours     : 10;
    uint32 unused1            : 2;
    uint32 duration_minutes      : 6;
    uint32 duration_seconds      : 6;
    uint32 duration_milliseconds : 10;
    uint32 duration_microseconds : 10;
    uint32 sequencer0             : 1;
    uint32 sequencer1             : 1;
    uint32 sequencer2             : 1;
    uint32 sequencer3             : 1;
    uint32 sequencer4             : 1;
    uint32 sequencer5             : 1;
    uint32 sequencer6             : 1;
    uint32 sequencer7             : 1;
    uint32 unused2                : 4;
    uint32 apply_intervalometer   : 1;
    uint32 smart_preview          : 6;
    uint32 mirror_lockup_enable   : 1;
    uint32 mirror_lockup_minutes  : 6;
    uint32 mirror_lockup_seconds  : 6;
    uint16 mirror_lockup_milliseconds; //10
} PacketCamSettings;

typedef struct {
    uint16 start_hours             : 10;
    uint16 start_minutes           : 6;
    uint16 start_seconds           : 6;
    uint16 start_milliseconds      : 10;
    uint16 start_microseconds;     // 10
    uint16 interval_hours          : 10;
    uint16 interval_minutes        : 6;
    uint16 interval_seconds        : 6;
    uint16 interval_milliseconds   : 10;
    uint16 interval_microseconds;  // 10
    uint16 repeats;
} PacketIntervalometer;

enum interModuleLogic {IML_OR, IML_AND, IML_OR_AND_OR, IML_AND_OR_AND};
typedef struct {
    uint8  enable_latch : 1;
    uint8  logic        : 4;
    uint8  unused       : 3;
} PacketInterModuleLogic;

typedef struct {
    uint8 enable_slave_mode     : 1;
    uint8 enable_extra_messages : 1;
    uint8 unused                : 6;
} PacketControlFlags;
*/

#endif // __CAPACKET_H__
