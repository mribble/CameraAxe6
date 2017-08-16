#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>                         // fork of tzapu WiFiManasger library: https://github.com/Rom3oDelta7/WiFiManager
#include <FS.h>
#include <CAStd.h>
#include <EEPROM.h>
//#include <CAPacket.h>
//#include <CAPacketHelper.h>

#define CA_AP_PASSWORD "ca6admin"

WiFiManager gWifiManager;
WiFiServer gServer(80);
WiFiClient gClient;

void setup (void) {
  Serial.begin(74880);    // SAM3X
  EEPROM.begin(128);      // allocates 128 bytes for wifiManager (required by the library)

  setupWiFi();

  if (!SPIFFS.begin()) {
    CA_INFO("Cannot open SPIFFS file system", "");
  }

#ifdef CA_DEBUG_INFO
  Dir dir = SPIFFS.openDir("/");
  Serial.println(F("SPIFFS directory contents:"));
  while (dir.next()) {
    Serial.print(dir.fileName());
    File f = dir.openFile("r");
    Serial.print(": Size: ");
    Serial.println(f.size());
  }
#endif  

}

void loop (void) {

  processHtml("Blah");
}
