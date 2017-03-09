/* 
 ESP8266 side of the serial I/O loopback test
 This verifies correct communiction from the ESPs hw UART to the Serial1 (19,18) ports on the Due
*/

#define BAUD_RATE      74880

void setup(void)
{
   Serial.begin(BAUD_RATE);
}

void loop(void)
{
   while ( Serial.available() ) {
      Serial.write(Serial.read());
   }
}
