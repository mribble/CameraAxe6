#include <CAPacketHelper.h>

uint16_t CAPacketHelper::serialFlowControlAvailable() {
    return mSerial->available();
}

void CAPacketHelper::serialFlowControlRead(uint8_t *buf, uint16_t length) {
    mSerial->readBytes(buf, length);
    // Code below prints out values of incoming packets for debug
    //for(int i = 0; i < length; ++i) {
    //    CAU::log("%d,",buf[i]);
    //}
    //CAU::log("\n");
}

void CAPacketHelper::serialFlowControlWrite(const uint8_t *buf, uint16_t length) {
    mSerial->write(buf, length);
}
    
void CAPacketHelper::init(HardwareSerial *serial, HardwareSerial *debugSerial) {
    mSerial = serial;
    mDebugSerial = debugSerial;
    flushGarbagePackets();
}

boolean CAPacketHelper::readOnePacket(uint8_t *data) {
    boolean ret = false;
    uint16_t avaliableBytes = serialFlowControlAvailable();

    // This reads the first byte, which verifies it's a valid packet, and then moves on
    //  Unless it isn't the guard byte and then it writes the char to the output serial port
    if (avaliableBytes && (mGuardFound == false)) {
        for(uint16_t i=0; i<avaliableBytes; ++i) {
            uint8_t buf[1];
            serialFlowControlRead(buf, PACK_GUARD_SZ);
            if (buf[0] == GUARD_PACKET) {
                mGuardFound = true;
                break;
            } else {
                if (mDebugSerial) {
                    mDebugSerial->print((char)(buf[0]));
                }
            }
        }
    }

    // To read one packet you need to know the first byte is the guard.  The next two bytes in a packet is the size.  
    // This code assumes that.  The third byte is always the packet type, but this code doesn't need to know that.

    avaliableBytes = serialFlowControlAvailable();
    if (mGuardFound && (avaliableBytes >= PACK_SIZE_SZ)) {
        if (mSize == 0) {
            uint8_t buf[PACK_SIZE_SZ];
            serialFlowControlRead(buf, PACK_SIZE_SZ);
            avaliableBytes -= PACK_SIZE_SZ;
            mSize = genPacketSize(buf[0], buf[1]);
            CA_ASSERT(mSize<MAX_PACKET_SIZE, "Invalid packet size");
        }

        if (avaliableBytes >= mSize-(PACK_GUARD_SZ+PACK_SIZE_SZ)) {
            data[0] = GUARD_PACKET;
            data[1] = getPacketSize(mSize, 0);
            data[2] = getPacketSize(mSize, 1);
            serialFlowControlRead(data+(PACK_GUARD_SZ+PACK_SIZE_SZ), mSize-(PACK_GUARD_SZ+PACK_SIZE_SZ));
            mSize = 0;
            mGuardFound = false;
            ret = true;
        }
    }
     return ret;
}

void CAPacketHelper::writeOnePacket(uint8_t *data) {
    CA_ASSERT(data[0] == GUARD_PACKET, "Failed packet guard check")
    uint16_t bufSize = genPacketSize(data[1], data[2]);

    if (bufSize >= MAX_PACKET_SIZE) {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    serialFlowControlWrite(data, bufSize);
}

void CAPacketHelper::flushGarbagePackets() {
    while (mSerial->read() != -1){}   // flush out all the writes
 }

void CAPacketHelper::writePacketString(uint8_t clientHostId, const char* str) {
    CAPacketString pack0(mPacker);
    pack0.set(clientHostId, str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketString(const String& str) {
    CAPacketString pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketUint32(uint8_t clientHostId, uint32_t val) {
    CAPacketUint32 pack0(mPacker);
    pack0.set(clientHostId, val);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketUint32(const String& str) {
    CAPacketUint32 pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketTimeBox(uint8_t clientHostId, uint32_t nanoseconds, uint32_t seconds){
    CAPacketTimeBox pack0(mPacker);
    pack0.set(clientHostId, nanoseconds, seconds);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketTimeBox(const String& str) {
    CAPacketTimeBox pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketMenuSelect(uint8_t menuMode, const char* menuName) {
    CAPacketMenuSelect pack0(mPacker);
    pack0.set(menuMode, menuName);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketMenuSelect(const String& str) {
    CAPacketMenuSelect pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

