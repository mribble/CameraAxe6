// CaFlash
// Compiled with Arduino Software 1.8.5 (http://arduino.cc) plus the attiny module
// This site explains how to setup tiny: http://highlowtech.org/?p=1695

// Here are the steps to loading the firmware:
//   Set Tools->Board to: ATtiny24/44/84
//   Set Tools->Processor to: ATtiny44
//   Set Tools->Clock to: Internal 8 MHz
//   Set Tools->Programmer to: USBtinyISP
//   Tools->Burn Bootloader - This sets some fuses so the chip runs at 8 Mhz.
//   Upload the program

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// REVISIONS:
// 1.00 January 30, 2018 - Maurice Ribble
//      Initial version
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADC maps 0->5V with values from 0 to 255
#define MAX_CAP_VOLTAGE 150        // v = 20V*20K/(1M+20K)*255/5

// Digital/analog pins
#define PIN_BOOST           6   // PORT_A - Output. Controls current flow to boost inductor
#define PIN_GREEN           2   // PORT_A - Output. Controls green indicator led  
#define PIN_RED             3   // PORT_A - Output. Controls red indicator led
#define PIN_LED             0   // PORT_B - Output. Controls pulse to big led
#define PIN_TIP             2   // PORT_B - Digital input. Reads tip of 3.5mm jack input
#define PIN_RING            1   // PORT_B - Digital input. Reads ring of 3.5mm jack input
#define APIN_VOLTAGE        7   // PORT_A - Analog input.  Reads voltage of the big capacitor

// Fast digital read macros
#define READ_TIP()   bitRead(PINB, PIN_TIP)
#define READ_RING()  bitRead(PINB, PIN_RING)

// Fast digital write macros
#define SET_BOOST()   bitSet(PORTA, PIN_BOOST)
#define CLR_BOOST()   bitClear(PORTA, PIN_BOOST)
#define SET_GREEN()   bitSet(PORTA, PIN_GREEN)
#define CLR_GREEN()   bitClear(PORTA, PIN_GREEN)
#define SET_RED()     bitSet(PORTA, PIN_RED)
#define CLR_RED()     bitClear(PORTA, PIN_RED)
#define SET_LED()     bitSet(PORTB, PIN_LED)
#define CLR_LED()     bitClear(PORTB, PIN_LED)

