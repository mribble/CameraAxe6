/* 
 SAM3X side of the serial I/O loopback test
 Alternate version where the SAM3X is the MASTER
 This verifies correct communiction from the ESPs hw UART to the Serial1 (19,18) ports on the Due
*/

#include <CAUtility.h>

#define BAUD_RATE      74880

void setup(void)
{
   Serial1.begin(BAUD_RATE);
   CAU::logInit(BAUD_RATE);
   CAU::log("SAM3X-ESP8266 Loopback STARTED...\n");
   delay(1000);
}

void loop(void)
{
   static int loopCount = 1;
   char cout;
   char cin;

   CAU::log("Loop interation: %d\n", loopCount);
//   Serial1.flush();

   for ( cout = '0'; cout <= 'Z'; cout++ ) {
      Serial1.write(cout);
      CAU::log("Out %c\n", cout);
      while (!Serial1.available()) {}
      cin = Serial1.read();
      CAU::log("In  %c\n", cin);
   }
   
   ++loopCount;
   delay(5000);
}
