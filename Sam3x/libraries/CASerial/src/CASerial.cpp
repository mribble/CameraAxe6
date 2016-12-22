#include <CASerial.h>

#ifdef __SAM3X8E__
#include <CAUtility.h>
#endif

void CASerial::init(uint32 baud)
{
#ifdef __SAM3X8E__
    m_serial = &Serial2;
    m_serial->begin(baud);
#elif __RFduino__
    m_serial = &Serial;
    Serial.begin(baud);
#endif
}

boolean CASerial::read(uint8 *bufSize, uint8 *header, uint8 *data)
{
    boolean ret = CA_FALSE;
    uint8 avaliableBytes = m_serial->available();
    
    if (avaliableBytes)
    {
        if (m_size == 0)
        {
            m_serial->readBytes((char*)&m_size, 1);
            CA_ASSERT(m_size<MAX_PACKET_SIZE, "Invalid packet size");
        }
        
        if (avaliableBytes >= m_size-1)
        {
            *bufSize = m_size;
            m_serial->readBytes((char*)header, 1);
            m_serial->readBytes((char*)data, m_size-2);
            m_size = 0;
            ret = CA_TRUE;
        }
    }
     return ret;
}

void CASerial::write(uint8 *data)
{
    uint8 val;
    uint8 bufSize = data[0];  // First byte is the size

    if (bufSize >= MAX_PACKET_SIZE)
    {
        CA_ASSERT(0, "Exceeded Max packet size");
        return;
    }

    val = m_serial->write(data, bufSize);
    CA_ASSERT(val==bufSize, "Failed CASerial::writePacket");
}


