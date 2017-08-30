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
    void writePacketTimeBox(uint8_t clientHostId, uint16_t hours, uint8_t minutes, uint8_t seconds,
                uint16_t milliseconds, uint16_t microseconds, uint16_t nanoseconds);
    void writePacketTimeBox(const String& str);
    void writePacketMenuSelect(uint8_t menuMode, const char* menuName);
    void writePacketMenuSelect(const String& str);
    // todo add missing packet types
    
private:
    uint16_t serialFlowControlAvailable();
    void serialFlowControlRead(uint8_t *buf, uint16_t length);
    void serialFlowControlWrite(const uint8_t *buf, uint16_t length);

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
