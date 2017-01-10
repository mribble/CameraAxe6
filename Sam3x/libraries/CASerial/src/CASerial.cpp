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

boolean CASerial::read(uint8 *bufSize, uint8 *header, uint8 *data) {
    boolean ret = CA_FALSE;
    uint8 avaliableBytes = mSerial->available();
    
    if (avaliableBytes) {
        if (mSize == 0) {
            mSerial->readBytes((char*)&mSize, 1);
            CA_ASSERT(mSize<MAX_PACKET_SIZE, "Invalid packet size");
        }
        
        if (avaliableBytes >= mSize-1) {
            *bufSize = mSize;
            mSerial->readBytes((char*)header, 1);
            mSerial->readBytes((char*)data, mSize-2);
            mSize = 0;
            ret = CA_TRUE;
        }
    }
     return ret;
}

void CASerial::write(uint8 *data) {
    uint8 val;
    uint8 bufSize = data[0];  // First byte is the size

    if (bufSize >= MAX_PACKET_SIZE) {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    val = mSerial->write(data, bufSize);
    CA_ASSERT(val==bufSize, "Failed CASerial::writePacket");
}