// Globals
uint8_t gLowTime = 40;      // Boost converter mosfet off time in microseconds
uint8_t gHighTime = 18;     // Boost converter mosfet on time in microseconds

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setAdcParams - Sets the ADC parameters
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void setAdcParams()
{
 // Setup ADC parameters.
 // Using free running mode and only a single ADC channel.  So we just start it running and then ADCH will hold the 
 // most recent value.  MUX values must change to match APIN_VOLTAGE
 // Note: Performing an Arduino analogRead() may change some of these parameters so don't do that.

  // ADCSRB - ADC Control and Status Register B
  ADCSRB &= ~(1<<ADTS0);  // 0 Setting ADSTSR0..2 to (0,0,0) puts the ADC in freee running mode
  ADCSRB &= ~(1<<ADTS1);  // 0
  ADCSRB &= ~(1<<ADTS2);  // 0
  // bit 3 reserved can write 0
  ADCSRB |=  (1<<ADLAR);  // 1, Set the upper 8 bits are found in ADCH, so reading ADCH has 8 msb
  //ADCSRB &= ~(1<<ACME); // 0, Don't set just use default
  //ADCSRB &= ~(1<<BIN);    // 0, Set bipolar input mode to unipolar (negative inputs not allowed)

  //ADMUX, ADC Mux Seletion Register
  ADMUX |= (1<<MUX0);    // 1, adc Mux channel for ADC0(PA7)
  ADMUX |= (1<<MUX1);    // 1
  ADMUX |= (1<<MUX2);    // 1
  ADMUX &= ~(1<<MUX3);   // 0
  ADMUX &= ~(1<<MUX4);   // 0
  ADMUX &= ~(1<<MUX5);   // 0
  ADMUX &= ~(1<<REFS0);  // 0, adc reference for Vcc need (0,0)
  ADMUX &= ~(1<<REFS1);  // 0

  // ADCSRA , ADC control & status reg A:
  // ADC clock divisor
  // ADPS2  ADPS1  ADPS0  Div Factor
  // 0      0      0      2
  // 0      0      1      2          // Fastest
  // 0      1      0      4
  // 0      1      1      8
  // 1      0      0      16
  // 1      0      1      32
  // 1      1      0      64
  // 1      1      1      128        // Slowest (default)
  // Default set by wiring is 128 (16 MHz/128 = 128 Khz)
  // A conversion takes 13 adc clocks the sample rate at 128Khz/13 = 9.8Khz or about 99 us
  // We use a 8mhz system clock and the div factor will be set to 8, (adc clock = 8 mhz/8 = 1 mhz).
  // 1mhz is at the highest suggested speed of the adc clock when full resolution is not required.
  // At a 1mhz clock, the 13 required cycles will give an adc sample rate of 1 mhz/13 = 76.9 khz (13 us).
  ADCSRA |=  (1<<ADPS0);  // 1  Setting ADPS0..2 to (1,1,0) sets div factor to 8
  ADCSRA |=  (1<<ADPS1);  // 1
  ADCSRA &= ~(1<<ADPS2);  // 0
  ADCSRA &= ~(1<<ADIE) ;  // 0, ADC Interrupt Enable
  //ADCSRA &= ~(1<<ADIF); // 0, ADC Interrupt Flag, since IE is not enabled no need to this setup
  ADCSRA |=  (1<<ADATE);  // 1, ADC Auto Trigger Enable
  ADCSRA |=  (1<<ADEN);   // 1, ADC Enable
  ADCSRA |=  (1<<ADSC);   // 1, ADC Start Conversion (required for free running mode)
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// readAdc - Fast analog read
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline uint8_t readAdc() {
  return ADCH;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setPwmParams - Sets up PWM parameters for boost mosfet.  Must be called before using the pwm from PA6 (PIN_BOOST)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline void setPwmBoost() {
  //F_CPU=8000000; 8MHz
  // Because the frequency of the CPU is 8Mhz and the prescaler is set to 8 that means low and high times are
  //  are just expressed in microseconds. (gLowTime and gHighTime)

  GTCCR = (1<<TSM) | (1<<PSR10);  //Reset and pause timere while it is setup

  // COM0A1/COM0A0 clear OC1A on compare match, set OC1A at bottom
  // WGM1[0..3] Fast PWM TOP=ICR1
  TCCR1A = (1<<COM1A1) | (1<<WGM11);
  
  // Pin7 (A6) the signal driving the boost converter generates a pwm
  // WGM13, WGM12, WGM11, WGM10: clear on compare ICR1
  // CS12 | CS11 | CS10
  //    0      0      1  no prescaling
  //    0      1      0  /8 prescaling  << Currently selected
  //    0      1      1  /64 prescaling
  TCCR1B = (1<<CS11) | (1<<WGM12) | (1<<WGM13);

  ICR1 = gLowTime+gHighTime-1;
  OCR1A = gHighTime;
  GTCCR = (1<<PSR10);  //Reset timer and let it run
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enables and disables charging from the boost converter until setPwmParams is called again
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void enablePwmBoost(bool val) {
  if (val == true) {
    GTCCR |= (1<<PSR10);  //Reset timer
    pinModePortA(PIN_BOOST, OUTPUT);
  }
  else {
    pinModePortA(PIN_BOOST, INPUT);
  }
}


void pinModePortA(uint8_t pin, uint8_t mode) {
  if (mode == INPUT) {
    bitClear(DDRA, pin);
  }
  else if (mode == OUTPUT) {
    bitSet(DDRA, pin);
  }
  else if (mode == INPUT_PULLUP) {
    bitClear(DDRA, pin);
    bitSet(PORTA, pin);
  }
}

void pinModePortB(uint8_t pin, uint8_t mode) {
  if (mode == INPUT) {
    bitClear(DDRB, pin);
  }
  else if (mode == OUTPUT) {
    bitSet(DDRB, pin);
  }
  else if (mode == INPUT_PULLUP) {
    bitClear(DDRB, pin);
    bitSet(PORTB, pin);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup - Main program init (this is just how Arduino code works)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  setAdcParams();

  pinModePortA(PIN_BOOST, INPUT);
  pinModePortA(PIN_GREEN, OUTPUT);
  pinModePortA(PIN_RED, OUTPUT);
  pinModePortB(PIN_LED, OUTPUT);
  pinModePortB(PIN_TIP, INPUT);
  pinModePortB(PIN_RING, INPUT);
  pinModePortA(APIN_VOLTAGE, INPUT);
  
  CLR_LED();
  setPwmBoost();
  enablePwmBoost(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// loop - Main program loop (this is just how Arduino code works)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//#define MODEL_LIGHT 1

void loop()
{
  bool allowTrigger = true;
  bool charging = false;
  uint32_t startTime;  
  
  while(1) {
    //testDataTransfer();
    
    uint8_t voltage = readAdc();
    
    if (voltage < MAX_CAP_VOLTAGE) {
      if (charging == false) {
        charging = true;
        enablePwmBoost(true);
        CLR_GREEN();
        SET_RED();
      }
    }
    else {
      charging = false;
      enablePwmBoost(false);
      SET_GREEN();
      CLR_RED();
    }

#ifdef MODEL_LIGHT
    if (allowTrigger) {
      SET_LED();
      __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"); // 1 us
      CLR_LED();
      startTime = millis();
      allowTrigger = false;
    }
    else {
      uint32_t duration = 10;
      if ((millis() - startTime) >= duration) {
        allowTrigger = true;
      }
      CLR_GREEN();
      CLR_RED();
    }

#else
    if (allowTrigger) {
      //if (READ_TIP() == LOW) {
        SET_LED();
        for(uint8_t i=0; i<1; ++i) {
          __asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t""nop\n\t"); // 5/8 us, but loop takes 3/8 us so total is 1 us
        }
        CLR_LED();
        startTime = millis();
        allowTrigger = false;
      //}
    }
    else {
      uint32_t duration = 1000;  // Allow retrigger every 1 second
      if ((millis() - startTime) >= duration) {
        allowTrigger = true;
      }
      CLR_GREEN();
      CLR_RED();
    }
#endif
  }  //while(1)
}

void testDataTransfer() {
  uint8_t val;
  if (READ_RING() == 0) {
    if (rxCaByte(val)) {
      if (val == 203) {
        CLR_RED();
        for (uint8_t i=0; i<3; ++i) {
          SET_GREEN();
          delay(100);
          CLR_GREEN();
          delay(100);
        }
      }
    }
    else {
        CLR_GREEN();
        for (uint8_t i=0; i<3; ++i) {
          SET_RED();
          delay(100);
          CLR_RED();
          delay(100);
        }
    }
  }
}


#define CA_QUANTUM 200  // This is the base time quantium in microseconds

// This uses a simple protocal to transfer 1 byte (8 bits).  The protocal can only send data one way becuase it is
// designed to work over camera trigger ports which are basically just 2 switched on the transmitter.  Then the
// receiver has two lines connected to those switches with pullup resistors so when when the transmitter is
// low the receiver is high and when the transmitter is high the reciever is low.
// 
// This protocal transfer data over a 3.5mm jack.  There are 3 connections (tip transfers data, ring enables 
// data transfers, and base is common/ground).  Here is how data is transfered:
// 1) Ring is set high
// 2) Tip set low for 4 quantums (init period)
// 3) For each bit:
// 4)   if bit is 1: tip set high for 3 quantums, then tip set low for 1 quantums
// 5)   if bit is 0: tip set high for 1 quantums, then tip set low for 3 quantums
// 6) Tip set high for 4 quantums (end period)
// 7) Tip set low
// 8) Ring is set low
//
// Receiving packets will verify the above happens, but is designed to be tolerant of the CPU frequecy being off
// at least 30% since the internal oscillators used for the led flash aren't very good.
// Remember signals on receiving end are reversed due to pullup resistors.
// 
bool rxCaByte(uint8_t &val) {
  uint8_t v = 0;
  if (!waitRxTip(HIGH, 4)) return false;  // Wait for start of init period
  if (!waitRxTip(LOW, 8))  return false;  // Wait for end of init period
  for(uint8_t i=0; i<8; ++i) {  // For each bit
    uint8_t tHigh = waitRxTip(HIGH, 5);  // High time in quantums for the bit
    uint8_t tLow = waitRxTip(LOW, 5);    // Low time in quantums for the bit
    if (tHigh == 0 || tLow == 0) {
      return false;  // Signal not detected so we fail
    }
    else if (tHigh >= 2 && tLow <= 1) {
      v |= 1<<i;  // Bit is high
    }
    else if (tLow >= 2 && tHigh <= 1) {
      // Bit is low
    }
    else {
      return false;  // Timing expectations not met
    }
  }
  if (!waitRxTip(HIGH, 8)) return false;  // Wait for end

  // Blink green indicator 3 times to indicate success
  val = v;
  return true;
}
// Check for bit set on the tip to the requested signal.  Max wait time in maxQuantums
// Returned value is time quantums.  This value is always the floor of us/QUANTUM, exept it is rounded up to to 1
//  since zero is reserved for failure
uint8_t waitRxTip(uint8_t signal, uint8_t maxQuantums) {
  uint32_t start = micros();
  uint32_t end;

  do {
    end = micros();
    if (READ_TIP() == signal) {
      uint8_t q;
      q = (uint8_t)((end-start)/CA_QUANTUM);  // Always clamp quantiums to floor
      q = max(1, q);  // But make sure any passed quantum is at least 1 since 0 means failure to find signal
      return q;
    }
  } while (end-start < maxQuantums*CA_QUANTUM);
  return 0;
}

