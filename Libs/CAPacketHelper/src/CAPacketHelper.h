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
    void writeMenu(const uint8_t *sData, uint16_t sz);
    void flushGarbagePackets();

    //todo add missing packet functions here
    void writePacketLogger(const char* str);
    void writePacketString(uint8_t clientHostId, uint8_t flags, const char* str);
    void writePacketMenuList(uint8_t menuId, uint8_t moduleId0, uint8_t moduleMask0,  uint8_t moduleId1, uint8_t moduleMask1,
                uint8_t moduleId2, uint8_t moduleMask2, uint8_t moduleId3, uint8_t moduleMask3,
                uint8_t moduleTypeId0, uint8_t moduleTypeMask0, uint8_t moduleTypeId1, uint8_t moduleTypeMask1,
                String menuName);
    void writePacketEcho(uint8_t mode, const char* str);

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
            return val && 0xFF;
        } else {
            return val >> 8;
        }
    }
    
};

#endif // __CAPACKET_HELPER_H__
