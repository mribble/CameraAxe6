#include <CAPacketHelper.h>

#ifdef __SAM3X8E__
#include <CAUtility.h>
#endif

void CAPacketHelper::init(uint32 baud) {
    mSerial = &Serial1;
    mSerial->begin(baud);
    delay(5000);
    while (mSerial->read() != -1){}   // flush out all the initialization writes
}

boolean CAPacketHelper::readOnePacket(uint8 *data) {
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

void CAPacketHelper::writeOnePacket(uint8 *data) {
    uint8 val;
    uint16 bufSize = genPacketSize(data[0], data[1]);
    
    if (bufSize >= MAX_PACKET_SIZE) {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    val = mSerial->write(data, bufSize);
    CA_ASSERT(val==bufSize, "Failed CAPacketHelper::writePacket");
}

void CAPacketHelper::writeMenu(const uint8 *sData, uint16 sz) {
    uint16 currentPacketSize = 0;
    uint16 currentPacketIndex = 0;
    for(uint16 i=0; i<sz; ++i)
    {
        if (currentPacketSize == 0) {
            uint16 b0 = pgm_read_byte_near(sData+(i++));
            uint16 b1 = pgm_read_byte_near(sData+i);
            currentPacketSize = genPacketSize(b0, b1);
            mData[currentPacketIndex++] = b0;
            mData[currentPacketIndex++] = b1;
        }
        else {
            mData[currentPacketIndex++] = pgm_read_byte_near(sData+i);
            if (currentPacketIndex == currentPacketSize) {
                writeOnePacket(mData);
                mPacker.resetBuffer();
                currentPacketIndex = 0;
                currentPacketSize = 0;
                delay(500);
            }
        }
    }
}

void CAPacketHelper::writePacketLogger(const char* str) {
    CAPacketLogger pack0(mPacker);
    pack0.set(str);
    uint8 packSize = pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}
