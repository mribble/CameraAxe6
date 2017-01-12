#ifndef __CASERIAL_H__
#define __CASERIAL_H__

#include "CATypes.h"

#define MAX_PACKET_SIZE      128

class CASerial {
public:
    CASerial() : mSize(0) {};
    void init(uint32 baud);
    boolean readOnePacket(uint8 *data);
    void write(uint8 *data);

private:
    HardwareSerial *mSerial;
    uint8 mSize;
};

#endif // __CASERIAL_H__
