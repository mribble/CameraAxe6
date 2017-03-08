/* 
 ESP8266 side of the serial I/O loopback test
 This verifies correct communiction from the ESPs hw UART to the Serial1 (19,18) ports on the Due
 Diagnostic output is sent to the separate serial port on (14,12)
*/

#include <SoftwareSerial.h>

#define BAUD_RATE      74880
#define DEBUG_TX       12
#define DEBUG_RX       14
SoftwareSerial SerialDebug(DEBUG_RX, DEBUG_TX, false, 256);

void setup(void)
{
   Serial.begin(BAUD_RATE);
   SerialDebug.begin(BAUD_RATE);
   SerialDebug.println("ESP8266 sender & echo STARTED...");
}

void loop(void)
{
   static int loopCount = 1;

   SerialDebug.printf("Loop interation: %d\n", loopCount);
   SerialDebug.printf("SENT\t\tRCVD\n");
   SerialDebug.printf("----\t\t----\n");
   Serial.flush();

   for ( char cout = '0'; cout <= 'Z'; cout++ ) {
      Serial.write(cout);
      SerialDebug.printf("   %c\t\t", cout);
      while ( Serial.available() ) {
         char cin = Serial.read();
         SerialDebug.printf("%c (%02x)", cin, cin);
         if ( Serial.available() ) {
            SerialDebug.write(",");
         }
      }
      SerialDebug.write("\n");
   }
   
   ++loopCount;
   delay(5000);
}
