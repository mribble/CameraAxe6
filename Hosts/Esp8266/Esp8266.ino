#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <WiFiManager.h>         // fork of tzapu WiFiManasger library: https://github.com/Rom3oDelta7/WiFiManager
#include <NetDiscovery.h>        // https://github.com/Rom3oDelta7/NetDiscovery
#include <ArduinoOTA.h>	
#include <SimpleTimer.h>
#include <EEPROM.h>

#define AP_WORKAROUND            // disable this define to eliminate the function to display the host IP address as an SSID

//#define DEBUG 3                  // define as minimum desired debug level, or comment out to disable debug statements

#ifdef DEBUG

#define SERIAL_DEBUG              // use separate serial line for debug messages as the hw serial is connected to the SAM3x - comment out for Serial I/O

#ifdef SERIAL_DEBUG

#define DEBUG_TX      12
#define DEBUG_RX      14
SoftwareSerial SerialDebug(DEBUG_RX, DEBUG_TX, false, 256);
#define SerialIO      SerialDebug

#else // SERIAL_DEBUG

#define SerialIO      Serial

#endif // SERIAL_DEBUG

#define DEBUG_MSG(L, H, M)	       if ((L) <= DEBUG) {SerialIO.print("DEBUG> "); SerialIO.print(H); SerialIO.print(": "); SerialIO.println(M);}
#else // DEBUG
	
#define SerialIO      Serial
#define DEBUG_MSG(...) ;
#endif // DEBUG



// auto-discovery globals
#define MCAST_PORT          7247
#define MCAST_ADDRESS       239, 12, 17, 87
#define CA6_ANNOUNCE_ID     "CA6ANC"                      // announcement packet ID
#define AD_ANNOUNCE_DELAY   2000	                         // frequency of autodiscovery announcements

NetDiscovery   discovery;
IPAddress      mcastIP(MCAST_ADDRESS);
SimpleTimer    timer;

#define AP_PASSWORD         "ca6admin"                    // to do: determine if this needs to be more secure
#define RESET_PIN           4                             // force WiFiManager credentials to be cleared

// WiFiManager globals
WiFiManager    wifiManager;

uint16         gPort = 4045;
WiFiUDP        gUDP;


/*
Create and return a (reasonably) unique WiFi SSID using the ESP8266 WiFi MAC address
Form the SSID as an IP address so the user knows what address to connect to when in AP mode just in case
(Even though the config page comes up automatically without the user having to use a browser)
*/
String createUniqueSSID (void) {
   uint8    mac[WL_MAC_ADDR_LENGTH];
   String   uSSID;

   WiFi.softAPmacAddress(mac);
   uSSID = String("CA6_") + String ("10.") + String(mac[WL_MAC_ADDR_LENGTH - 3]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 2]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 1]);
   DEBUG_MSG(2, F("Derived SSID"), uSSID);
   return uSSID;
}

/*
 create a (reasonably) unique class A private IP address using the ESP8266 WiFi MAC address
 we use class A private addresses to have a large potential address space to avoid conflicts
*/
IPAddress createUniqueIP (void) {
   uint8     mac[WL_MAC_ADDR_LENGTH];
   IPAddress result;

   WiFi.softAPmacAddress(mac);
   result[0] = 10;
   result[1] = mac[WL_MAC_ADDR_LENGTH - 3];
   result[2] = mac[WL_MAC_ADDR_LENGTH - 2];
   result[3] = mac[WL_MAC_ADDR_LENGTH - 1];
   DEBUG_MSG(2, F("Derived IP"), result);
   return result;
}

/*
 send autodiscovery announcement packet - called by the timer
*/
void hostAnnounce (void) {
   ND_Packet localPacket;

   // client must check this ID string to determine that this is a CA6 discovery packet
   DEBUG_MSG(4, "hostAnnounce", F(CA6_ANNOUNCE_ID));
   strcpy((char *)&localPacket.payload[0], CA6_ANNOUNCE_ID);
   if ( !discovery.announce(&localPacket) ) {	           // to do: is there a reason to check for an ACK or not?
      DEBUG_MSG(1, F("hostAnnounce"), F("failed"));
   }
}


