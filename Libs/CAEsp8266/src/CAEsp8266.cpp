////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
//  CAEsp8266 - Class for handling the interface pins between SAM3X & ESP8266
//                  Initialized pins, handles putting the ESP8266 into programming mode
//                  and alto handles resetting the ESP8266
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CAEsp8266.h"
#include "CAUtility.h"

void CAEsp8266::init(uint32_t baud) {
    // There are two pins connected from sam3x to esp8266.  One we call mEspGpio0 (book from flash on high and reprogram
    // from uart on low).  The other is mEspReset and that controls resetting (reset on low).
    // The initial state is boot from flash and don't reset

    mEspGpio0 = CAU::getOnboardDevicePin(CC_PROG);
    mEspReset = CAU::getOnboardDevicePin(CC_RESET);

    CAU::pinMode(mEspGpio0, OUTPUT);
    CAU::digitalWrite(mEspGpio0, HIGH);
    CAU::pinMode(mEspReset, OUTPUT);
    CAU::digitalWrite(mEspReset, HIGH);
    
    getSerial()->begin(baud);
}

void CAEsp8266::end() {
    getSerial()->end();
}

void CAEsp8266::reprogramESP() {
    CAU::digitalWrite(mEspReset, LOW);
    delay(20);
    CAU::digitalWrite(mEspGpio0, LOW);
    delay(20);
    CAU::digitalWrite(mEspReset, HIGH);
    delay(50);
    CAU::digitalWrite(mEspGpio0, HIGH);
}

void CAEsp8266::resetESP() {
    CAU::digitalWrite(mEspReset, LOW);
    delay(10);
    CAU::digitalWrite(mEspReset, HIGH);
    delay(10);
}
