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
                PID_END_SENTINEL        =  7, // Must be last
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

class CAPacketCamSettings : public CAPacketElement  {
public:
    const uint8_t SEQ0    = 0x01;
    const uint8_t SEQ1    = 0x02;
    const uint8_t SEQ2    = 0x04;
    const uint8_t SEQ3    = 0x08;
    const uint8_t SEQ4    = 0x10;
    const uint8_t SEQ5    = 0x20;
    const uint8_t SEQ6    = 0x40;
    const uint8_t SEQ7    = 0x80;
    CAPacketCamSettings();
    uint8_t getCamPortNumber() {return mCamPortNumber;};
    uint8_t getMode() {return mMode;};
    uint16_t getDelayHours() {return mDelayHours;};
    uint8_t getDelayMinutes() {return mDelayMinutes;};
    uint8_t getDelaySeconds() {return mDelaySeconds;};
    uint16_t getDelayMilliseconds() {return mDelayMilliseconds;};
    uint16_t getDelayMicroseconds() {return mDelayMicroseconds;};
    uint16_t getDurationHours() {return mDurationHours;};
    uint8_t getDurationMinutes() {return mDurationMinutes;};
    uint8_t getDurationSeconds() {return mDurationSeconds;};
    uint16_t getDurationMilliseconds() {return mDurationMilliseconds;};
    uint16_t getDurationMicroseconds() {return mDurationMicroseconds;};
    uint8_t getSequencer() {return mSequencer;};
    uint8_t getApplyIntervalometer() {return mApplyIntervalometer;};
    uint8_t getSmartPreview() {return mSmartPreview;};
    uint8_t getMirrorLockupEnable() {return mMirrorLockupEnable;};
    uint8_t getMirrorLockupMinutes() {return mMirrorLockupMinutes;};
    uint8_t getMirrorLockupSeconds() {return mMirrorLockupSeconds;};
    uint16_t getMirrorLockupMilliseconds() {return mMirrorLockupMilliseconds;};
    void set(uint8_t camPortNumber, uint8_t mode, uint16_t delayHours, uint8_t delayMinutes,
                uint8_t delaySeconds, uint16_t delayMilliseconds, uint16_t delayMicroseconds,
                uint16_t durationHours, uint8_t durationMinutes, uint8_t durationSeconds, 
                uint16_t durationMilliseconds, uint16_t durationMicroseconds, uint8_t sequencer,
                uint8_t applyIntervalometer, uint8_t smartPreview, uint8_t mirrorLockupEnable, 
                uint8_t mirrorLockupMinutes, uint8_t mirrorLockupSeconds, uint16_t mirrorLockupMilliseconds);
                CAPacketCamSettings(CAPacket& caPacket);
    void set(const String& str);
    uint8_t getPacketType() {return PID_CAM_SETTINGS;};
    uint8_t getClientHostId() {return NULL_CLIENT_HOST_ID;};
    void unpack();
    uint16_t pack();
    void packetToString(String& str);
private:
    uint8_t mCamPortNumber;
    uint8_t mMode;
    uint16_t mDelayHours;
    uint8_t mDelayMinutes;
    uint8_t mDelaySeconds;
    uint16_t mDelayMilliseconds;
    uint16_t mDelayMicroseconds;
    uint16_t mDurationHours;
    uint8_t mDurationMinutes;
    uint8_t mDurationSeconds;
    uint16_t mDurationMilliseconds;
    uint16_t mDurationMicroseconds;
    uint8_t mSequencer;
    uint8_t mApplyIntervalometer;
    uint8_t mSmartPreview;
    uint8_t mMirrorLockupEnable;
    uint8_t mMirrorLockupMinutes;
    uint8_t mMirrorLockupSeconds;
    uint16_t mMirrorLockupMilliseconds;
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

#endif // __CAPACKET_H__
