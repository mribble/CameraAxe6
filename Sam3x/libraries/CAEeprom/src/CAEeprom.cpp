
#include "CAEeprom.h"

CAEeprom::CAEeprom(unioDevice device)
{
    m_deviceAddr = ADDR_11AA;
    
    switch (device)
    {
        case CA_INTERNAL:
            m_hwPortPin = CAU::getOnboardDevicePin(EEPROM_PIN);
            break;
        case CA_MODULE0:
            m_hwPortPin = CAU::getModulePin(0, 3);
            break;
        case CA_MODULE1:
            m_hwPortPin = CAU::getModulePin(1, 3);
            break;
        case CA_MODULE2:
            m_hwPortPin = CAU::getModulePin(2, 3);
            break;
        case CA_MODULE3:
            m_hwPortPin = CAU::getModulePin(3, 3);
            break;
        default:
            CA_ASSERT(0, "Invalid CAEeprom device");
    }
}

boolean CAEeprom::read(uint8 *buf, uint16 addr, uint16 len)
{
    uint8 cmd[4];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_READ;
    cmd[2] = (uint8)(addr >> 8);
    cmd[3] = (uint8)(addr & 0xff);
    standbyPulse();
    noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 4, CA_FALSE))
    {
        interrupts();
        return CA_FALSE;
    }
    if (!readRawBytes(buf, len))
    {
        interrupts();
        return CA_FALSE;
    }
    interrupts();
    return CA_TRUE;
}

boolean CAEeprom::write(const uint8 *buf, uint16 addr, uint16 len)
{
    uint16 tlen;
    
    while (len > 0)
    {
        tlen=len;
        if (((addr & 0x0f) + tlen) > 16)
        {
            // Writing crosses page boundary, truncate to not do this
            tlen = 16 - (addr & 0x0f);
        }

        if (!writeEnable())
        {
            return CA_FALSE;
        }
        if (!writeStart(buf, addr, tlen))
        {
            return CA_FALSE;
        }
        if (!writeWaitComplete())
        {
            return CA_FALSE;
        }
        buf     += tlen;
        addr    += tlen;
        len     -= tlen;
    }
    return CA_TRUE;
}

boolean CAEeprom::statusRead(unioReadStatus *status)
{
    uint8 cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_RDSR;
    standbyPulse();
    noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, CA_FALSE))
    {
        interrupts();
        return CA_FALSE;
    }
    if (!readRawBytes((uint8*)status, 1))
    {
        interrupts();
        return CA_FALSE;
    }
    interrupts();
    return CA_TRUE;
}

boolean CAEeprom::statusWrite(unioWriteStatus status)
{
    uint8 cmd[3];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRSR;
    cmd[2] = (uint8)status;
    standbyPulse();
    noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 3, CA_TRUE))
    {
        interrupts();
        return CA_FALSE;
    }
    interrupts();
    return CA_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//PRIVATE
////////////////////////////////////////////////////////////////////////////////

// This low to high pulse is required to make sure the bus is in sync
void CAEeprom::standbyPulse()
{
    CAU::pinMode(m_hwPortPin, OUTPUT);
    CAU::digitalWrite(m_hwPortPin, LOW);
    delayMicroseconds(UNIO_DELAY_TSS);
    CAU::digitalWrite(m_hwPortPin, HIGH);
    delayMicroseconds(UNIO_DELAY_TSTBY);
}

// Manchester encoding beaks every bit into quarters.
// Same path used for reading and writing
boolean CAEeprom::readWriteBit(boolean bit)
{
    boolean s0, s1;
    CAU::digitalWrite(m_hwPortPin, !bit);
    delayMicroseconds(UNIO_DELAY_QUARTER_BIT);
    s0 =  CAU::digitalRead(m_hwPortPin);
    delayMicroseconds(UNIO_DELAY_QUARTER_BIT);
    CAU::digitalWrite(m_hwPortPin, !!bit);
    delayMicroseconds(UNIO_DELAY_QUARTER_BIT);
    s1 =  CAU::digitalRead(m_hwPortPin);
    delayMicroseconds(UNIO_DELAY_QUARTER_BIT);
    return (!s0 && s1);
}