void setup (void) {
   Serial.begin(74880);                     // SAM3X
   pinMode(RESET_PIN, INPUT_PULLUP);
#ifdef SERIAL_DEBUG
   SerialIO.begin(74880);                // console output
#endif

  IPAddress AP_Address = createUniqueIP();                 // unique IP address for AP mode to prevent conflicts with multiple devices

  // first check that there are WiFi networks to possibly connect to                                                  //WiFi.disconnect();
  int netCount = WiFi.scanNetworks();
  bool connectToAP = false;
  if ( netCount > 0 ) {
     // try to connect (saved credentials or manual entry if not) and default to AP mode if this fails

#ifdef DEBUG
     wifiManager.setDebugOutput(true);
#else
     wifiManager.setDebugOutput(false);
#endif

     DEBUG_MSG(3, F("Network scan"), netCount);
     if ( digitalRead(RESET_PIN) == LOW ) {	                 //reset settings option - for testing
        DEBUG_MSG(3, F("WiFiManager reset"), F("Activated"));
        wifiManager.resetSettings();
     }

     wifiManager.setBreakAfterConfig(true);	                                                 // undocumented function to return if config unsuccessful/skipped
     EEPROM.begin(128);                                                                       // allocates 128 bytes for wifiManager
     wifiManager.setSaveCredentialsInEEPROM(true, 0);                                         // [Local mod] forces credentials to be saved in EEPROM also

     WiFi.softAPConfig(AP_Address, AP_Address, IPAddress(255, 0, 0, 0));	                   // workaround for callout issue - see above

     if ( wifiManager.autoConnect(createUniqueSSID().c_str(), AP_PASSWORD) ) {	             // establish a unique SSID for connection in AP mode
        DEBUG_MSG(2, F("Connected (local WiFi)"), WiFi.localIP().toString());

        /*
         If we get to this point in the code, we are connected as a client and the ESP is in STA mode
         For a discussion of switching WiFi modes, see https://github.com/esp8266/Arduino/issues/2352
        */
#ifdef AP_WORKAROUND
        /*
         Until we implement autodiscovery on the client, the user must manually enter the address of the client
         The temporary workaround is to switch to mixed mode and reset the AP config to use the host address as the SSID
         while keeping the original MAC_based AP IP address. They user can thus find the local address by looking at the scanned SSIDs on their device.
         Disable this once autodiscovery has been implemented on the client.
        */

        WiFi.mode(WIFI_AP_STA);
        String ssid = String("Host IP: ") + WiFi.localIP().toString();
        WiFi.softAP(ssid.c_str());
        DEBUG_MSG(2, F("Host IP as SSID"), ssid);
        WiFi.softAPConfig(AP_Address, AP_Address, IPAddress(255, 0, 0, 0));
        //WiFi.reconnect();                                        // supposedly required, but does not work if this is called
        DEBUG_MSG(1, F("AP IP"), WiFi.softAPIP().toString());
#if DEBUG >= 3
        SerialIO.println(F("Workaround AP diag:"));
        WiFi.printDiag(SerialIO);
#endif

#endif

        /*
         OTA setup, which only makes sense when on a local WiFi network
        */
        // Port defaults to 8266
        ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        //ArduinoOTA.setHostname("myesp8266");

        // No authentication by default
        // ArduinoOTA.setPassword((const char *)"123");

        ArduinoOTA.onStart([]() {
           SerialIO.println(F("Start"));
        });
        ArduinoOTA.onEnd([]() {
           SerialIO.println(F("\nEnd. Restarting ..."));
           delay(5000);
           ESP.restart(); 
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
           SerialIO.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
           SerialIO.printf("Error[%u]: ", error);
           switch ( error ) {
           case OTA_AUTH_ERROR:
              SerialIO.println(F("Auth Failed"));
              break;

           case OTA_BEGIN_ERROR:
              SerialIO.println(F("Begin Failed"));
              break;

           case OTA_CONNECT_ERROR:
              SerialIO.println(F("Connect Failed"));
              break;

           case OTA_RECEIVE_ERROR:
              SerialIO.println(F("Receive Failed"));
              break;

           case OTA_END_ERROR:
              SerialIO.println(F("End Failed"));
              break;

           default:
              SerialIO.println(F("Unknown error"));
              break;
           }
        });
        ArduinoOTA.begin();
        SerialIO.println(F("OTA Ready:"));
        SerialIO.print(F("\tIP address: "));
        SerialIO.println(WiFi.localIP());
        SerialIO.print(F("\tChip ID: "));
        SerialIO.println(ESP.getChipId(), HEX);
     } else {
        // we get here if the credentials on the setup page are incorrect (or blank - easy way to exit)
        DEBUG_MSG(3, F("Did not connect to local WiFi"), F("use AP mode"));
        connectToAP = true;
     }
  } else {
     DEBUG_MSG(3, F("No local networks"), F("use AP mode"));
     connectToAP = true;
  }

  if ( connectToAP ) {
     // use AP mode	 - WiFiManager leaves the ESP in AP+STA mode if there was no connection made (eg STA mode)
     DEBUG_MSG(2, F("Using AP Mode"), AP_Address.toString());

     WiFi.mode(WIFI_AP);
     WiFi.softAP(createUniqueSSID().c_str(), AP_PASSWORD);
     WiFi.softAPConfig(AP_Address, AP_Address, IPAddress(255, 0, 0, 0));
#if DEBUG >= 3
     SerialIO.println(F("AP Diag:"));
     WiFi.printDiag(SerialIO);
#endif
  }

  // init autodiscovery, again only applicable to local WiFi networks
  if ( discovery.begin(mcastIP, MCAST_PORT) ) {
     timer.setInterval(AD_ANNOUNCE_DELAY, hostAnnounce);
  } else {
     DEBUG_MSG(1, F("Cannot initialize discovery mcast group"), mcastIP);
     // to do: indicate error on status LED when this is available
     //while ( true ) delay(1000);               // ESP will get a WDT reset if you don't have something in the loop
  }
  

  if ( gUDP.begin(gPort) == 1 ) {
     SerialIO.printf("Connected UDP, port: %d\n", gPort);
  } else {
     SerialIO.printf("UDP connection failed!\n");
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

#if DEBUG >= 4
void hexDump (const uint8 *buf, const int len) {
	SerialIO.printf("Dumping %d bytes:", len);
	for ( int i = 0; i < len; i++) {
		if ( i % 4 == 0 ) {
			SerialIO.write(" ");
		}
		SerialIO.printf("%02x", buf[i]);
	}
	SerialIO.write("\n");
}
#endif

void loop (void) {
  uint16    udpSize = gUDP.parsePacket();
  uint16    serialSize = Serial.available();
  static    uint16 gPacketSize = 0;
  static    IPAddress gIp;
  uint8     buf[2048];

  // read from the client
  if (udpSize > 0) {
     gIp = gUDP.remoteIP();
     udpSize = gUDP.read(buf, 2048);
	  size_t len = Serial.write(buf, udpSize);
     DEBUG_MSG(2, "UDP packet rcvd", udpSize);
     DEBUG_MSG(2, F("Device IP"), gIp);
#if DEBUG >= 4
	  hexDump(buf, udpSize);
#endif
     if ( len != udpSize ) {
        DEBUG_MSG(1, F("Error writing packet to SAM3x"), len);
     }
  }

  if (serialSize > 0) {
     // data avilable from the SAM3X for the client
     DEBUG_MSG(2, F("Client data available"), serialSize);
    
     if (gPacketSize == 0 && serialSize >= PACKET_SIZE_SIZE) {
        // calculate the size of the packet from the first 2 bytes in the stream
        uint8 ibuf[PACKET_SIZE_SIZE];
        Serial.readBytes(ibuf, PACKET_SIZE_SIZE);
        gPacketSize = genPacketSize(ibuf[0], ibuf[1]);
        DEBUG_MSG(3, F("packet size"), gPacketSize);
     }
     // the remaining bytes are the packet contents - copy from serial and send UDP packet
     if (gPacketSize != 0 && serialSize >= gPacketSize-PACKET_SIZE_SIZE) {
        buf[0] = getPacketSize(gPacketSize, 0);
        buf[1] = getPacketSize(gPacketSize, 1);
        Serial.readBytes(buf+PACKET_SIZE_SIZE, gPacketSize-PACKET_SIZE_SIZE);
        if ( gUDP.beginPacket(gIp, gPort + 1) ) {
           size_t length = gUDP.write(buf, gPacketSize);
           if ( length == gPacketSize ) {
              if ( gUDP.endPacket() == 0 ) {
                 DEBUG_MSG(1, F("Packet send error"), (IPAddress)gIp);
              }
           } else {
              DEBUG_MSG(1, F("Packet write failed"), length);
           }
           gPacketSize = 0;
        } else {
           DEBUG_MSG(1, F("Cannot create packet"), (IPAddress)gIp);
        }
     }
  }
  ArduinoOTA.handle();
  yield();
  timer.run();
}
