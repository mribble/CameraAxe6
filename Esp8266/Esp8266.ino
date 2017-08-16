#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>                         // fork of tzapu WiFiManasger library: https://github.com/Rom3oDelta7/WiFiManager
#include <FS.h>
#include <CAStd.h>
#include <EEPROM.h>
//#include <CAPacket.h>
//#include <CAPacketHelper.h>

#define AP_WORKAROUND                            // disable this define to eliminate the function to display the host IP address as an SSID

#define CA_AP_PASSWORD "ca6admin"

WiFiManager gWifiManager;
WiFiServer gServer(80);
WiFiClient gClient;

// Create and return a unique WiFi SSID using the ESP8266 WiFi MAC address
// Form the SSID as an IP address so the user knows what address to connect to when in AP mode just in case
// (Even though the config page typically comes up automatically without the user having to enter a URL)
String createUniqueSSID (void) {
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  String uSSID;

  WiFi.softAPmacAddress(mac);
  uSSID = String("CA6_AP_") + String ("10.") + String(mac[WL_MAC_ADDR_LENGTH - 3]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 2]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 1]);
  CA_INFO("Derived SSID", uSSID);
  return uSSID;
}

// return a unique class A private IP address using the ESP8266 WiFi MAC address
// we use class A private addresses to have a large potential address space to avoid conflicts
IPAddress createUniqueIP (void) {
  uint8_t   mac[WL_MAC_ADDR_LENGTH];
  IPAddress result;

  WiFi.softAPmacAddress(mac);
  result[0] = 10;
  result[1] = mac[WL_MAC_ADDR_LENGTH - 3];
  result[2] = mac[WL_MAC_ADDR_LENGTH - 2];
  result[3] = mac[WL_MAC_ADDR_LENGTH - 1];
  CA_INFO(F("Derived IP"), result);
  return result;
}

void printFile(const char *fileName) {
  const uint16_t bufSize = 1024;
  static uint8_t buf[bufSize];
  int16_t sz;
  File f = SPIFFS.open(fileName, "r");

  if (!f) {
    if (!f.available()) {
      f.close();
    }
    CA_INFO("Failed to load file: ", fileName);
    return;
  }

  sz = f.size();
  do {
    if (sz > bufSize) {
      f.read(buf, bufSize);
      gClient.write((uint8_t*)buf, bufSize);\
      sz -= bufSize;
    }
    else {
      f.read(buf, sz);
      gClient.write((uint8_t*)buf, sz);
      sz = 0;
    }
  } while (sz);

  f.close();
}

void setupWiFi ( void ) {
  IPAddress myIPAddress = createUniqueIP();

  // First check that there are WiFi networks to possibly connect to
  int netCount = WiFi.scanNetworks();
  bool connectToAP = false;

  if ( netCount > 0 ) {
    // try to connect (saved credentials or manual entry if not) and default to AP mode if this fails
    WiFiManager wifiManager;
#ifdef CA_DEBUG_INFO
    wifiManager.setDebugOutput(true);
#else
    wifiManager.setDebugOutput(false);
#endif

    CA_INFO("Network scan count: ", netCount);

    wifiManager.setBreakAfterConfig(true);                         // undocumented function to return if config unsuccessful
    wifiManager.setSaveCredentialsInEEPROM(true);                  // [Local mod] forces credentials to be saved in EEPROM also
    wifiManager.setExitButtonLabel("Standalone Mode");             // [Local mod] sets the label on the exit button to clarify the meaning of exiting from the portal

    //wifiManager.setAPStaticIPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));    // use native WiFi class call below instead
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));                    // Force a static ip
    WiFi.setAutoConnect(true);

    if ( wifiManager.autoConnect(createUniqueSSID().c_str(), CA_AP_PASSWORD) ) {
      // we are connected as a client and the ESP is in STA mode Ref: https://github.com/esp8266/Arduino/issues/2352
      CA_INFO("Connected(STA mode) local WiFi ip:)", WiFi.localIP().toString());

      // We also need to know the address of the client as it will be running our HTTP server
      // Switch to mixed mode and broadcast the local IP address in the AP SSID name.
      // The user can thus find the local address by looking at the scanned SSIDs on their device
      WiFi.mode(WIFI_AP_STA);
      String ssid = String("CA6_STA_") + WiFi.localIP().toString();
      CA_INFO(F("Local IP as SSID"), ssid);
      WiFi.softAP(ssid.c_str());
      WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));
      //WiFi.reconnect();  // supposedly required, but does not work if this is called
    } else {
      // We get here if the credentials on the setup page are incorrect, blank, or the "Exit" button was used
      CA_INFO("Did not connect to local WiFi", "");
      connectToAP = true;
    }
  } else {
    CA_INFO("No WiFI networks", "");
    connectToAP = true;
  }

  if (connectToAP) {
    // use AP mode   - WiFiManager leaves the ESP in AP+STA mode at this point
    CA_INFO("Using AP Mode", myIPAddress.toString());
    WiFi.softAP(createUniqueSSID().c_str(), CA_AP_PASSWORD);
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));
    WiFi.mode(WIFI_AP);
  }

  // start the server & init the SPIFFS file system
  gServer.begin();
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



void parseUri(String &uri, const char* title) {
  if (uri.indexOf("GET /updateAll ") != -1) {
    // Reload dynamic data
    static int val = 0;
    ++val;
    gClient.println(val);
  } else if (uri.indexOf("GET /button0 ") != -1) {
    // Button pressed
        
  } else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    // Initial page load
    sendHtml(title);
  } else if (uri.indexOf("GET /favicon.ico") != -1) {
    // ignore this case
  } else {
    CA_INFO("ERROR - Unknown URI - ", uri);
  }
}

void processHtml(const char* title) {
  gClient = gServer.available();
  if (!gClient || !gClient.connected()) {
    return;
  }

  String uri = gClient.readStringUntil('\r');
  gClient.flush();
  CA_INFO("URI: ", uri.c_str());
  parseUri(uri, title);

  gClient.stop();
}

void sendHtml(const char* title) {
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  gClient.println("<!DOCTYPE HTML> <HTML> <HEAD> <TITLE>");
  gClient.println(title);
  gClient.println("</TITLE> <meta charset=\"UTF-8\">");
  printFile("/css.html");

  printFile("/script.html");
 
  gClient.println("</HEAD>");
  gClient.println("<BODY>");
  gClient.println("<H1 id=\"title\"></H1>");
  gClient.println("<SCRIPT>");
  printFile("/testMenu.html");
  gClient.println("</SCRIPT>");
  gClient.println("</BODY>");
  gClient.println("</HTML>");
}

void setup (void) {
  Serial.begin(74880);    // SAM3X
  EEPROM.begin(128);      // allocates 128 bytes for wifiManager (required by the library)

  setupWiFi();
}

void loop (void) {

  processHtml("Blah");
}
