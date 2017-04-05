#include <CAPacketHelper.h>

#ifdef __SAM3X8E__
#include <CAUtility.h>
#endif


//#define USE_FLOW_CONTROL
#ifdef USE_FLOW_CONTROL

#define RING_BUF_SIZE 4096
uint8_t gBuf[RING_BUF_SIZE];
int16_t gHead = 0;
int16_t gTail = 0;

uint16_t CAPacketHelper::serialFlowControlAvailable() {
  int16_t available = gTail - gHead;
  if (available < 0) {
    available = RING_BUF_SIZE + available;
  }
  return available;
}

void CAPacketHelper::serialFlowControlPoll() {
  while(mSerial->available()) {
    gBuf[gTail] = mSerial->read();
    gTail = (gTail + 1) % RING_BUF_SIZE;
  }

  if (serialFlowControlAvailable() > RING_BUF_SIZE/2) {
    CAU::digitalWrite(mRtsPin, LOW);   // Tell master to stop sending more data
  } else {
    CAU::digitalWrite(mRtsPin, HIGH);  // Tell master to keep sending more data
  }
}

void CAPacketHelper::serialFlowControlRead(uint8_t *buf, uint16_t length) {
  for(uint16_t i=0; i<length; ++i) {
    buf[i] = gBuf[gHead];
    gHead = (gHead + 1) % RING_BUF_SIZE;
  }
}

void CAPacketHelper::serialFlowControlWrite(const uint8_t *buf, uint16_t length) {
  for(uint16_t i=0; i<length; ++i) {
    while (CAU::digitalRead(mCtsPin) != HIGH) {;} // Wait until RTS is high
    mSerial->write(buf[i]);
  }
}
#else

uint16_t CAPacketHelper::serialFlowControlAvailable() {
    return mSerial->available();
}

void CAPacketHelper::serialFlowControlPoll() {
    return;
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
    
#endif


void CAPacketHelper::init(HardwareSerial *serial, hwPortPin rts, hwPortPin cts) {
    mSerial = serial;
    flushGarbagePackets();
    
    mRtsPin = rts;
    mCtsPin = cts;
#ifdef USE_FLOW_CONTROL
    CAU::pinMode(mRtsPin, OUTPUT);
    CAU::pinMode(mCtsPin, INPUT);
    CAU::digitalWrite(mRtsPin, HIGH);
#endif
}

boolean CAPacketHelper::readOnePacket(uint8 *data) {
    boolean ret = CA_FALSE;
    uint8 avaliableBytes = serialFlowControlAvailable();
    
    // To read one packet you need to know the first two bytes in a packet is the size.  This code assumes that.
    // The third byte is always the packet type, but this code doesn't need to know that.
    
    serialFlowControlPoll();
    
    if (avaliableBytes >= 2) {
        if (mSize == 0) {
            uint8 buf[2];
            serialFlowControlRead(buf, 2);
            avaliableBytes -= 2;
            mSize = genPacketSize(buf[0], buf[1]);
            CA_ASSERT(mSize<MAX_PACKET_SIZE, "Invalid packet size");
        }

        if (avaliableBytes >= mSize-2) {
            data[0] = getPacketSize(mSize, 0);
            data[1] = getPacketSize(mSize, 1);
            serialFlowControlRead(data+2, mSize-2);
            mSize = 0;
            ret = CA_TRUE;
        }
    }
     return ret;
}

void CAPacketHelper::writeOnePacket(uint8 *data) {
    uint16 bufSize = genPacketSize(data[0], data[1]);

    serialFlowControlPoll();

    if (bufSize >= MAX_PACKET_SIZE) {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    serialFlowControlWrite(data, bufSize);
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
            }
        }
    }
}

void CAPacketHelper::flushGarbagePackets() {
    while (mSerial->read() != -1){}   // flush out all the writes
 }

void CAPacketHelper::writePacketLogger(const char* str) {
    CAPacketLogger pack0(mPacker);
    pack0.set(str);
    uint16 packSize = pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketTextDynamic(uint8 clientHostId, uint8 modAttribute, const char* text1) {
    CAPacketTextDynamic pack0(mPacker);
    pack0.set(clientHostId, modAttribute, "0", text1);
    uint16 packSize = pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketMenuList(uint8 menuId, uint8 moduleId0, uint8 moduleMask0,  uint8 moduleId1,
                uint8 moduleMask1, uint8 moduleId2, uint8 moduleMask2, uint8 moduleId3, uint8 moduleMask3,
                uint8 moduleTypeId0, uint8 moduleTypeMask0, uint8 moduleTypeId1, uint8 moduleTypeMask1,
                String menuName) {
    CAPacketMenuList pack0(mPacker);
    pack0.set(menuId, moduleId0, moduleMask0,  moduleId1, moduleMask1, moduleId2, moduleMask2, moduleId3, moduleMask3,
                moduleTypeId0, moduleTypeMask0, moduleTypeId1, moduleTypeMask1, menuName);
    uint16 packSize = pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}

void CAPacketHelper::writePacketEcho(uint8 mode, const char* str) {
    CAPacketEcho pack0(mPacker);
    pack0.set(mode, str);
    uint16 packSize = pack0.pack();
    writeOnePacket(mData);
    mPacker.resetBuffer();
}
