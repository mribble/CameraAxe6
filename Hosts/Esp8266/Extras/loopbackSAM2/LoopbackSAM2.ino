/* 
 Alternate loopback test
  ESP8266 is the SLAVE
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
