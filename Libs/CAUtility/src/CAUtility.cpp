#include "CAUtility.h"

namespace CAU
{
// Register
//  volatile WoReg *regsPIO_IDR[4]  = {&REG_PIOA_IDR,  &REG_PIOB_IDR,  &REG_PIOC_IDR,  &REG_PIOD_IDR};  // IDR  = Interrupt disable register
//  volatile WoReg *regsPIO_MDDR[4] = {&REG_PIOA_MDDR, &REG_PIOB_MDDR, &REG_PIOC_MDDR, &REG_PIOD_MDDR}; // MDDR = Multi-driver disable register
//  volatile WoReg *regsPIO_MDER[4] = {&REG_PIOA_MDER, &REG_PIOB_MDER, &REG_PIOC_MDER, &REG_PIOD_MDER}; // MDER = Multi-driver enable register
//  volatile WoReg *regsPIO_OER[4]  = {&REG_PIOA_OER,  &REG_PIOB_OER,  &REG_PIOC_OER,  &REG_PIOD_OER};  // OER  = Output Enable Register
//  volatile WoReg *regsPIO_ODR[4]  = {&REG_PIOA_ODR,  &REG_PIOB_ODR,  &REG_PIOC_ODR,  &REG_PIOD_ODR};  // ODR  = Output Disable Register
//  volatile WoReg *regsPIO_PDR[4]  = {&REG_PIOA_PDR,  &REG_PIOB_PDR,  &REG_PIOC_PDR,  &REG_PIOD_PDR};  // PDR  = PIO Disable Register
//  volatile WoReg *regsPIO_PER[4]  = {&REG_PIOA_PER,  &REG_PIOB_PER,  &REG_PIOC_PER,  &REG_PIOD_PER};  // PER  = PIO Enable Register
//  volatile RoReg *regsPIO_PSR[4]  = {&REG_PIOA_PSR,  &REG_PIOB_PSR,  &REG_PIOC_PSR,  &REG_PIOD_PSR};  // PSR  = PIO Status Register
//  volatile WoReg *regsPIO_PUDR[4] = {&REG_PIOA_PUDR, &REG_PIOB_PUDR, &REG_PIOC_PUDR, &REG_PIOD_PUDR}; // PUDR = Pullup disable register
//  volatile WoReg *regsPIO_PUER[4] = {&REG_PIOA_PUER, &REG_PIOB_PUER, &REG_PIOC_PUER, &REG_PIOD_PUER}; // PUER = Pullup enable register

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// pinMode - Sets the an IO pin to the specified mode
//   port  - The port (PORT_A, PORT_B, PORT_C, PORT_D)
//   inBit - The port's bit (0-31)
//   mode  - The new mode (INPUT, OUTPUT, INPUT_PULLUP, ANALOG_INPUT)
// returns - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pinMode(hwPortPin pp, uint8_t mode)
{
    volatile WoReg *regsPIO_IDR[4]  = {&REG_PIOA_IDR,  &REG_PIOB_IDR,  &REG_PIOC_IDR,  &REG_PIOD_IDR};  // IDR  = Interrupt disable register
    volatile WoReg *regsPIO_OER[4]  = {&REG_PIOA_OER,  &REG_PIOB_OER,  &REG_PIOC_OER,  &REG_PIOD_OER};  // OER  = Output Enable Register
    volatile WoReg *regsPIO_ODR[4]  = {&REG_PIOA_ODR,  &REG_PIOB_ODR,  &REG_PIOC_ODR,  &REG_PIOD_ODR};  // ODR  = Output Disable Register
    volatile WoReg *regsPIO_PER[4]  = {&REG_PIOA_PER,  &REG_PIOB_PER,  &REG_PIOC_PER,  &REG_PIOD_PER};  // PER  = PIO Enable Register
    volatile WoReg *regsPIO_PDR[4]  = {&REG_PIOA_PDR,  &REG_PIOB_PDR,  &REG_PIOC_PDR,  &REG_PIOD_PDR};  // PDR  = PIO Disable Register
    volatile WoReg *regsPIO_PUDR[4] = {&REG_PIOA_PUDR, &REG_PIOB_PUDR, &REG_PIOC_PUDR, &REG_PIOD_PUDR}; // PUDR = Pullup disable register
    volatile WoReg *regsPIO_PUER[4] = {&REG_PIOA_PUER, &REG_PIOB_PUER, &REG_PIOC_PUER, &REG_PIOD_PUER}; // PUER = Pullup enable register
    uint8_t portPerID[4] = {11, 12, 13, 14}; // The peripheral identifiers for Port A,B,C,D
    uint32_t mask = 1 << pp.pin;
  
    CA_ASSERT(pp.port <= 3, "Error: Bad port size");
    CA_ASSERT(pp.pin <= 31, "Error: Bad pin size");
    CA_ASSERT((mode == INPUT) || (mode == OUTPUT) || (mode == INPUT_PULLUP) || (mode == ANALOG_INPUT), "Error: Bad mode");
    CA_ASSERT(INPUT==0 && OUTPUT==1 && INPUT_PULLUP==2 && ANALOG_INPUT==3, "Error in mode values");  // Check for changes since I put ANALOG_INPUT in different file from others

    if (mode == INPUT)
    {
        (*regsPIO_PER[pp.port])  = mask;             // Enable PIO for this register
        (*regsPIO_IDR[pp.port])  = mask;             // disable interrupts
        (*regsPIO_ODR[pp.port])  = mask;             // disable output
        (*regsPIO_PUDR[pp.port]) = mask;             // disable pullup
        REG_PMC_PCER0     = 1 << portPerID[pp.port]; // enable peripheral clock for this port (possible power saving features)
        
        if (pp.analogInputChannel != NO_PIN)
        {
            CA_ASSERT((pp.analogInputChannel >= 0) && (pp.analogInputChannel <= 14), "Error: Bad analog channel number");
            REG_ADC_CHDR = 1 << pp.analogInputChannel;   // Disable this analog channel
        }
    }
    else if (mode == OUTPUT)
    {
        (*regsPIO_PER[pp.port])  = mask;             // Enable PIO for this register
        (*regsPIO_IDR[pp.port])  = mask;             // disable interrupts
        (*regsPIO_OER[pp.port])  = mask;             // enable output
        (*regsPIO_PUDR[pp.port]) = mask;             // disable pullup
        REG_PMC_PCER0     = 1 << portPerID[pp.port]; // enable peripheral clock for this port (possible power saving features)

        if (pp.analogInputChannel != NO_PIN)
        {
            CA_ASSERT((pp.analogInputChannel >= 0) && (pp.analogInputChannel <= 14), "Error: Bad analog channel number");
            REG_ADC_CHDR = 1 << pp.analogInputChannel;   // Disable this analog channel
        }
    }
    else if (mode == INPUT_PULLUP)
    {
        (*regsPIO_PER[pp.port])  = mask;             // Enable PIO for this register
        (*regsPIO_IDR[pp.port])  = mask;             // disable interrupts
        (*regsPIO_ODR[pp.port])  = mask;             // disable output
        (*regsPIO_PUER[pp.port]) = mask;             // enable pullup
        REG_PMC_PCER0     = 1 << portPerID[pp.port]; // enable peripheral clock for this port (possible power saving features)

        if (pp.analogInputChannel != NO_PIN)
        {
            CA_ASSERT((pp.analogInputChannel >= 0) && (pp.analogInputChannel <= 14), "Error: Bad analog channel number");
            REG_ADC_CHDR = 1 << pp.analogInputChannel;   // Disable this analog channel
        }
    }
    else //(mode == ANALOG_INPUT)
    {
        CA_ASSERT((pp.analogInputChannel >= 0) && (pp.analogInputChannel <= 14), "Error: Bad analog channel number");
        (*regsPIO_PDR[pp.port])  = mask;             // Disable PIO for this register
        (*regsPIO_IDR[pp.port])  = mask;             // disable interrupts
        (*regsPIO_ODR[pp.port])  = mask;             // disable output
        (*regsPIO_PUDR[pp.port]) = mask;             // disable pullup
        REG_ADC_CHER = 1 << pp.analogInputChannel;   // Enable this analog channel
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// digitalWrite - Writes a single bit from an IO port
//   pp  - The port (PORT_A, PORT_B, PORT_C, PORT_D) and pin (0-31) in that port to set
//   val   - The value being written (LOW or HIGH)
// returns - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void digitalWrite(hwPortPin pp, uint8_t val)
{
    // SODR = Set output data register
    // CODR = Clear output data register
    volatile RwReg *regs[8] = {&REG_PIOA_CODR, &REG_PIOB_CODR, &REG_PIOC_CODR, &REG_PIOD_CODR,
                               &REG_PIOA_SODR, &REG_PIOB_SODR, &REG_PIOC_SODR, &REG_PIOD_SODR};

    CA_ASSERT(pp.port <= 3, "Error: Bad port size");
    CA_ASSERT(pp.pin <= 31, "Error: Bad pin size");
    CA_ASSERT(val == LOW || val == HIGH, "Error: bad val");

    // We could make this faster if needed by caching (math on index slow on sam3x)
    *(regs[(val*4)+pp.port]) = 1<<pp.pin;  
}


//------------------------------------------------------------------
//  initializeAnalog -- call once at the beginning to disable prior channels and set up registers
void initializeAnalog()
{
    REG_PMC_PCER1 = 1 << 5; // Enable ADC Peripheral clock, bit 5 in PMC_PCER1
    REG_ADC_CHDR = 0x0000FFFF;  // Disable all analog channels to start, bits 0-15
    REG_ADC_SEQR1 = 0x0;    // Zero out the User Sequence Registers
    REG_ADC_SEQR2 = 0x0;    // Zero out the User Sequence Registers
    //--------------------------------------
    //  ADC mode register - settings
    //    USEQ - bit 31 = 0 - User channel Sequence not used
    //    TRANSFER=0 bits 28-29 (means 3 clock periods for Transfer)
    //    TRACKTIM=0 bits 24-27(means 1 clock period plus the conversion time for the previous sample)
    //    ANACH =0 bit 23 - means all channels have the same settings
    //    SETTLING=0 bits 20-21 - means 3 clock periods
    //    STARTUP=0 bits 16-19 - means 0 clock periods, not needed in Freerun mode
    //    PRESCAL=0 bits 8-15 - means ADCClock=MCK/2
    //    FREERUN=1 bit 7 - means ON - Freerun mode
    //    FWUP=0 bit 6 - meand OFF - not needed for Freerun mode
    //    SLEEP=0 bit 5(normal)
    //    LOWRES=0 bit 4(12-bit resolution)
    //    TRGSEL = 0 bits 1-3
    //    TRIGEN=0 bit 0 - means disabled No Hardware Triggers
    ADC->ADC_MR = 0x00000080;
    ADC->ADC_CR=2; // ADC Control reg bit 1 = START
    //--------------------------------------
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getModulePin - Gets the hw port and pin for a module port
//   module - The module port number
//   pin    - The digital pin on the module
// returns  - Hw port and pin number along with a ptr for digital reads and analog channel if available
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hwPortPin getModulePin(uint8_t module, uint8_t pin)
{
    hwPortPin ret;
    CA_ASSERT( module <= 3, "Error: Invalid module value");
    CA_ASSERT(pin <= 5, "Error: Invalid module pin value");
    uint8_t index = module*6 + pin;

    switch(index)
    {
        case 0:  ret.port=PORT_A;  ret.pin=22;  ret.analogInputChannel=4;       break;  // MOD0_AN1
        case 1:  ret.port=PORT_B;  ret.pin=13;  ret.analogInputChannel=9;       break;  // MOD0_SCL
        case 2:  ret.port=PORT_B;  ret.pin=12;  ret.analogInputChannel=8;       break;  // MOD0_SDA
        case 3:  ret.port=PORT_C;  ret.pin=9;   ret.analogInputChannel=NO_PIN;  break;  // MOD0_ID
        case 4:  ret.port=PORT_A;  ret.pin=10;  ret.analogInputChannel=NO_PIN;  break;  // MOD0_RX
        case 5:  ret.port=PORT_A;  ret.pin=11;  ret.analogInputChannel=NO_PIN;  break;  // MOD0_TX
        case 6:  ret.port=PORT_A;  ret.pin=23;  ret.analogInputChannel=5;       break;  // MOD1_AN1
        case 7:  ret.port=PORT_A;  ret.pin=18;  ret.analogInputChannel=NO_PIN;  break;  // MOD1_SCL
        case 8:  ret.port=PORT_A;  ret.pin=17;  ret.analogInputChannel=NO_PIN;  break;  // MOD1_SDA
        case 9:  ret.port=PORT_C;  ret.pin=8;   ret.analogInputChannel=NO_PIN;  break;  // MOD1_ID
        case 10: ret.port=PORT_D;  ret.pin=5;   ret.analogInputChannel=NO_PIN;  break;  // MOD1_RX
        case 11: ret.port=PORT_D;  ret.pin=4;   ret.analogInputChannel=NO_PIN;  break;  // MOD1_TX
        case 12: ret.port=PORT_A;  ret.pin=24;  ret.analogInputChannel=6;       break;  // MOD2_AN1
        case 13: ret.port=PORT_A;  ret.pin=4;   ret.analogInputChannel=2;       break;  // MOD2_AN2
        case 14: ret.port=PORT_A;  ret.pin=3;   ret.analogInputChannel=1;       break;  // MOD2_AN3
        case 15: ret.port=PORT_C;  ret.pin=7;   ret.analogInputChannel=NO_PIN;  break;  // MOD2_ID
        case 16: ret.port=PORT_A;  ret.pin=2;   ret.analogInputChannel=0;       break;  // MOD2_AN4
        case 17: ret.port=PORT_B;  ret.pin=17;  ret.analogInputChannel=10;      break;  // MOD2_AN5
        case 18: ret.port=PORT_A;  ret.pin=16;  ret.analogInputChannel=7;       break;  // MOD3_AN1
        case 19: ret.port=PORT_A;  ret.pin=26;  ret.analogInputChannel=NO_PIN;  break;  // MOD3_MOSI
        case 20: ret.port=PORT_A;  ret.pin=25;  ret.analogInputChannel=NO_PIN;  break;  // MOD3_MISO
        case 21: ret.port=PORT_C;  ret.pin=6;   ret.analogInputChannel=NO_PIN;  break;  // MOD3_ID
        case 22: ret.port=PORT_A;  ret.pin=27;  ret.analogInputChannel=NO_PIN;  break;  // MOD3_SCLK
        case 23: ret.port=PORT_A;  ret.pin=28;  ret.analogInputChannel=NO_PIN;  break;  // MOD3_CS
        default: CA_ASSERT(0, "Error: Invalid switch for ModulePortPin");
    }

    // Cache register location to speed up digital reads.
    volatile RoReg *regsPDSR[4] = {&REG_PIOA_PDSR, &REG_PIOB_PDSR, &REG_PIOC_PDSR, &REG_PIOD_PDSR};
    ret.digitalReadReg = regsPDSR[ret.port];
    
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getCameraPin - Gets the hw port and pin for a camera port
//   cam - The camera port number
//   type - Choose either the shutter or focus pin
// returns  - Hw port and pin number
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hwPortPin getCameraPin(uint8_t cam, camPinType type)
{
    hwPortPin ret;
    CA_ASSERT( cam <= 7, "Error: Invalid camera value");
    CA_ASSERT((type == FOCUS) || (type == SHUTTER), "Error: Invalid camera pin type");
    uint8_t index = cam*2 + (uint8_t)type;

    switch(index)
    {
        case 0:  ret.port=PORT_B;  ret.pin=24;  ret.analogInputChannel=NO_PIN;  break;  // Camera0, Focus
        case 1:  ret.port=PORT_B;  ret.pin=25;  ret.analogInputChannel=NO_PIN;  break;  // Camera0, Shutter
        case 2:  ret.port=PORT_B;  ret.pin=26;  ret.analogInputChannel=NO_PIN;  break;  // Camera1, Focus
        case 3:  ret.port=PORT_D;  ret.pin=0;   ret.analogInputChannel=NO_PIN;  break;  // Camera1, Shutter
        case 4:  ret.port=PORT_D;  ret.pin=1;   ret.analogInputChannel=NO_PIN;  break;  // Camera2, Focus
        case 5:  ret.port=PORT_D;  ret.pin=2;   ret.analogInputChannel=NO_PIN;  break;  // Camera2, Shutter
        case 6:  ret.port=PORT_D;  ret.pin=3;   ret.analogInputChannel=NO_PIN;  break;  // Camera3, Focus
        case 7:  ret.port=PORT_D;  ret.pin=6;   ret.analogInputChannel=NO_PIN;  break;  // Camera3, Shutter
        case 8:  ret.port=PORT_D;  ret.pin=7;   ret.analogInputChannel=NO_PIN;  break;  // Camera4, Focus
        case 9:  ret.port=PORT_D;  ret.pin=8;   ret.analogInputChannel=NO_PIN;  break;  // Camera4, Shutter
        case 10: ret.port=PORT_D;  ret.pin=9;   ret.analogInputChannel=NO_PIN;  break;  // Camera5, Focus
        case 11: ret.port=PORT_A;  ret.pin=0;   ret.analogInputChannel=NO_PIN;  break;  // Camera5, Shutter
        case 12: ret.port=PORT_A;  ret.pin=1;   ret.analogInputChannel=NO_PIN;  break;  // Camera6, Focus
        case 13: ret.port=PORT_A;  ret.pin=5;   ret.analogInputChannel=NO_PIN;  break;  // Camera6, Shutter
        case 14: ret.port=PORT_A;  ret.pin=7;   ret.analogInputChannel=NO_PIN;  break;  // Camera7, Focus
        case 15: ret.port=PORT_D;  ret.pin=10;  ret.analogInputChannel=NO_PIN;  break;  // Camera7, Shutter
        default: CA_ASSERT(0, "Error: Invalid switch for getCameraPin");
    }

    // Cache register location to speed up digital reads, these shouldn't be read because Camera ports are always outputs
    ret.digitalReadReg = (RwReg*)NO_PIN;
    
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getAuxPortPin - Gets the hw port and pin for the aux port
//   pin    - The digital pin on the aux port
// returns  - Hw port and pin number
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hwPortPin getAuxPin(uint8_t pin)
{
    hwPortPin ret;
    CA_ASSERT(pin <= 47, "Error: Invalid aux pin value");

    switch(pin)
    {
        case 0:  ret.port=PORT_A;  ret.pin=9;   ret.analogInputChannel=NO_PIN;  break;  //AUX_TX
        case 1:  ret.port=PORT_A;  ret.pin=8;   ret.analogInputChannel=NO_PIN;  break;  //AUX_RX
        case 2:  ret.port=PORT_B;  ret.pin=23;  ret.analogInputChannel=NO_PIN;  break;
        case 3:  ret.port=PORT_B;  ret.pin=22;  ret.analogInputChannel=NO_PIN;  break;
        case 4:  ret.port=PORT_B;  ret.pin=14;  ret.analogInputChannel=NO_PIN;  break;
        case 5:  ret.port=PORT_C;  ret.pin=28;  ret.analogInputChannel=NO_PIN;  break;
        case 6:  ret.port=PORT_C;  ret.pin=27;  ret.analogInputChannel=NO_PIN;  break;
        case 7:  ret.port=PORT_C;  ret.pin=26;  ret.analogInputChannel=NO_PIN;  break;
        case 8:  ret.port=PORT_C;  ret.pin=25;  ret.analogInputChannel=NO_PIN;  break;
        case 9:  ret.port=PORT_C;  ret.pin=24;  ret.analogInputChannel=NO_PIN;  break;
        case 10: ret.port=PORT_C;  ret.pin=23;  ret.analogInputChannel=NO_PIN;  break;
        case 11: ret.port=PORT_C;  ret.pin=22;  ret.analogInputChannel=NO_PIN;  break;
        case 12: ret.port=PORT_C;  ret.pin=21;  ret.analogInputChannel=NO_PIN;  break;
        case 13: ret.port=PORT_C;  ret.pin=20;  ret.analogInputChannel=NO_PIN;  break;
        case 14: ret.port=PORT_B;  ret.pin=10;  ret.analogInputChannel=NO_PIN;  break;
        case 15: ret.port=PORT_B;  ret.pin=9;   ret.analogInputChannel=NO_PIN;  break;
        case 16: ret.port=PORT_B;  ret.pin=8;   ret.analogInputChannel=NO_PIN;  break;
        case 17: ret.port=PORT_B;  ret.pin=7;   ret.analogInputChannel=NO_PIN;  break;
        case 18: ret.port=PORT_B;  ret.pin=6;   ret.analogInputChannel=NO_PIN;  break;
        case 19: ret.port=PORT_B;  ret.pin=5;   ret.analogInputChannel=NO_PIN;  break;
        case 20: ret.port=PORT_B;  ret.pin=4;   ret.analogInputChannel=NO_PIN;  break;
        case 21: ret.port=PORT_B;  ret.pin=3;   ret.analogInputChannel=NO_PIN;  break;
        case 22: ret.port=PORT_C;  ret.pin=10;  ret.analogInputChannel=NO_PIN;  break;
        case 23: ret.port=PORT_C;  ret.pin=4;   ret.analogInputChannel=NO_PIN;  break;
        case 24: ret.port=PORT_B;  ret.pin=2;   ret.analogInputChannel=NO_PIN;  break;
        case 25: ret.port=PORT_B;  ret.pin=1;   ret.analogInputChannel=NO_PIN;  break;
        case 26: ret.port=PORT_B;  ret.pin=0;   ret.analogInputChannel=NO_PIN;  break;
        case 27: ret.port=PORT_A;  ret.pin=29;  ret.analogInputChannel=NO_PIN;  break;
        case 28: ret.port=PORT_C;  ret.pin=30;  ret.analogInputChannel=NO_PIN;  break;
        case 29: ret.port=PORT_C;  ret.pin=29;  ret.analogInputChannel=NO_PIN;  break;
        case 30: ret.port=PORT_C;  ret.pin=19;  ret.analogInputChannel=NO_PIN;  break;
        case 31: ret.port=PORT_C;  ret.pin=18;  ret.analogInputChannel=NO_PIN;  break;
        case 32: ret.port=PORT_C;  ret.pin=17;  ret.analogInputChannel=NO_PIN;  break;
        case 33: ret.port=PORT_C;  ret.pin=16;  ret.analogInputChannel=NO_PIN;  break;
        case 34: ret.port=PORT_C;  ret.pin=15;  ret.analogInputChannel=NO_PIN;  break;
        case 35: ret.port=PORT_C;  ret.pin=14;  ret.analogInputChannel=NO_PIN;  break;
        case 36: ret.port=PORT_C;  ret.pin=13;  ret.analogInputChannel=NO_PIN;  break;
        case 37: ret.port=PORT_C;  ret.pin=12;  ret.analogInputChannel=NO_PIN;  break;
        case 38: ret.port=PORT_C;  ret.pin=11;  ret.analogInputChannel=NO_PIN;  break;
        case 39: ret.port=PORT_B;  ret.pin=21;  ret.analogInputChannel=14;      break;
        case 40: ret.port=PORT_B;  ret.pin=20;  ret.analogInputChannel=13;      break;
        case 41: ret.port=PORT_B;  ret.pin=19;  ret.analogInputChannel=12;      break;
        case 42: ret.port=PORT_B;  ret.pin=18;  ret.analogInputChannel=11;      break;
        case 43: ret.port=PORT_B;  ret.pin=16;  ret.analogInputChannel=NO_PIN;  break;
        case 44: ret.port=PORT_B;  ret.pin=15;  ret.analogInputChannel=NO_PIN;  break;
        case 45: ret.port=PORT_A;  ret.pin=20;  ret.analogInputChannel=NO_PIN;  break;
        case 46: ret.port=PORT_A;  ret.pin=19;  ret.analogInputChannel=NO_PIN;  break;
        case 47: ret.port=PORT_B;  ret.pin=27;  ret.analogInputChannel=NO_PIN;  break;
        default:  CA_ASSERT(0, "Error: Invalid switch for AuxPortPin");
    }

    // Cache register location to speed up digital reads.
    volatile RoReg *regsPDSR[4] = {&REG_PIOA_PDSR, &REG_PIOB_PDSR, &REG_PIOC_PDSR, &REG_PIOD_PDSR};
    ret.digitalReadReg = regsPDSR[ret.port];
    
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// getOnboardDevicePin - Gets the hw port and pin for the onboard devices
//   device    - The onboard device pin
// returns  - Hw port and pin number
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
hwPortPin getOnboardDevicePin(hwDevice device)
{
    hwPortPin ret;
    
    switch(device)
    {
        case EEPROM_PIN:    ret.port=PORT_A; ret.pin=21;   ret.analogInputChannel=NO_PIN;   break;
        case LV_DETECT_PIN: ret.port=PORT_A; ret.pin=6;    ret.analogInputChannel=3;        break;
        case CC_PROG:       ret.port=PORT_C; ret.pin=5;    ret.analogInputChannel=NO_PIN;   break;
        case CC_RESET:      ret.port=PORT_C; ret.pin=3;    ret.analogInputChannel=NO_PIN;   break;
        case CC_CTS:        ret.port=PORT_A; ret.pin=15;   ret.analogInputChannel=NO_PIN;   break;
        case CC_RTS:        ret.port=PORT_A; ret.pin=14;   ret.analogInputChannel=NO_PIN;   break;
        case CC_TX:         ret.port=PORT_A; ret.pin=13;   ret.analogInputChannel=NO_PIN;   break;
        case CC_RX:         ret.port=PORT_A; ret.pin=12;   ret.analogInputChannel=NO_PIN;   break;
        case CC_EXT0:       ret.port=PORT_C; ret.pin=2;    ret.analogInputChannel=NO_PIN;   break;
        case CC_EXT1:       ret.port=PORT_C; ret.pin=1;    ret.analogInputChannel=NO_PIN;   break;
        default:  CA_ASSERT(0, "Error: Invalid switch for getOnboardDevicePin");
    }

    // Cache register location to speed up digital reads.
    volatile RoReg *regsPDSR[4] = {&REG_PIOA_PDSR, &REG_PIOB_PDSR, &REG_PIOC_PDSR, &REG_PIOD_PDSR};
    ret.digitalReadReg = regsPDSR[ret.port];
    return ret;
}

}