////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
//
// This runs the Camera Axe 6 (CA6) software on an esp8266 module
//   The primary purposes of this software is:
//    Handle connecting a device over wifi
//    Serve webpages to a device running a browser (including html, css, and javascript(JS))
//    Send/receieve binary data packets to/from the sam3x
//    Send/receive strings to/from a remote device running the javascript
//
// 2017.9.6
//    - Initial version
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/Rom3oDelta7/WiFiManager
#include <FS.h>
#include <CAStd.h>
#include <EEPROM.h>
#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CALed.h>

#define GREEN_PIN 4
#define RED_PIN 5

#define CA_AP_PASSWORD "ca6admin"
#define G_LED 4
#define R_LED 5

#define MAX_DYNAMIC_MESSAGES 4
struct DynamicMessages
{
  uint8_t numMessages = 0;
  uint8_t id[MAX_DYNAMIC_MESSAGES];
  String str[MAX_DYNAMIC_MESSAGES];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
WiFiManager gWifiManager;           // Helps get connected to a wifi with a headless device like the CA6
WiFiServer gServer(80);             // TCP/IP Server
WiFiClient gClient;                 // Client that connects to a specific IP address and port
CAPacketHelper gPh;                 // Helps assemble binary packets going to the sam3x
DynamicMessages gDynamicMessages;   // Stores dynamic string packet updates from sam3x headed to JS on next refresh
CALed gLed(G_LED, R_LED);           // Manages a red/green LED used to indicate connection status

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup - Initialization code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup (void) {
  gLed.set(CALed::RED_ON);          // Force solid red led until connection made
  Serial.begin(74880);              // Connection to SAM3X
  EEPROM.begin(128);                // allocates 128 bytes for wifiManager (required by the library)
  setupWiFi();
  gPh.init((HardwareSerial*)(&SerialIO), NULL);

  if (!SPIFFS.begin()) {
    CA_INFO("Cannot open SPIFFS", "");
  }
  else {
    displaySpiffsInfo();  
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop (void) {
  gLed.pollLeds();
  receivePacket();
  serviceUri();
  pollWifiMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print some useful info about the SPIFFS file system
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displaySpiffsInfo() {
#ifdef CA_DEBUG_INFO
  Dir dir = SPIFFS.openDir("/");
  CA_LOG("SPIFFS directory contents:\n");
  while (dir.next()) {
    File f = dir.openFile("r");
    CA_LOG(" - %s: Size: %d\n", dir.fileName().c_str(), f.size());
  }

  FSInfo info;
  SPIFFS.info(info);
  CA_LOG("SPIFFS - TotalBytes: %d | UsedBytes: %d\n", info.totalBytes, info.usedBytes);
  CA_LOG("Flash real size: %u\n", ESP.getFlashChipRealSize());
  CA_LOG("Flash ide  size: %u\n", ESP.getFlashChipSize()); // Size programmed by ide (should match real size)
  //FlashMode_t ideMode = ESP.getFlashChipMode();
  //CA_LOG("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
  //CA_LOG("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
#endif
}

