#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>                         // fork of tzapu WiFiManasger library: https://github.com/Rom3oDelta7/WiFiManager
#include <FS.h>
#include <CAStd.h>
#include <EEPROM.h>
#include <CAPacket.h>
#include <CAPacketHelper.h>

#define CA_AP_PASSWORD "ca6admin"

WiFiManager gWifiManager;
WiFiServer gServer(80);
WiFiClient gClient;
CAPacketHelper gPh;

void setup (void) {
  Serial.begin(74880);    // SAM3X
  EEPROM.begin(128);      // allocates 128 bytes for wifiManager (required by the library)

  setupWiFi();
  gPh.init((HardwareSerial*)(&SerialIO), NULL);

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
  FSInfo info;
  SPIFFS.info(info);
  CA_LOG("SPIFFS - TotalBytes: %d | UsedBytes: %d\n", info.totalBytes, info.usedBytes);

  FlashMode_t ideMode = ESP.getFlashChipMode();
  Serial.printf("Flash real id:   %08X\n", ESP.getFlashChipId());
  Serial.printf("Flash real size: %u\n\n", ESP.getFlashChipRealSize());
  Serial.printf("Flash ide  size: %u\n", ESP.getFlashChipSize()); // Size programmed by ide (should match real size)
  Serial.printf("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
#endif  

}

void loop (void) {
  processHtml("Blah");
}
