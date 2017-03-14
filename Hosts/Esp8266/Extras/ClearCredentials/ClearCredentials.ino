/*
 This utility clears the WiFi credentials stored on the ESP8266 in firmware
 Clears both locations - the one used byt he ESP8266 SDK and the EEPROM backup used by CA6
 */

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiManager.h>

#define  EEPROM_START       0           // per WiFiManager documentation (readme.md)
#define  EEPROM_SIZE        128


void setup(void) {
   Serial.begin(74880);
   EEPROM.begin(EEPROM_SIZE);

   Serial.print(F("\nEnter 'c' to clear saved WiFi credentials: "));
}

void loop(void) {
   char buffer[65];                     // hw serial buffer is 64 bytes

   //Serial.flush();                     // no longer flushes remaining input :-(
   if ( Serial.readBytesUntil('\n', buffer, 64) ) {
      if ( buffer[0] == 'c' ) {
         Serial.println(F("\n\nClearing all saved WiFi credentials...\n"));

         WiFi.disconnect(true);               // clears SDK-stored SSID and password
         Serial.println(F("(1)SDK credentials cleared"));

         // erase saved EEPROM credentials
         if ( EEPROM.read(EEPROM_START) == EEPROM_KEY ) {
            for ( int i = 0; i < EEPROM_SIZE; i++ ) {
               EEPROM.write(i, 0);
            }
            EEPROM.commit();
            Serial.println(F("(2)EEPROM credentials cleared"));
         } else {
            Serial.println(F("(2)No saved EEPROM credentials"));
         }
         // wait here
         while ( true ) delay(1000);                      // need delay to avoid WDT reset
      } else {
         Serial.print(F("\nEnter 'c' to clear saved WiFi credentials: "));
      }
   }
}