boolean CAEeprom::readBit()
{
    boolean s;
    CAU::pinMode(m_hwPortPin, INPUT);
    s = readWriteBit(HIGH);
    CAU::pinMode(m_hwPortPin, OUTPUT);
    return s;
}

boolean CAEeprom::sendByte(uint8 b, boolean mak)
{
    uint8 i;
    for (i=0; i<8; ++i)
    {
        readWriteBit(b&0x80);
        b<<=1;
    }
    readWriteBit(mak);
    return readBit();
}

boolean CAEeprom::readByte(uint8 *b, boolean mak)
{
    uint8 i;
    uint8 data=0;
    CAU::pinMode(m_hwPortPin, INPUT);
    for (i=0; i<8; ++i)
    {
        data = (data << 1) | readWriteBit(HIGH);
    }
    CAU::pinMode(m_hwPortPin, OUTPUT);
    *b = data;
    readWriteBit(mak);
    return readBit();
}

//Send data with MAK  if end commend or NoMAK if not last byte
boolean CAEeprom::sendRawBytes(const uint8 *buf, uint16 len, boolean end)
{
    uint16 i;
    boolean ret = CA_TRUE;
    for(i=0; i<len; ++i)
    {
        boolean mak = !(((i+1)==len) && end);
        if (!sendByte(buf[i], mak))
        {
            ret = CA_FALSE;
            break;
        }
    }
    return ret;
}

boolean CAEeprom::readRawBytes(uint8 *buf, uint16 len)
{
    uint16 i;
    boolean ret = CA_TRUE;
    for(i=0; i<len; ++i)
    {
        boolean mak = !((i+1)==len);
        if (!readByte(buf+i, mak))
        {
            ret = CA_FALSE;
            break;
        }
    }
    return ret;
}

// There is no SAK for this command
void CAEeprom::startHeader()
{
    CAU::digitalWrite(m_hwPortPin, LOW);
    delayMicroseconds(UNIO_DELAY_THDR);
    sendByte(UNIO_CMD_STARTHEADER, CA_TRUE);
}

boolean CAEeprom::writeEnable()
{
    uint8 cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WREN;
    standbyPulse();
    noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, CA_TRUE))
    {
        interrupts();
        return CA_FALSE;
    }
    
    interrupts();
    return CA_TRUE;
}

boolean CAEeprom::writeDisable()
{
    uint8 cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRDI;
    standbyPulse();
    noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, CA_TRUE))
    {
        interrupts();
        return CA_FALSE;
    }
    interrupts();
    return CA_TRUE;
}

boolean CAEeprom::writeStart(const uint8 *buf, uint16 addr, uint16 len)
{
    uint8 cmd[4];
    if (((addr&0x0f) + len) > 16)
    {
        return false; // crosses page boundary
    }

    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRITE;
    cmd[2] = (uint8)(addr >> 8);
    cmd[3] = (uint8)(addr & 0xff);
    standbyPulse();
    noInterrupts();
    startHeader();
     if (!sendRawBytes(cmd, 4, CA_FALSE))
    {
        interrupts();
        return CA_FALSE;
    }
    if (!sendRawBytes(buf, len, CA_TRUE))
    {
        interrupts();
        return CA_FALSE;
    }
    interrupts();
    return true;
}

boolean CAEeprom::writeWaitComplete()
{
    uint8 cmd[2];
    uint8 stat;
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_RDSR;
    standbyPulse();
    do
    {
        CAU::digitalWrite(m_hwPortPin, HIGH);
        delayMicroseconds(UNIO_DELAY_TSS);
        noInterrupts();
        startHeader();
        if (!sendRawBytes(cmd, 2, CA_FALSE))
        {
            interrupts();
            return CA_FALSE;
        }
        if (!readRawBytes(&stat, 1))
        if (!sendRawBytes(cmd, 2, CA_FALSE))
        {
            interrupts();
            return CA_FALSE;
        }
        interrupts();  // Enable interrupts briefly between loops
    } while (stat & 0x01);

    return CA_TRUE;
}
