#include <CAPacketHelper.h>

void CAPacketHelper::init(HardwareSerial *serial, HardwareSerial *debugSerial) {
    mSerial = serial;
    mDebugSerial = debugSerial;
    flushGarbagePackets();
}

boolean CAPacketHelper::readOnePacket() {
    boolean ret = false;
    uint16_t avaliableBytes = mSerial->available();

    // This reads the first byte, which verifies it's a valid packet, and then moves on
    //  Unless it isn't the guard byte and then it writes the char to the output serial port
    if (avaliableBytes && (mGuardFound == false)) {
        for(uint16_t i=0; i<avaliableBytes; ++i) {
            uint8_t buf[1];
            mSerial->readBytes(buf, PACK_GUARD_SZ);
            if (buf[0] == GUARD_PACKET) {
                mGuardFound = true;
                break;
            } else {
                if (mDebugSerial && CHECK_SERIAL) {
                    mDebugSerial->print((char)(buf[0]));
                }
            }
        }
    }

    // To read one packet you need to know the first byte is the guard.  The next two bytes in a packet is the size.  
    // This code assumes that.  The third byte is always the packet type, but this code doesn't need to know that.
    avaliableBytes = mSerial->available();
    if(mGuardFound && (mSize == 0) && (avaliableBytes >= PACK_SIZE_SZ)) {
        uint8_t buf[PACK_SIZE_SZ];
        mSerial->readBytes(buf, PACK_SIZE_SZ);
        avaliableBytes -= PACK_SIZE_SZ;
        mSize = genPacketSize(buf[0], buf[1]);
        mData[0] = GUARD_PACKET;
        mData[1] = getPacketSize(mSize, 0);
        mData[2] = getPacketSize(mSize, 1);
        mTransferredBytes = PACK_GUARD_SZ+PACK_SIZE_SZ;
        CA_ASSERT(mSize<MAX_PACKET_SIZE, "Invalid packet size");
    }

    if (mSize && (mTransferredBytes < mSize) && avaliableBytes) {
        uint16_t transferBytes = min(avaliableBytes, mSize-mTransferredBytes);
        mSerial->readBytes(mData+mTransferredBytes, transferBytes);
        mTransferredBytes += transferBytes;
    }

    if (mSize && (mTransferredBytes == mSize)) {
        mSize = 0;
        mTransferredBytes = 0;
        mGuardFound = false;
        ret = true;
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
    mSerial->write(data, bufSize);
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

void CAPacketHelper::writePacketTimeBox(uint8_t clientHostId, uint32_t seconds, uint32_t nanoseconds){
    CAPacketTimeBox pack0(mPacker);
    pack0.set(clientHostId, seconds, nanoseconds);
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

void CAPacketHelper::writePacketCamSettings(uint8_t camPortNumber, uint8_t mode, uint32_t delaySeconds, 
                uint32_t delayNanoSeconds, uint32_t durationSeconds, uint32_t durationNanoseconds, 
                uint32_t postDelaySeconds, uint32_t postDelayNanoseconds, uint8_t sequencer, uint8_t mirrorLockup) {
    CAPacketCamSettings pack0(mPacker);
    pack0.set(camPortNumber, mode, delaySeconds, delayNanoSeconds, durationSeconds, durationNanoseconds, 
                postDelaySeconds, postDelayNanoseconds, sequencer, mirrorLockup);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketCamSettings(const String& str) {
    CAPacketCamSettings pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketIntervalometer(uint8_t enable, uint32_t startSeconds, uint32_t startNanoseconds, 
                        uint32_t intervalSeconds, uint32_t intervalNanoseconds, uint16_t repeats) {
    CAPacketIntervalometer pack0(mPacker);
    pack0.set(enable, startSeconds, startNanoseconds, intervalSeconds, intervalNanoseconds, repeats);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketIntervalometer(const String& str) {
    CAPacketIntervalometer pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketCamTrigger(uint8_t mode, uint8_t focus, uint8_t shutter) {
    CAPacketCamTrigger pack0(mPacker);
    pack0.set(mode, focus, shutter);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketCamTrigger(const String& str) {
    CAPacketCamTrigger pack0(mPacker);
    pack0.set(str);
    pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}
