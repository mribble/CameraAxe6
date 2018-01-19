////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Dreaming Robots - Copyright 2017, 2018
//
// This runs the Camera Axe 6 (CA6) software on an esp8266 module
//   The primary purposes of this software is:
//    Handle connecting a device over wifi
//    Serve webpages to a device running a browser (including html, css, and javascript(JS))
//    Send/receieve binary data packets to/from the sam3x
//    Send/receive strings to/from a remote device running the javascript
//
// 2018.1.18
//    - Initial version
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Loading you must use a recent version of esptool (https://github.com/espressif/esptool) with the following command line:
// esptool.py --port COM3 --before no_reset --after no_reset write_flash 0x0 C:\Users\xyz\AppData\Local\Temp\arduino_build_440408/Esp8266.ino.bin 0x300000 C:\Users\xyz\AppData\Local\Temp\arduino_build_440408/Esp8266.spiffs.bin

#include "lwip/tcp_impl.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/Rom3oDelta7/WiFiManager
#include <FS.h>
#include <CAStd.h>
#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CALed.h>
#include <ArduinoOTA.h>

#define CA_AP_PASSWORD "ca6admin"
#define G_LED 4
#define R_LED 5

#define MAX_DYNAMIC_MESSAGES 16
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
String gDynamicUint32s;             // Stores uint32 packet updates from sam32 headed to JS on next refresh
CALed gLed(G_LED, R_LED);           // Manages a red/green LED used to indicate connection status
uint32_t gVoltage = 0;              // Current voltage being read by sam3x

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defaults
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const char* gMainPageFilename = "/Index.html";
const char* gCamSettingsFilename = "/data/cams";
const char* gIntervalFilename = "/data/interval";
const char* gStartLocation = "/data/startLocation";

// Assumes PID_CAM_SETTINGS is 5
String gCamSettingDefaults = "5~0~1~0~0~0~200000000~0~0~255~0~&5~1~1~0~0~0~200000000~0~0~255~0~&5~2~1~0~0~0~200000000~0~0~255~0~&5~3~1~0~0~0~200000000~0~0~255~0~&5~4~1~0~0~0~200000000~0~0~255~0~&5~5~1~0~0~0~200000000~0~0~255~0~&5~6~1~0~0~0~200000000~0~0~255~0~&5~7~1~0~0~0~200000000~0~0~255~0~&";

// Assumes PID_INTERVALOMETER is 6
String gIntervalometerDefaults = "6~0~0~0~1~0~4~";

String gStartLocationDefaults = "Home Page (default)";

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup - Initialization code
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup (void) {
  gLed.set(CALed::RED_ON); // Force solid red led until connection made
  SerialIO.begin(74880); // Connection to SAM3X
  delay(10); // Wait for serial connection to sam3x
  gPh.init((HardwareSerial*)(&SerialIO), NULL);
  if (!SPIFFS.begin()) {
    CA_LOG(CA_ERROR, "Cannot open SPIFFS\n");
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
  tcpCleanup();
  //CA_LOG(CA_INFO, "memleft: %d\n", ESP.getFreeHeap()); delay(50);
  ArduinoOTA.handle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print some useful info about the SPIFFS file system
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displaySpiffsInfo() {
#ifdef CA_DEBUG_INFO
  Dir dir = SPIFFS.openDir("/");
  CA_LOG(CA_INFO, "SPIFFS directory contents:\n");
  while (dir.next()) {
    File f = dir.openFile("r");
    CA_LOG(CA_INFO, " - %s: Size: %d\n", dir.fileName().c_str(), f.size());
  }

  FSInfo info;
  SPIFFS.info(info);
  CA_LOG(CA_INFO, "SPIFFS - TotalBytes: %d | UsedBytes: %d\n", info.totalBytes, info.usedBytes);
  CA_LOG(CA_INFO, "Flash real size: %u\n", ESP.getFlashChipRealSize());
  CA_LOG(CA_INFO, "Flash ide  size: %u\n", ESP.getFlashChipSize()); // Size programmed by ide (should match real size)
  //FlashMode_t ideMode = ESP.getFlashChipMode();
  //CA_LOG(CA_INFO, "Flash ide speed: %u\n", ESP.getFlashChipSpeed());
  //CA_LOG(CA_INFO, "Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
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
    CA_LOG(CA_INFO, "Starting OTA\n");
  });
  ArduinoOTA.onEnd([]() {
    CA_LOG(CA_INFO, "Ending OTA\n");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    CA_LOG(CA_INFO, "Progress OTA: %u%%\n", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    CA_LOG(CA_INFO, "OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) CA_LOG(CA_INFO, "Auth Failed\n");
    else if (error == OTA_BEGIN_ERROR) CA_LOG(CA_INFO, "Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR) CA_LOG(CA_INFO, "Connect Failed\n");
    else if (error == OTA_RECEIVE_ERROR) CA_LOG(CA_INFO, "Receive Failed\n");
    else if (error == OTA_END_ERROR) CA_LOG(CA_INFO, "End Failed\n");
  });
  ArduinoOTA.begin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Each http requesst consumes 184 bytes.  It get added to a list and is freed after a few minutes.  But because of 
//  ajax we make a lot of http request and consume all memory and cause an exception if we don't clean up this list
//  frequently.  (I would expect this workaround eventually get added to esp8266 master.)
//  https://github.com/esp8266/Arduino/issues/1923
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void tcpCleanup()
{
  while(tcp_tw_pcbs!=NULL)
  {
    tcp_abort(tcp_tw_pcbs);
  }
}


