#include "CAEsp8266.h"
#include "CAUtility.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  CAEsp8266 - Class for handling the interface pins between SAM3X & ESP8266
//                  Initialized pins, handles putting the ESP8266 into programming mode
//                  and alto handles resetting the ESP8266
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CAEsp8266::init(uint32 baud) {
    // There are two pins connected from sam3x to esp8266.  One we call mEspGpio0 (book from flash on high and reprogram
    // from uart on low).  The other is mEspReset and that controls resetting (reset on low).
    // The initial state is boot from flash and don't reset

    // TODO - Need to update these pins on v4 pcb
    mEspGpio0 = CAU::getModulePin(0, 1); // Use module0 3rd wire for ESP GPIO0/Flash
    mEspReset = CAU::getModulePin(0, 2); // Use module0 4th wire for ESP RESET

    CAU::pinMode(mEspGpio0, OUTPUT);
    CAU::digitalWrite(mEspGpio0, HIGH);
    CAU::pinMode(mEspReset, OUTPUT);
    CAU::digitalWrite(mEspReset, HIGH);
    
    Serial1.begin(baud);
}

void CAEsp8266::reprogramESP() {
    uint8 input;
    CAU::digitalWrite(mEspReset, LOW);
    delay(20);
    CAU::digitalWrite(mEspGpio0, LOW);
    delay(20);
    CAU::digitalWrite(mEspReset, HIGH);
    delay(50);
    CAU::digitalWrite(mEspGpio0, HIGH);
    SerialUSB.print("ESP8266 is in Flash Programing mode, Upload the ESP8266 sketch now\n");
    SerialUSB.print(" - wait until upload has completed and ESP8266 has connected, then type y\n");
    SerialUSB.print(" - if the ESP8266 does not connect to a network, type y then repeat the programming\n");
    // Need to wait here until reprogramming is done to avoid packet errors and disruption of reprogram data stream
    input = ' ';
    while (input != 'y') {
        while (!SerialUSB.available());
        input = SerialUSB.read();
    }

    SerialUSB.print("ESP8266 Programing complete, back to data mode\n");
}

void CAEsp8266::resetESP() {
    uint8 input;
    SerialUSB.print("ESP8266 will be reset - When complete, type y\n");
    CAU::digitalWrite(mEspReset, LOW);
    delay(10);
    CAU::digitalWrite(mEspReset, HIGH);
    delay(10);
    input = ' ';
    while (input != 'y') {
        while (!SerialUSB.available());
        input = SerialUSB.read();
    }
    
    SerialUSB.print("ESP8266 Reset complete, back to data mode\n");
}
