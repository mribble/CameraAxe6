#ifndef __CAPACKET_HELPER_H__
#define __CAPACKET_HELPER_H__

#include "CATypes.h"
#include "CAPacket.h"
#include "CAUtility.h"

#define MAX_PACKET_SIZE      128
#define NUM_CAMERAS 8

class CAPacketHelper {
public:
    CAPacketHelper() : 
    mSize(0),
    mPacker(STATE_PACKER, mData, MAX_PACKET_SIZE),
    mUnpacker(STATE_UNPACKER, mData, MAX_PACKET_SIZE)
    {};
    
    CAPacket& getUnpacker() {return mUnpacker;};
    uint8* getData() {return mData;};
    
    void init(HardwareSerial *serial, hwPortPin rts, hwPortPin cts);
    boolean readOnePacket(uint8 *data);
    void writeOnePacket(uint8 *data);
    void writeMenu(const uint8 *sData, uint16 sz);
    void flushGarbagePackets();
    
    //todo add missing packet functions here
    void writePacketLogger(const char* str);
    void writePacketTextDynamic(uint8 clientHostId, uint8 modAttribute, const char* text1);
    void writePacketMenuList(uint8 menuId, uint8 moduleId0, uint8 moduleMask0,  uint8 moduleId1, uint8 moduleMask1,
                uint8 moduleId2, uint8 moduleMask2, uint8 moduleId3, uint8 moduleMask3,
                uint8 moduleTypeId0, uint8 moduleTypeMask0, uint8 moduleTypeId1, uint8 moduleTypeMask1,
                String menuName);
    void writePacketEcho(uint8 mode, const char* str);

private:
    uint16_t serialFlowControlAvailable();
    void serialFlowControlPoll();
    void serialFlowControlRead(uint8_t *buf, uint16_t length);
    void serialFlowControlWrite(const uint8_t *buf, uint16_t length);

    HardwareSerial *mSerial;
    hwPortPin mRtsPin;
    hwPortPin mCtsPin;
    uint16 mSize;
    CAPacket mPacker;
    CAPacket mUnpacker;
    uint8 mData[MAX_PACKET_SIZE];
    
    uint16 genPacketSize(uint8 b0, uint8 b1) {
        uint16 ret = uint16(b0) + (uint16(b1)<<8);
        return ret;
    }

    uint8 getPacketSize(uint16 val, uint8 byteNumber) {
        if (byteNumber == 0) {
            return val && 0xFF;
        } else {
            return val >> 8;
    }
}
    
};

#endif // __CAPACKET_HELPER_H__
