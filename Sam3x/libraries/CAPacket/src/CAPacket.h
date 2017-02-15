#ifndef __CAPACKET_H__
#define __CAPACKET_H__

#include "CATypes.h"
#include "CAPacketBase.h"

enum packetId  {PID_START_SENTINEL      =  0,  // Must be first
                PID_MENU_HEADER         =  1,
                PID_TEXT_STATIC         =  2,
                PID_TEXT_DYNAMIC        =  3,
                PID_BUTTON              =  4,
                PID_CHECK_BOX           =  5,
                PID_DROP_SELECT         =  6,
                PID_EDIT_NUMBER         =  7,
                PID_TIME_BOX            =  8,
                PID_SCRIPT_END          =  9,
                PID_MENU_SELECT         = 10,
                PID_MENU_LIST           = 11,
                PID_MODULE_LIST         = 12,
                PID_LOGGER              = 13,
                PID_CAM_STATE           = 14,
                PID_CAM_SETTINGS        = 15,
                PID_INTERVALOMETER      = 16,
                PID_CONTROL_FLAGS       = 17,
                PID_END_SENTINEL        = 18, // Must be last
               };

enum packetState { STATE_PACKER=1, STATE_UNPACKER=2 };

#define NULL_CLIENT_HOST_ID 0xff

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAPacket - This class handles converting from variables the app uses to a packed byte stream sent over the
//                  network.  This class manages the data buffer for either packing or unpacking modes.  Packing
//                  means converting from variables to the byte buffer.  Unpacking is converting from the byte
//                  buffer to variables.  There is one class per packet type and each of these classes has a 
//                  reference to this CAPacket class.
//
//             unpackSize() and unpackType() are separated from the main unpack, which is implemented in the 
//                  packet type specific classes, because having this the packet size and packet type early is
//                  useful in the code that uses these classes.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAPacket
{
public:
    CAPacket(uint8 state, uint8 *buf, uint16 bufSize);
    void resetBuffer();                                 // Resets the data buffer back to the start
    uint16 unpackSize();                                // Unpacks the size of the packet
    uint8 unpackType();                                 // Unpacks the type of the packet
    uint32 unpacker(uint8 unpackBits);                  // Unpacks 1..32 bits from the byte stream
    void unpackerString(String& str);                   // Unpacks a null terminated string from the byte stream
    void packer(uint32 val, uint8 packBits);            // Packs 1..32 bits into the byte stream
    void packerString(const char* src);                 // Packs a null terminated string into the byte stream
    void flushPacket();                                 // This just does some validation

private:    
    uint8 mBitsUsed;        // Bits of an uncompleted byte (pack or unpack) used in the byte stream
    uint8 mBitsVal;         // Value of the unused bits
    uint16 mBytesUsed;      // Number of bytes used in the pack or unpack stream
    uint8 mState;           // State of the class (prevents invalid use like unpacking from a pack buffer)
    uint8* mBuf;            // The byte stream
    uint16 mBufSize;        // Size of the byte stream
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAPacketElement - This is an interface class for all the specific packet types that follow.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAPacketElement {
public:
    virtual uint8 getPacketType() = 0;
    virtual uint8 getClientHostId() = 0;
    virtual void unpack() = 0;
    virtual uint16 pack() = 0;
protected:
    CAPacket* mCAP;    
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Below are many classes.  One for each packet type.
//   * All these classes have a constructor that attaches a reference of CAPacket to this class.
//   * All these classes have get*() methods to access private data members.
//   * All these classes have a set() method to set all the private data members.
//   * All these classes have an unpack() method to do the unpacking (not including packet size or type)
//   * All these classes have a pack() method to do the packing (including the packet size and type)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


class CAPacketMenuHeader : public CAPacketElement {
public:
    CAPacketMenuHeader(CAPacket& caPacket);
    uint8 getPacketType() {return PID_MENU_HEADER;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint16 getMajorVersion() {return mMajorVersion;};
    uint16 getMinorVersion() {return mMinorVersion;};
    const char* getMenuName() {return mMenuName.c_str();};
    void set(uint16 majorVersion, uint16 minorVersion, String menuName);
    void unpack();
    uint16 pack();
private:
    uint16 mMajorVersion;
    uint16 mMinorVersion;
    String mMenuName;
};

class CAPacketTextStatic : public CAPacketElement {
public:
    CAPacketTextStatic(CAPacket& caPacket);
    uint8 getPacketType() {return PID_TEXT_STATIC;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    const char* getText0() {return mText0.c_str();};
    void set(String text0);
    void unpack();
    uint16 pack();
private:
    String mText0;
};

class CAPacketTextDynamic : public CAPacketElement {
public:
    CAPacketTextDynamic(CAPacket& caPacket);
    uint8 getPacketType() {return PID_TEXT_DYNAMIC;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    const char* getText0() {return mText0.c_str();};
    const char* getText1() {return mText1.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, String text0, String text1);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    String mText0;
    String mText1;
};

class CAPacketButton : public CAPacketElement {
public:
    CAPacketButton(CAPacket& caPacket);
    uint8 getPacketType() {return PID_BUTTON;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    uint8 getType() {return mType;};
    uint8 getValue() {return mValue;};
    const char* getText0() {return mText0.c_str();};
    const char* getText1() {return mText1.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, uint8 type, uint8 value, String text0, String text1);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    uint8 mType;
    uint8 mValue;
    String mText0;
    String mText1;
};

class CAPacketCheckBox : public CAPacketElement {
public:
    CAPacketCheckBox(CAPacket& caPacket);
    uint8 getPacketType() {return PID_CHECK_BOX;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    uint8 getValue() {return mValue;};
    const char* getText0() {return mText0.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, uint8 value, String text0);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    uint8 mValue;
    String mText0;
};

class CAPacketDropSelect : public CAPacketElement {
public:
    CAPacketDropSelect(CAPacket& caPacket);
    uint8 getPacketType() {return PID_DROP_SELECT;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    uint8 getValue() {return mValue;};
    const char* getText0() {return mText0.c_str();};
    const char* getText1() {return mText1.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, uint8 value, String text0, String text1);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    uint8 mValue;
    String mText0;
    String mText1;
};

class CAPacketEditNumber : public CAPacketElement {
public:
    CAPacketEditNumber(CAPacket& caPacket);
    uint8 getPacketType() {return PID_EDIT_NUMBER;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    uint8 getDigitsBeforeDecimal() {return mDigitsBeforeDecimal;};
    uint8 getDigitsAfterDecimal() {return mDigitsAfterDecimal;};
    uint32 getMinValue() {return mMinValue;};
    uint32 getMaxValue() {return mMaxValue;};
    uint32 getValue() {return mValue;};
    const char* getText0() {return mText0.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, uint8 digitsBeforeDecimal, uint8 digitsAfterDecimal, 
                uint32 minValue, uint32 maxValue, uint32 value, String text0);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    uint8 mDigitsBeforeDecimal;
    uint8 mDigitsAfterDecimal;
    uint32 mMinValue;
    uint32 mMaxValue;
    uint32 mValue;
    String mText0;
};

class CAPacketTimeBox : public CAPacketElement {
public:
    const uint8 HOUR_MASK           = 0x01;
    const uint8 MINUTE_MASK         = 0x02;
    const uint8 SECOND_MASK         = 0x04;
    const uint8 MILLISECOND_MASK    = 0x08;
    const uint8 MICROSECOND_MASK    = 0x10;
    const uint8 NANOSECOND_MASK     = 0x20;
    CAPacketTimeBox(CAPacket& caPacket);
    uint8 getPacketType() {return PID_TIME_BOX;};
    uint8 getClientHostId() {return mClientHostId;};
    uint8 getModAttribute() {return mModAttribute;};
    uint8 getEnableMask() {return mEnableMask;};
    uint16 getHours() {return mHours;};
    uint8 getMinutes() {return mMinutes;};
    uint8 getSeconds() {return mSeconds;};
    uint16 getMilliseconds() {return mMilliseconds;};
    uint16 getMicroseconds() {return mMicroseconds;};
    uint16 getNanoseconds() {return mNanoseconds;};
    const char* getText0() {return mText0.c_str();};
    void set(uint8 clientHostId, uint8 modAttribute, uint8 enableMask, uint16 hours, uint8 minutes, uint8 seconds,
                uint16 milliseconds, uint16 microseconds, uint16 nanoseconds, String text0);
    void unpack();
    uint16 pack();
private:
    uint8 mClientHostId;
    uint8 mModAttribute;
    uint8 mEnableMask;
    uint16 mHours;
    uint8 mMinutes;
    uint8 mSeconds;
    uint16 mMilliseconds;
    uint16 mMicroseconds;
    uint16 mNanoseconds;
    String mText0;
};

class CAPacketScriptEnd : public CAPacketElement {
public:
    CAPacketScriptEnd(CAPacket& caPacket);
    uint8 getPacketType() {return PID_SCRIPT_END;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    void set();
    void unpack();
    uint16 pack();
private:
};

class CAPacketMenuSelect : public CAPacketElement {
public:
    CAPacketMenuSelect(CAPacket& caPacket);
    uint8 getPacketType() {return PID_MENU_SELECT;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8 getMode() {return mMode;};
    uint8 getMenuNumber() {return mMenuNumber;};
    void set(uint8 activate, uint8 menuNumber);
    void unpack();
    uint16 pack();
private:
    uint8 mMode;
    uint8 mMenuNumber;
};

class CAPacketMenuList : public CAPacketElement {
public:
    CAPacketMenuList(CAPacket& caPacket);
    uint8 getPacketType() {return PID_MENU_LIST;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8 getMenuId() {return mMenuId;};
    uint8 getModuleId0() {return mModuleId0;};
    uint8 getModuleMask0() {return mModuleMask0;};
    uint8 getModuleId1() {return mModuleId1;};
    uint8 getModuleMask1() {return mModuleMask1;};
    uint8 getModuleId2() {return mModuleId2;};
    uint8 getModuleMask2() {return mModuleMask2;};
    uint8 getModuleId3() {return mModuleId3;};
    uint8 getModuleMask3() {return mModuleMask3;};
    uint8 getModuleTypeId0() {return mModuleTypeId0;};
    uint8 getModuleTypeMask0() {return mModuleTypeMask0;};
    uint8 getModuleTypeId1() {return mModuleTypeId1;};
    uint8 getModuleTypeMask1() {return mModuleTypeMask1;};
    const char* getMenuName() {return mMenuName.c_str();};
    void set(uint8 menuId, uint8 moduleId0, uint8 moduleMask0,  uint8 moduleId1, uint8 moduleMask1,
                uint8 moduleId2, uint8 moduleMask2, uint8 moduleId3, uint8 moduleMask3,
                uint8 moduleTypeId0, uint8 moduleTypeMask0, uint8 moduleTypeId1, uint8 moduleTypeMask1,
                String menuName);
    void unpack();
    uint16 pack();
private:
    uint8 mMenuId;
    uint8 mModuleId0;
    uint8 mModuleMask0;
    uint8 mModuleId1;
    uint8 mModuleMask1;
    uint8 mModuleId2;
    uint8 mModuleMask2;
    uint8 mModuleId3;
    uint8 mModuleMask3;
    uint8 mModuleTypeId0;
    uint8 mModuleTypeMask0;
    uint8 mModuleTypeId1;
    uint8 mModuleTypeMask1;
    String mMenuName;
};

class CAPacketModuleList : public CAPacketElement {
public:
    CAPacketModuleList(CAPacket& caPacket);
    uint8 getPacketType() {return PID_MODULE_LIST;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8 getModuleId() {return mModuleId;};
    uint8 getModuleTypeId() {return mModuleTypeId;};
    const char* getModuleName() {return mModuleName.c_str();};
    void set(uint8 moduleId, uint8 moduleTypeId, String moduleName);
    void unpack();
    uint16 pack();
private:
    uint8 mModuleId;
    uint8 mModuleTypeId;
    String mModuleName;
};

class CAPacketLogger : public CAPacketElement {
public:
    CAPacketLogger(CAPacket& caPacket);
    uint8 getPacketType() {return PID_LOGGER;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    const char* getLog() {return mLog.c_str();};
    void set(String log);
    void unpack();
    uint16 pack();
private:
    String mLog;
};

class CAPacketCamState : public CAPacketElement {
public:
    const uint8 CAM0    = 0x01;
    const uint8 CAM1    = 0x02;
    const uint8 CAM2    = 0x04;
    const uint8 CAM3    = 0x08;
    const uint8 CAM4    = 0x10;
    const uint8 CAM5    = 0x20;
    const uint8 CAM6    = 0x40;
    const uint8 CAM7    = 0x80;
    CAPacketCamState(CAPacket& caPacket);
    uint8 getPacketType() {return PID_CAM_STATE;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8 getMultiplier() {return mMultiplier;};
    uint8 getFocus() {return mFocus;};
    uint8 getShutter() {return mShutter;};
    void set(uint8 multiplier, uint8 focus, uint8 shutter);
    void unpack();
    uint16 pack();
private:
    uint8 mMultiplier;
    uint8 mFocus;
    uint8 mShutter;
};

class CAPacketCamSettings : public CAPacketCamSettingsBase, public CAPacketElement  {
public:
    CAPacketCamSettings(CAPacket& caPacket);
    uint8 getPacketType() {return PID_CAM_SETTINGS;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    void unpack();
    uint16 pack();
private:
};

class CAPacketIntervalometer : public CAPacketElement {
public:
    CAPacketIntervalometer(CAPacket& caPacket);
    uint8 getPacketType() {return PID_INTERVALOMETER;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint16 getStartHours() {return mStartHours;};
    uint8 getStartMinutes() {return mStartMinutes;};
    uint8 getStartSeconds() {return mStartSeconds;};
    uint16 getStartMilliseconds() {return mStartMilliseconds;};
    uint16 getStartMicroseconds() {return mStartMicroseconds;};
    uint16 getIntervalHours() {return mIntervalHours;};
    uint8 getIntervalMinutes() {return mIntervalMinutes;};
    uint8 getIntervalSeconds() {return mIntervalSeconds;};
    uint16 getIntervalMilliseconds() {return mIntervalMilliseconds;};
    uint16 getIntervalMicroseconds() {return mIntervalMicroseconds;};
    uint16 getRepeats() {return mRepeats;};
    void set(uint16 startHours, uint8 startMinutes, uint8 startSeconds, uint16 startMilliseconds,
                uint16 startMicroseconds, uint16 intervalHours, uint8 intervalMinutes,
                uint8 intervalSeconds, uint16 intervalMilliseconds, uint16 intervalMicroseconds,
                uint16 repeats);
    void unpack();
    uint16 pack();
private:
    uint16 mStartHours;
    uint8 mStartMinutes;
    uint8 mStartSeconds;
    uint16 mStartMilliseconds;
    uint16 mStartMicroseconds;
    uint16 mIntervalHours;
    uint8 mIntervalMinutes;
    uint8 mIntervalSeconds;
    uint16 mIntervalMilliseconds;
    uint16 mIntervalMicroseconds;
    uint16 mRepeats;
};

class CAPacketControlFlags : public CAPacketElement {
public:
    CAPacketControlFlags(CAPacket& caPacket);
    uint8 getPacketType() {return PID_CONTROL_FLAGS;};
    uint8 getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8 getSlaveModeEnable() {return mSlaveModeEnable;};
    uint8 getExtraMessagesEnable() {return mExtraMessagesEnable;};
    void set(uint8 slaveModeEnabe, uint8 extraMessagesEnable);
    void unpack();
    uint16 pack();
private:
    uint8 mSlaveModeEnable;
    uint8 mExtraMessagesEnable;
};

#endif // __CAPACKET_H__
