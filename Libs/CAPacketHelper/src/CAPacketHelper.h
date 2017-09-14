#ifndef __CAPACKET_HELPER_H__
#define __CAPACKET_HELPER_H__

#include "CAPacket.h"

class CAPacketHelper {
public:
    static const uint16_t MAX_PACKET_SIZE = 128;
    
    CAPacketHelper() :
    mGuardFound(false),
    mSize(0),
    mPacker(STATE_PACKER, mData, MAX_PACKET_SIZE),
    mUnpacker(STATE_UNPACKER, mData, MAX_PACKET_SIZE)
    {};
    
    CAPacket& getUnpacker() {return mUnpacker;};
    uint8_t* getData() {return mData;};
    
    void init(HardwareSerial *serial, HardwareSerial *debugSerial);
    boolean readOnePacket(uint8_t *data);
    void writeOnePacket(uint8_t *data);
    void flushGarbagePackets();

    void writePacketString(uint8_t clientHostId, const char* str);
    void writePacketString(const String& str);
    void writePacketUint32(uint8_t clientHostId, uint32_t val);
    void writePacketUint32(const String& str);
    void writePacketTimeBox(uint8_t clientHostId, uint32_t seconds, uint32_t nanoseconds);
    void writePacketTimeBox(const String& str);
    void writePacketMenuSelect(uint8_t menuMode, const char* menuName);
    void writePacketMenuSelect(const String& str);
    void writePacketCamSettings(uint8_t camPortNumber, uint8_t mode, uint32_t delaySeconds, 
                uint32_t delayNanoSeconds, uint32_t durationSeconds, uint32_t durationNanoseconds, 
                uint32_t postDelaySeconds, uint32_t postDelayNanoseconds, uint8_t sequencer, uint8_t mirrorLockup);
    void writePacketCamSettings(const String& str);
    void writePacketIntervalometer(uint8_t enable, uint32_t startSeconds, uint32_t startNanoseconds, 
                                    uint32_t intervalSeconds, uint32_t intervalNanoseconds, uint16_t repeats);
    void writePacketIntervalometer(const String& str);
    void writePacketCamTrigger(uint8_t mode, uint8_t focus, uint8_t shutter);
    void writePacketCamTrigger(const String& str);
    
private:
    HardwareSerial *mSerial;
    HardwareSerial *mDebugSerial;
    bool mGuardFound;
    uint16_t mSize;
    CAPacket mPacker;
    CAPacket mUnpacker;
    uint8_t mData[MAX_PACKET_SIZE];
    
    uint16_t genPacketSize(uint8_t b0, uint8_t b1) {
        uint16_t ret = uint16_t(b0) + (uint16_t(b1)<<8);
        return ret;
    }

    uint8_t getPacketSize(uint16_t val, uint8_t byteNumber) {
        if (byteNumber == 0) {
            return val & 0xFF;
        } else {
            return val >> 8;
        }
    }
    
};

#endif // __CAPACKET_HELPER_H__
