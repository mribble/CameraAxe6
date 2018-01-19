////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
// 
// This class implements a binary protocol for sending the type of data data between sam3x and esp8266 via serial
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CAPACKET_H__
#define __CAPACKET_H__

#include <CAStd.h>

enum packetId  {PID_START_SENTINEL      =  0,  // Must be first
                PID_STRING              =  1,
                PID_UINT32              =  2,
                PID_TIME_BOX            =  3,
                PID_MENU_SELECT         =  4,
                PID_CAM_SETTINGS        =  5,
                PID_INTERVALOMETER      =  6,
                PID_CAM_TRIGGER         =  7,
                PID_PERIODIC_DATA       =  8,
                PID_END_SENTINEL        =  9, // Must be last
               };

enum packetState { STATE_PACKER=1, STATE_UNPACKER=2 };

#define MAX_STRING_SIZE  	64

#define GUARD_PACKET     	22

#define PACK_GUARD_SZ     1
#define PACK_SIZE_SZ      2
#define PACK_TYPE_SZ      1
#define PACK_TOTAL_SZ     (PACK_GUARD_SZ+PACK_SIZE_SZ+PACK_TYPE_SZ)

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
    CAPacket(uint8_t state, uint8_t *buf, uint16_t bufSize);
    void resetBuffer();                                 // Resets the data buffer back to the start
    boolean unpackGuard();                              // Test to make sure it's a valid packet
    uint16_t unpackSize();                              // Unpacks the size of the packet
    uint8_t unpackType();                               // Unpacks the type of the packet
    uint32_t unpacker(uint8_t unpackBits);              // Unpacks 1..32 bits from the byte stream
    void unpackerString(String& str);                   // Unpacks a null terminated string from the byte stream
    void packer(uint32_t val, uint8_t packBits);        // Packs 1..32 bits into the byte stream
    void packerString(const char* src);                 // Packs a null terminated string into the byte stream
    void flushPacket();                                 // This just does some validation

