#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Open Questions:
// Is the port I'm using for UDP packets ok (4040)?
// Is broadcasting to 255.255.255.255 or should I be doing something else?
// Are the gateway and submask numbers ok?

const unsigned int gPort = 4040;
WiFiUDP gUDP;

// Different wifi connection modes.  One must be enabled.
#define SETUP_WIFI_MANAGER
//#define SETUP_AP_NETWORK
//#define SETUP_STA_NETWORK

void setup() {
  Serial.begin(9600);

#ifdef SETUP_WIFI_MANAGER
  // Use wifi manager to try and connect to to a local network
  WiFiManager wifiManager;
  //wifiManager.resetSettings();  // Reset settings for debug
  wifiManager.autoConnect("CA6_Network", "pass1234");
  Serial.printf("Connected (Wifi Manager Mode), IP address: %s\n", WiFi.localIP().toString().c_str());
#endif
#ifdef SETUP_AP_NETWORK
  boolean ret;
  
  // Generate a unique name
  uint8_t macAddr[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(macAddr);
  String macId =  String(macAddr[WL_MAC_ADDR_LENGTH - 4], HEX) +
                  String(macAddr[WL_MAC_ADDR_LENGTH - 3], HEX) +
                  String(macAddr[WL_MAC_ADDR_LENGTH - 2], HEX) +
                  String(macAddr[WL_MAC_ADDR_LENGTH - 1], HEX);
  macId.toUpperCase();
  String apNetName = "CA6_Network_" + macId;
  String apPass = "pass1234";

  ret = WiFi.softAPConfig(IPAddress(192,168,0,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));
  Serial.printf((ret) ? "Passed WiFi.softApConfig\n" : "Failed WiFi.softApConfig\n");
  ret = WiFi.softAP(apNetName.c_str(), apPass.c_str());
  Serial.printf((ret) ? "Passed WiFi.softAP\n" : "Failed WiFi.softAP\n");
  WiFi.begin();
  WiFi.mode(WIFI_AP);  // Use softAP local network ** must be after WiFi.begin due to bug with .softAP triggering both AP and STA modes
  Serial.printf("Connected (AP Mode), IP address: %s\n", WiFi.localIP().toString().c_str());
#endif
#ifdef SETUP_STA_NETWORK
  // Connect to local wifi network
  WiFi.begin(gSsid, gPassword);
  WiFi.mode(WIFI_STA);  // Use station (local wifi network)
  Serial.printf("\nConnecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.printf(".");
  }
  Serial.printf("Connected (STA Mode), IP address: %s\n", WiFi.localIP().toString().c_str());
#endif

  if(gUDP.begin(gPort) == 1) {
    Serial.printf("Connected UDP, port: %d\n", gPort);
  }
  else {
    Serial.printf("UDP connection failed!\n");
  }
}

void loop() {
  int packetSize = gUDP.parsePacket();
  char buf[256];

  if(packetSize) {
    Serial.printf("%s -- ", gUDP.remoteIP().toString().c_str());
    
    // Read the packet into packetBufffer
    int len = gUDP.read(buf, 255);
    if (len > 0) {
        buf[len] = 0; // string null termination
    }
    Serial.printf("%s\n", buf);

    // send a reply over udp
    gUDP.beginPacket(gUDP.remoteIP(), gUDP.remotePort());
    gUDP.write("Message from Esp8266");
    gUDP.endPacket();
  }
  delay(10);
}
