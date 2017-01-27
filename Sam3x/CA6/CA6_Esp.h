// CA6_Esp.h

#ifndef _CA6_ESP_h
#define _CA6_ESP_h

#include "CAUtility.h"

class CA6_EspClass
{
 protected:


 public:
	void init();
  void reprogramESP();
  void resetESP();

private:
  hwPortPin espGpio0pp;
  hwPortPin espResetpp;
};

extern CA6_EspClass CA6_Esp;

#endif

