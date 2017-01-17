#ifndef __CAPACKET_HELPER_H__
#define __CAPACKET_HELPER_H__

#include "CATypes.h"
#include "CAPacket.h"

#define MAX_PACKET_SIZE      128
#define NUM_CAMERAS 8

class CAPacketHelper {
public:
    CAPacketHelper() : 
    mSize(0),
    mPacker(STATE_PACKER, mData, MAX_PACKET_SIZE),
    mUnpacker(STATE_UNPACKER, mData, MAX_PACKET_SIZE),
    mInterModuleLogic(mPacker),
    mCamSettings{mPacker, mPacker, mPacker, mPacker, mPacker, mPacker, mPacker, mPacker}
    {};
    
    void init(uint32 baud);
    boolean readOnePacket(uint8 *data);
    void writeOnePacket(uint8 *data);
    void processIncomingPacket();

public:     // todo make these private and create base classes to hold this data in g_ctx
    CAPacketInterModuleLogic mInterModuleLogic;
    CAPacketCamSettings mCamSettings[NUM_CAMERAS];
    
private:
    HardwareSerial *mSerial;
    uint8 mSize;
    CAPacket mPacker;
    CAPacket mUnpacker;
    uint8 mData[MAX_PACKET_SIZE];
};

#endif // __CAPACKET_HELPER_H__
