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
  //  espGpio0pp = CAU::getAuxPin(24); // Use module0 3rd wire for ESP GPIO0/Flash
  //  espResetpp = CAU::getAuxPin(25); // Use module0 4th wire for ESP RESET
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
  SerialUSB.println(F("ESP8266 is in Flash Programing mode, type y when complete"));
  // Need to wait here until reprogramming is done to avoid packet errors and disruption of reprogram data stream
  input = ' ';
  while (input != 'y') {
    while (!SerialUSB.available());
    input = SerialUSB.read();
  }
  SerialUSB.println(F("ESP8266 Programing complete"));
  SerialUSB.println(F("ESP8266 will be reset - Please wait a few seconds for network connection"));
  CAU::digitalWrite(espResetpp, LOW);
  delay(10);
  CAU::digitalWrite(espResetpp, HIGH);
  // delay here to allow ESP to connect to a network *** may want to read from ESP serial to get past the start up messages from WiFiManager
  delay(2000);
  return;
}

void CA6_EspClass::resetESP() {
  SerialUSB.println(F("ESP8266 will be reset - Please wait a few seconds for network connection"));
  CAU::digitalWrite(espResetpp, LOW);
  delay(10);
  CAU::digitalWrite(espResetpp, HIGH);
  delay(10);
  return;
}

CA6_EspClass CA6_Esp;

