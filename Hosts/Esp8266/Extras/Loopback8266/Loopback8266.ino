/* 
 ESP8266 side of the serial I/O loopback test
 ESP8266 is the MASTER
 This verifies correct communiction from the ESPs hw UART to the Serial1 (19,18) ports on the Due
 Diagnostic output is sent to the separate serial port on  Serial1 (GPIO2)
*/

#include <SoftwareSerial.h>

#define BAUD_RATE      74880

void setup(void)
{
   Serial.begin(BAUD_RATE);
   Serial1.begin(BAUD_RATE);
   Serial1.println("ESP8266 sender & echo STARTED...");
}

void loop(void)
{
   static int loopCount = 1;

   Serial1.printf("Loop interation: %d\n", loopCount);
   Serial1.printf("SENT\t\tRCVD\n");
   Serial1.printf("----\t\t----\n");
   //Serial.flush();

	// use read & write serial functions to minimize overhead
   for ( char cout = '0'; cout <= 'Z'; cout++ ) {
      Serial.write(cout);
      Serial1.printf("   %c\t\t", cout);
      while ( Serial.available() ) {
         char cin = Serial.read();
         Serial1.printf("%c (%02x)", cin, cin);
         if ( Serial.available() ) {
            Serial1.write(",");
         }
      }
      Serial1.write("\n");
   }
   
   ++loopCount;
   delay(5000);
}
