/*
 This utility clears the WiFi credentials stored on the ESP8266 in firmware
 Clears both locations - the one used byt he ESP8266 SDK and the EEPROM backup used by CA6
 */

#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiManager.h>

#define  EEPROM_START       0
#define  EEPROM_SIZE        128


void setup(void) {
   Serial.begin(74880);
   EEPROM.begin(EEPROM_SIZE);
}

void loop(void) {
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
   while ( true ) delay(5000);                  // delay is requried to avoid WDT reset
}
