////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Manages eeprom chip
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

boolean CAEeprom::read(uint8_t *buf, uint16_t addr, uint16_t len)
{
    uint8_t cmd[4];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_READ;
    cmd[2] = (uint8_t)(addr >> 8);
    cmd[3] = (uint8_t)(addr & 0xff);
    standbyPulse();
    //noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 4, false))
    {
        //interrupts();
        return false;
    }
    if (!readRawBytes(buf, len))
    {
        //interrupts();
        return false;
    }
    //interrupts();
    return true;
}

boolean CAEeprom::write(const uint8_t *buf, uint16_t addr, uint16_t len)
{
    uint16_t tlen;
    
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
            return false;
        }
        if (!writeStart(buf, addr, tlen))
        {
            return false;
        }
        if (!writeWaitComplete())
        {
            return false;
        }
        buf     += tlen;
        addr    += tlen;
        len     -= tlen;
    }
    return true;
}

boolean CAEeprom::statusRead(unioReadStatus *status)
{
    uint8_t cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_RDSR;
    standbyPulse();
    //noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, false))
    {
        //interrupts();
        return false;
    }
    if (!readRawBytes((uint8_t*)status, 1))
    {
        //interrupts();
        return false;
    }
    //interrupts();
    return true;
}

boolean CAEeprom::statusWrite(unioWriteStatus status)
{
    uint8_t cmd[3];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRSR;
    cmd[2] = (uint8_t)status;
    standbyPulse();
    //noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 3, true))
    {
        //interrupts();
        return false;
    }
    //interrupts();
    return true;
}

boolean CAEeprom::readModuleId(uint8_t *val) {
    uint8_t buf[5];
    if (read(buf, EEPROM_ADDR, 5)) {
        if ((buf[0] == ((EEPROM_MAGIC_VAL>>0)&0xff)) &&
            (buf[1] == ((EEPROM_MAGIC_VAL>>8)&0xff)) &&
            (buf[2] == ((EEPROM_MAGIC_VAL>>16)&0xff)) &&
            (buf[3] == ((EEPROM_MAGIC_VAL>>24)&0xff))) {
            *val = buf[4];
            return true;
        } else {
            CA_ERROR("Module eeprom magic value check failed", 0);
            *val = 0;
            return false;
        } 
    }
    
    *val = 0;
    return false;
}

boolean CAEeprom::writeModuleId(uint8_t val) {
    uint8_t buf[5] = {(EEPROM_MAGIC_VAL>>0)&0xff,
                    (EEPROM_MAGIC_VAL>>8)&0xff,
                    (EEPROM_MAGIC_VAL>>16)&0xff,
                    (EEPROM_MAGIC_VAL>>24)&0xff,
                    val};
    return write(buf, EEPROM_ADDR, 5);
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

boolean CAEeprom::sendByte(uint8_t b, boolean mak)
{
    uint8_t i;
    for (i=0; i<8; ++i)
    {
        readWriteBit(b&0x80);
        b<<=1;
    }
    readWriteBit(mak);
    return readBit();
}

boolean CAEeprom::readByte(uint8_t *b, boolean mak)
{
    uint8_t i;
    uint8_t data=0;
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
boolean CAEeprom::sendRawBytes(const uint8_t *buf, uint16_t len, boolean end)
{
    uint16_t i;
    boolean ret = true;
    for(i=0; i<len; ++i)
    {
        boolean mak = !(((i+1)==len) && end);
        if (!sendByte(buf[i], mak))
        {
            ret = false;
            break;
        }
    }
    return ret;
}

boolean CAEeprom::readRawBytes(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    boolean ret = true;
    for(i=0; i<len; ++i)
    {
        boolean mak = !((i+1)==len);
        if (!readByte(buf+i, mak))
        {
            ret = false;
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
    sendByte(UNIO_CMD_STARTHEADER, true);
}

boolean CAEeprom::writeEnable()
{
    uint8_t cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WREN;
    standbyPulse();
    //noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, true))
    {
        //interrupts();
        return false;
    }
    
    //interrupts();
    return true;
}

boolean CAEeprom::writeDisable()
{
    uint8_t cmd[2];
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRDI;
    standbyPulse();
    //noInterrupts();
    startHeader();
    if (!sendRawBytes(cmd, 2, true))
    {
        //interrupts();
        return false;
    }
    //interrupts();
    return true;
}

boolean CAEeprom::writeStart(const uint8_t *buf, uint16_t addr, uint16_t len)
{
    uint8_t cmd[4];
    if (((addr&0x0f) + len) > 16)
    {
        return false; // crosses page boundary
    }

    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_WRITE;
    cmd[2] = (uint8_t)(addr >> 8);
    cmd[3] = (uint8_t)(addr & 0xff);
    standbyPulse();
    //noInterrupts();
    startHeader();
     if (!sendRawBytes(cmd, 4, false))
    {
        //interrupts();
        return false;
    }
    if (!sendRawBytes(buf, len, true))
    {
        //interrupts();
        return false;
    }
    //interrupts();
    return true;
}

boolean CAEeprom::writeWaitComplete()
{
    uint8_t cmd[2];
    uint8_t stat;
    cmd[0] = m_deviceAddr;
    cmd[1] = UNIO_CMD_RDSR;
    standbyPulse();
    do
    {
        CAU::digitalWrite(m_hwPortPin, HIGH);
        delayMicroseconds(UNIO_DELAY_TSS);
        //noInterrupts();
        startHeader();
        if (!sendRawBytes(cmd, 2, false))
        {
            //interrupts();
            return false;
        }
        if (!readRawBytes(&stat, 1))
        if (!sendRawBytes(cmd, 2, false))
        {
            //interrupts();
            return false;
        }
        //interrupts();  // Enable interrupts briefly between loops
    } while (stat & 0x01);

    return true;
}
