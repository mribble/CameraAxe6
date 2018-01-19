////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
//  CAEsp8266 - Class for handling the interface pins between SAM3X & ESP8266
//                  Initialized pins, handles putting the ESP8266 into programming mode
//                  and alto handles resetting the ESP8266
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CAESP8266_H__
#define __CAESP8266_H__

#include "CAUtility.h"

class CAEsp8266
{
 public:
    void init(uint32_t baud);
    void end();
    HardwareSerial* getSerial() {return &Serial2;};
    void reprogramESP();
    void resetESP();

private:
    hwPortPin mEspGpio0;  // Boot chip either from flash (high) or program it from uart (low)
    hwPortPin mEspReset;  // Reset chip on low
};

#endif //__CAESP8266_H__

