#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

// Open Questions:
// Is the port I'm using for UDP packets ok (4040)?
// Is broadcasting to 255.255.255.255 or should I be doing something else?
// Are the gateway and submask numbers ok?

uint16 gPort = 4045;
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

#define PACKET_SIZE_SIZE 2

uint16 genPacketSize(uint8 b0, uint8 b1) {
  uint16 ret = uint16(b0) + (uint16(b1)<<8);
  return ret;
}

uint8 getPacketSize(uint16 val, uint8 byteNumber) {
  if (byteNumber == 0) {
    return val && 0xFF;
  } else {
    return val >> 8;
  }
}

void loop() {
  uint16 udpSize = gUDP.parsePacket();
  uint16 serialSize = Serial.available();
  static uint16 gPacketSize = 0;
  static IPAddress gIp;
  uint8 buf[2048];

  if (udpSize > 0) {
    gIp = gUDP.remoteIP();
    udpSize = gUDP.readBytes(buf, 2048);
    Serial.write(buf, udpSize);
  }

  if (serialSize > 0) {
    
    if (gPacketSize == 0 && serialSize >= PACKET_SIZE_SIZE) {
      uint8 ibuf[PACKET_SIZE_SIZE];
      Serial.readBytes(ibuf, PACKET_SIZE_SIZE);
      gPacketSize = genPacketSize(ibuf[0], ibuf[1]);
    }
    if (gPacketSize != 0 && serialSize >= gPacketSize-PACKET_SIZE_SIZE) {
      buf[0] = getPacketSize(gPacketSize, 0);
      buf[1] = getPacketSize(gPacketSize, 1);
      Serial.readBytes(buf+PACKET_SIZE_SIZE, gPacketSize-PACKET_SIZE_SIZE);
      gUDP.beginPacket(gIp, gPort+1);
      gUDP.write(buf, gPacketSize);
      gUDP.endPacket();
      gPacketSize = 0;
    }
  }
}
