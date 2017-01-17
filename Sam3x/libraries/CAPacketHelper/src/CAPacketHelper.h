#ifndef __CAPACKET_HELPER_H__
#define __CAPACKET_HELPER_H__

#include "CATypes.h"

#define MAX_PACKET_SIZE      128

class CAPacketHelper {
public:
    CAPacketHelper() : mSize(0) {};
    void init(uint32 baud);
    boolean readOnePacket(uint8 *data);
    void writeOnePacket(uint8 *data);

private:
    HardwareSerial *mSerial;
    uint8 mSize;
};

#endif // __CAPACKET_HELPER_H__
