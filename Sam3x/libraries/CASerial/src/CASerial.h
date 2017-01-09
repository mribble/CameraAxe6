#ifndef __CASERIAL_H__
#define __CASERIAL_H__

#include "CATypes.h"

#define MAX_PACKET_SIZE      128

class CASerial
{
public:
    CASerial() : m_size(0) {};
    void init(uint32 baud);
    boolean read(uint8 *bufSize, uint8 *header, uint8 *data);
    void write(uint8 *data);

private:
    HardwareSerial *m_serial;
    uint8 m_size;
};

#endif // __CASERIAL_H__
