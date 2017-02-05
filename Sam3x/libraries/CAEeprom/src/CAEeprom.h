#ifndef __CAEEPROM_H__
#define __CAEEPROM_H__

// This is for the Microchip 11AAxxxx family of eeproms. (example - 11AA160T)
// These chips use a single wire I/O called UNI/O serial interface bus.

#include "CAUtility.h"

#define ADDR_11AA 0xA0

// These are chip commands from the datasheet
#define UNIO_CMD_STARTHEADER 0x55
#define UNIO_CMD_READ        0x03
#define UNIO_CMD_CRRD        0x06
#define UNIO_CMD_WRITE       0x6c
#define UNIO_CMD_WREN        0x96
#define UNIO_CMD_WRDI        0x91
#define UNIO_CMD_RDSR        0x05
#define UNIO_CMD_WRSR        0x6e
#define UNIO_CMD_ERAL        0x6d
#define UNIO_CMD_SETAL       0x67


// The following are defined in the datasheet as _minimum_ times, in
// microseconds.  There is no maximum.
#define UNIO_DELAY_FUDGE 10
#define UNIO_DELAY_TSTBY  (600 + UNIO_DELAY_FUDGE)
#define UNIO_DELAY_TSS    (10  + UNIO_DELAY_FUDGE)
#define UNIO_DELAY_THDR   (5   + UNIO_DELAY_FUDGE)

// Used break bits into 4 pieces to handle Manchester encoding this is 10 us.
#define UNIO_DELAY_QUARTER_BIT 10

enum unioDevice {CA_INTERNAL=0, CA_MODULE0=1, CA_MODULE1=2, CA_MODULE2=3, CA_MODULE3=4};

enum unioReadStatus  { EEPROM_WRITE_IN_PROGRESS_FLAG = 0x01,
                       EEPROM_WRITE_ENABLE_FLAG      = 0x02,
                       EEPROM_BLOCK_PROTECT_0_FLAG   = 0x04,
                       EEPROM_BLOCK_PROTECT_1_FLAG   = 0x08,
                     };

enum unioWriteStatus { EEPROM_WRITE_PROTECTED_NONE    = 0x00, // None write protected
                       EEPROM_WRITE_PROTECTED_QUARTER = 0x04, // Upper quarter write protected
                       EEPROM_WRITE_PROTECTED_HALF    = 0x08, // Upper half write protected
                       EEPROM_WRITE_PROTECTED_ALL     = 0x0c, // All write protect
                     };

#define EEPROM_ADDR 0
#define EEPROM_MAGIC_VAL ((uint32)0xdeadc0de)
                     
class CAEeprom
{
public:
    CAEeprom(unioDevice device);
    boolean read(uint8 *buf, uint16 addr, uint16 len);
    boolean write(const uint8 *buf, uint16 addr, uint16 len);
    boolean statusRead(unioReadStatus *status);
    boolean statusWrite(unioWriteStatus status);
    boolean readModuleId(uint8 *val);
    boolean writeModuleId(uint8 val);

private:
    void    standbyPulse();
    boolean readWriteBit(boolean bit);
    boolean readBit();
    boolean sendByte(uint8 b, boolean mak);
    boolean readByte(uint8 *b, boolean mak);
    boolean sendRawBytes(const uint8 *buf, uint16 len, boolean end);
    boolean readRawBytes(uint8 *buf, uint16 len);
    void    startHeader();
    boolean writeEnable();
    boolean writeDisable();
    boolean writeStart(const uint8 *buf, uint16 addr, uint16 len);
    boolean writeWaitComplete();
    
    uint8       m_deviceAddr;
    hwPortPin   m_hwPortPin;
};

#endif //__CAEEPROM_H__
