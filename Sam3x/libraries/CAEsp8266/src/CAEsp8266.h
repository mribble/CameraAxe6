#ifndef __CAESP8266_H__
#define __CAESP8266_H__

#include "CAUtility.h"

class CAEsp8266
{
 public:
    void init();
    void reprogramESP();
    void resetESP();

private:
    hwPortPin mEspGpio0;  // Boot chip either from flash (high) or program it from uart (low)
    hwPortPin mEspReset;  // Reset chip on low
};

#endif //__CAESP8266_H__