private:    
    uint8_t mBitsUsed;        // Bits of an uncompleted byte (pack or unpack) used in the byte stream
    uint8_t mBitsVal;         // Value of the unused bits
    uint16_t mBytesUsed;      // Number of bytes used in the pack or unpack stream
    uint8_t mState;           // State of the class (prevents invalid use like unpacking from a pack buffer)
    uint8_t* mBuf;            // The byte stream
    uint16_t mBufSize;        // Size of the byte stream
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAPacketElement - This is an interface class for all the specific packet types that follow.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CAPacketElement {
public:
    virtual ~CAPacketElement() {};
    virtual uint8_t getPacketType() = 0;
    virtual uint8_t getClientHostId() = 0;
    virtual void unpack() = 0;
    virtual uint16_t pack() = 0;

    // This walks through the strings from javascript that contain all the data needed to generate a packet
    uint32_t getUint32FromString(uint16_t& startIndex, const String& str) {
        uint32_t val;
        int16_t endIndex = str.indexOf('~', startIndex);
        CA_ASSERT(endIndex!=-1, "Failed check");
        val = (str.substring(startIndex, endIndex)).toInt();
        startIndex = endIndex+1;
        return val;
    }

    // This walks through the strings from javascript that contain all the data needed to generate a packet
    String getStringFromString(uint16_t& startIndex, const String& str) {
        String val;
        int16_t endIndex = str.indexOf('~', startIndex);
        CA_ASSERT(endIndex!=-1, "Failed check");
        val = str.substring(startIndex, endIndex);
        startIndex = endIndex+1;
        return val;
    }
    
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

class CAPacketString : public CAPacketElement {
public:
    CAPacketString(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_STRING;};
    uint8_t getClientHostId() {return mClientHostId;};
    const char* getString() {return mString.c_str();};
    void set(uint8_t clientHostId, String str);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mClientHostId;
    String mString;
};

class CAPacketUint32 : public CAPacketElement {
public:
    CAPacketUint32(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_UINT32;};
    uint8_t getClientHostId() {return mClientHostId;};
    uint32_t getValue() {return mValue;};
    void set(uint8_t clientHostId, uint32_t value);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mClientHostId;
    uint32_t mValue;
};

class CAPacketTimeBox : public CAPacketElement {
public:
    CAPacketTimeBox(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_TIME_BOX;};
    uint8_t getClientHostId() {return mClientHostId;};
    uint32_t getSeconds() {return mSeconds;};
    uint32_t getNanoseconds() {return mNanoseconds;};
    void set(uint8_t clientHostId, uint32_t seconds, uint32_t nanoseconds);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mClientHostId;
    uint32_t mSeconds;
    uint32_t mNanoseconds;
};

class CAPacketMenuSelect : public CAPacketElement {
public:
    CAPacketMenuSelect(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_MENU_SELECT;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8_t getMenuMode() {return mMenuMode;};
    const char* getMenuName() {return mMenuName.c_str();};
    void set(uint8_t menuMode, String menuName);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mMenuMode;
    String mMenuName;
};

enum CAM_SETTINGS_SEQ {
    CA_SEQ0    = 0x01,
    CA_SEQ1    = 0x02,
    CA_SEQ2    = 0x04,
    CA_SEQ3    = 0x08,
    CA_SEQ4    = 0x10,
    CA_SEQ5    = 0x20,
    CA_SEQ6    = 0x40,
    CA_SEQ7    = 0x80
};
enum CAM_SETTINGS_MODE {
    CA_MODE_NONE           = 0,
    CA_MODE_CAMERA         = 1,
    CA_MODE_PREFOCUS       = 2,
    CA_MODE_SMART_PREFOCUS = 3,
    CA_MODE_FLASH          = 4,
};

class CAPacketCamSettings : public CAPacketElement  {
public:
    CAPacketCamSettings();
    CAPacketCamSettings(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_CAM_SETTINGS;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8_t getCamPortNumber() {return mCamPortNumber;};
    uint8_t getMode() {return mMode;};
    uint32_t getDelaySeconds() {return mDelaySeconds;};
    uint32_t getDelayNanoseconds() {return mDelayNanoseconds;};
    uint32_t getDurationSeconds() {return mDurationSeconds;};
    uint32_t getDurationNanoseconds() {return mDurationNanoseconds;};
    uint32_t getPostDelaySeconds() {return mPostDelaySeconds;};
    uint32_t getPostDelayNanoseconds() {return mPostDelayNanoseconds;};
    uint8_t getSequencer() {return mSequencer;};
    uint8_t getMirrorLockup() {return mMirrorLockup;};
    void set(uint8_t camPortNumber, uint8_t mode, uint32_t delaySeconds, uint32_t delayNanoSeconds,
                uint32_t durationSeconds, uint32_t durationNanoseconds, uint32_t postDelaySeconds,
                uint32_t postDelayNanoseconds, uint8_t sequencer, uint8_t mirrorLockup);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mCamPortNumber;
    uint8_t mMode;
    uint32_t mDelaySeconds;
    uint32_t mDelayNanoseconds;
    uint32_t mDurationSeconds;
    uint32_t mDurationNanoseconds;
    uint32_t mPostDelaySeconds;
    uint32_t mPostDelayNanoseconds;
    uint8_t mSequencer;
    uint8_t mMirrorLockup;
};

class CAPacketIntervalometer : public CAPacketElement {
public:
    CAPacketIntervalometer(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_INTERVALOMETER;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8_t getEnable() {return mEnable;};
    uint32_t getStartSeconds() {return mStartSeconds;};
    uint32_t getStartNanoseconds() {return mStartNanoseconds;};
    uint32_t getIntervalSeconds() {return mIntervalSeconds;};
    uint32_t getIntervalNanoseconds() {return mIntervalNanoseconds;};
    uint16_t getRepeats() {return mRepeats;};
    void set(uint8_t enable, uint32_t startSeconds, uint32_t startNanoseconds,
                uint32_t intervalSeconds, uint32_t intervalNanoseconds, uint16_t repeats);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mEnable;
    uint32_t mStartSeconds;
    uint32_t mStartNanoseconds;
    uint32_t mIntervalSeconds;
    uint32_t mIntervalNanoseconds;
    uint16_t mRepeats;
};

enum CAM_TRIGGER_MODE {
    CA_MODE_STANDARD       = 0,
    CA_MODE_TOGGLE         = 1,
};

class CAPacketCamTrigger : public CAPacketElement {
public:
    CAPacketCamTrigger(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_CAM_TRIGGER;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint8_t getMode() {return mMode;};
    uint8_t getFocus() {return mFocus;};
    uint8_t getShutter() {return mShutter;};
    void set(uint8_t mode, uint32_t focus, uint32_t shutter);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mMode;
    uint8_t mFocus;
    uint8_t mShutter;
};

class CAPacketPeriodicData : public CAPacketElement {
public:
    CAPacketPeriodicData(CAPacket& caPacket);
    uint8_t getPacketType() {return PID_PERIODIC_DATA;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    uint16_t getVoltage() {return mVoltage;};
    void set(uint16_t voltage);
    void set(const String& str);
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint16_t mVoltage;
};

#endif // __CAPACKET_H__
