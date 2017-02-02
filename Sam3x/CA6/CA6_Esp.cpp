// Class for handling the interface pins between SAM3X & ESP8266
// Initialized pins, handles putting the ESP8266 into programming mode
//   and alto handles resetting the ESP8266

#include "CA6_Esp.h"
#include "CAUtility.h"

void CA6_EspClass::init() {
  // Make sure ESP pins are HIGH right away, and GPIO0/Flash first
  // Top pair for pins on module0
  espGpio0pp = CAU::getModulePin(0, 1); // Use module0 3rd wire for ESP GPIO0/Flash
  espResetpp = CAU::getModulePin(0, 2); // Use module0 4th wire for ESP RESET
  // next pair in case you want to use AUX pins
  //  espGpio0pp = CAU::getAuxPin(24); // Aux 24 is same as Due pin 44
  //  espResetpp = CAU::getAuxPin(25); // Aux 25 is same as Due pin 45
  CAU::pinMode(espGpio0pp, OUTPUT);
  CAU::digitalWrite(espGpio0pp, HIGH);
  CAU::pinMode(espResetpp, OUTPUT);
  CAU::digitalWrite(espResetpp, HIGH);
}

void CA6_EspClass::reprogramESP() {
  byte input;
  CAU::digitalWrite(espResetpp, LOW);
  delay(20);
  CAU::digitalWrite(espGpio0pp, LOW);
  delay(20);
  CAU::digitalWrite(espResetpp, HIGH);
  delay(50);
  CAU::digitalWrite(espGpio0pp, HIGH);
  SerialUSB.println(F("ESP8266 is in Flash Programing mode, Upload the ESP8266 sketch now"));
  SerialUSB.println(F(" - wait until upload has completed and (hopefully) ESP8266 has Connected, then type y"));
  SerialUSB.println(F(" - if the ESP8266 does not reconnect to a network, type y then repeat the programming"));
  // Need to wait here until reprogramming is done to avoid packet errors and disruption of reprogram data stream
  input = ' ';
  while (input != 'y') {
    while (!SerialUSB.available());
    input = SerialUSB.read();
  }
// get rid of any remaining characters in the input buffer so they don't disrupt packet processing
  while (SerialUSB.available()) {
    input = SerialUSB.read();
  }
  SerialUSB.println(F("ESP8266 Programing complete, back to data mode"));
  delay(2000);
}

void CA6_EspClass::resetESP() {
  byte input;
  SerialUSB.println(F("ESP8266 will be reset - When complete, type y"));
  CAU::digitalWrite(espResetpp, LOW);
  delay(10);
  CAU::digitalWrite(espResetpp, HIGH);
  delay(10);
  input = ' ';
  while (input != 'y') {
    while (!SerialUSB.available());
    input = SerialUSB.read();
  }
  // get rid of any remaining characters in the input buffer so they don't disrupt packet processing
  while (SerialUSB.available()) {
    input = SerialUSB.read();
  }
  SerialUSB.println(F("ESP8266 Reset complete, back to data mode"));
}

CA6_EspClass CA6_Esp;

