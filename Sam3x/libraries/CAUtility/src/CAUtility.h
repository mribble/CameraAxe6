#ifndef __CAUTILITY_H__
#define __CAUTILITY_H__

#include "CATypes.h"

enum hwPorts {PORT_A=0, PORT_B=1, PORT_C=2, PORT_D=3};
enum camPinType {FOCUS=0, SHUTTER=1};
enum hwDevice {EEPROM_PIN=0, BLE_TX_PIN=1, BLE_RX_PIN=2, BLE_RTS_PIN=3, BLE_CTS_PIN=4, LED_GREEN_PIN=5, LED_RED_PIN=6, LV_DETECT_PIN=7};

#define NO_PIN -1

// ANALOG_INPUT goes along with (INPUT, OUTPUT, INPUT_PULLUP).
#define ANALOG_INPUT 3

struct hwPortPin
{
  hwPorts port;
  uint8   pin;
  volatile RwReg *digitalReadReg;     // Cached register location for super fast digital reads
  volatile int8 analogInputChannel;   // Analog channel number 0-15 or -1 if not an analog port/pin
};

namespace CAU
{
    void logInit(uint32 baud);
    void log(char *fmt, ... );
    void pinMode(hwPortPin pp, uint8 mode);
    inline uint8 digitalRead(hwPortPin pp);
    void digitalWrite(hwPortPin pp, uint8 val);
    void initializeAnalog();
    inline uint16 analogRead(hwPortPin pp);
    hwPortPin getModulePin(uint8 module, uint8 pin);
    hwPortPin getCameraPin(uint8 cam, camPinType type);
    hwPortPin getLinkPin(camPinType type);
    hwPortPin getAuxPin(uint8 pin);
    hwPortPin getOnboardDevicePin(hwDevice device);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caDigitalRead - Reads a pin from an IO port
//   port  - The port (PORT_A, PORT_B, PORT_C, PORT_D)
//   inBit - The port's bit (0-31)
// returns - 0 or 1 depending of if the bit was set or not
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint8 digitalRead(hwPortPin pp)
{
    // PDSR = Pin data status register
    //volatile RoReg *regsPDSR[4] = {&REG_PIOA_PDSR, &REG_PIOB_PDSR, &REG_PIOC_PDSR, &REG_PIOD_PDSR};   // Slow option1
    //uint32 val = *(regsPDSR[pp.port]);                                                                // Slow option1
    uint32 val = *(pp.digitalReadReg);                                                                   // Fast option2

    CA_ASSERT((pp.digitalReadReg == &REG_PIOA_PDSR) || (pp.digitalReadReg == &REG_PIOB_PDSR) ||
              (pp.digitalReadReg == &REG_PIOC_PDSR) || (pp.digitalReadReg == &REG_PIOD_PDSR), "Error: Bad digitalReadReg");
    CA_ASSERT(pp.port <= 3, "Error: Bad port size");
    CA_ASSERT(pp.pin <= 31, "Error: Bad pin size");

    return bitRead(val, pp.pin);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CAU::analogRead - Reads an analog value from a channel
//   hwPortPin -- analogInputChannel channel number is a member of this structure
// returns - the 12-bit analog value
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint16 analogRead(hwPortPin pp)
{
  uint32 mask = (1<<pp.analogInputChannel);   // setup mask for the End of Conversion bit (0-15) for this channel

  CA_ASSERT((pp.analogInputChannel >= 0) && (pp.analogInputChannel <= 14), "Error: Bad analog channel number");
  CA_ASSERT(pp.port <= 3, "Error: Bad port size");
  CA_ASSERT(pp.pin <= 31, "Error: Bad pin size");

  while((ADC->ADC_ISR & mask)==0);   // wait for conversion
  return ADC->ADC_CDR[pp.analogInputChannel];
}

}

#endif __CAUTILITY_H__
