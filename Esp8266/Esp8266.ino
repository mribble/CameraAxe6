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
#include <ArduinoOTA.h>

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
// Defaults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* gMainPageFilename = "/Index.html";
const char* gCamSettingsFilename = "/data/cams";
const char* gIntervalFilename = "/data/interval";
const char* gStartLocation = "/data/startLocation";

// Assumes PID_CAM_SETTINGS is 5
String gCamSettingDefaults = "5~0~0~0~0~1~0~0~0~255~0~&5~1~0~0~0~1~0~0~0~255~0~&5~2~0~0~0~1~0~0~0~255~0~&5~3~0~0~0~1~0~0~0~255~0~&5~4~0~0~0~1~0~0~0~255~0~&5~5~0~0~0~1~0~0~0~255~0~&5~6~0~0~0~1~0~0~0~255~0~&5~7~0~0~0~1~0~0~0~255~0~&";

// Assumes PID_INTERVALOMETER is 6
String gIntervalometerDefaults = "6~0~0~0~1~0~4~";

String gStartLocationDefaults = "Home Page (default)";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup - Initialization code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup (void) {
  gLed.set(CALed::RED_ON);          // Force solid red led until connection made
  Serial.begin(74880);              // Connection to SAM3X
  EEPROM.begin(128);                // allocates 128 bytes for wifiManager (required by the library)
  gPh.init((HardwareSerial*)(&SerialIO), NULL);
  if (!SPIFFS.begin()) {
    CA_INFO("Cannot open SPIFFS", "");
  }
  else {
    displaySpiffsInfo();  
  }
  initStartPage();
  setupWiFi();
  initOTA();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main loop
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop (void) {
  gLed.pollLeds();
  receivePacket();
  serviceUri();
  pollWifiMode();
  ArduinoOTA.handle();
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Starts to the selected mode if no webpage connects
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initStartPage() {
  String name;
  String str;
  readFileToString(gStartLocation, name);
  if ((name.length() == 0) || (name == gStartLocationDefaults)) {
    return; // Don't do anything if it's the default case since home screen will be available after after starting wifi
  }
  delay(100); // Plenty of time for sam3x to get ready to receive packets

  // Send camera packets to sam3x
  readFileToString(gCamSettingsFilename, str);
  if (str.length() == 0) {
    str = gCamSettingDefaults;
  }
  setAllCams(str);

  // Send intervalometer packet to sam3x
  readFileToString(gIntervalFilename, str);
  if (str.length() == 0) {
    str = gIntervalometerDefaults;
  }
  gPh.writePacketIntervalometer(str);

  // Send packet to force photo mode on the selected packet
  str = String(PID_MENU_SELECT) + "~1~" + name + "~";
  gPh.writePacketMenuSelect(str);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Starts the over the air (OTA) updates so you can update over wifi instead of an ftdi cable
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void initOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    CA_LOG("Starting OTA\n");
  });
  ArduinoOTA.onEnd([]() {
    CA_LOG("Ending OTA\n");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress OTA: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    CA_LOG("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) CA_LOG("Auth Failed\n");
    else if (error == OTA_BEGIN_ERROR) CA_LOG("Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR) CA_LOG("Connect Failed\n");
    else if (error == OTA_RECEIVE_ERROR) CA_LOG("Receive Failed\n");
    else if (error == OTA_END_ERROR) CA_LOG("End Failed\n");
  });
  ArduinoOTA.begin();
}

