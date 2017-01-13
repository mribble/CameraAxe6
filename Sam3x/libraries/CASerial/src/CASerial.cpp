#include <CASerial.h>

#ifdef __SAM3X8E__
#include <CAUtility.h>
#endif

void CASerial::init(uint32 baud) {
#ifdef __SAM3X8E__
    mSerial = &Serial1;
    mSerial->begin(baud);
    delay(5000);
    while (mSerial->read() != -1){}   // flush out all the initialization writes
#elif ESP8266
    mSerial = &Serial1;
    mSerial->begin(baud);
#elif __RFduino__
    mSerial = &Serial;
    Serial.begin(baud);
#endif
}

boolean CASerial::readOnePacket(uint8 *data) {
    boolean ret = CA_FALSE;
    uint8 avaliableBytes = mSerial->available();
    
    // To read one packet you need to know the first byte in a packet is the size.  This code assumes that.
    // The second byte is always the packet type, but this code doesn't need to know that.
    
    if (avaliableBytes) {
        if (mSize == 0) {
            mSerial->readBytes((char*)&mSize, 1);
            CA_ASSERT(mSize<MAX_PACKET_SIZE, "Invalid packet size");
        }
        
        if (avaliableBytes >= mSize-1) {
            data[0] = mSize;
            mSerial->readBytes(data+1, mSize-1);
            mSize = 0;
            ret = CA_TRUE;
        }
    }
     return ret;
}

void CASerial::writeOnePacket(uint8 *data) {
    uint8 val;
    uint8 bufSize = data[0];  // First byte is the size

    if (bufSize >= MAX_PACKET_SIZE) {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    val = mSerial->write(data, bufSize);
    CA_ASSERT(val==bufSize, "Failed CASerial::writePacket");
}

