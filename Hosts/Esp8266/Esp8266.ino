#include <ESP8266WiFi.h>
#include <WiFiUDP.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // fork of tzapu WiFiManasger library: https://github.com/Rom3oDelta7/WiFiManager
#include <NetDiscovery.h>        // https://github.com/Rom3oDelta7/NetDiscovery
#include <ArduinoOTA.h>	
#include <EEPROM.h>

// ESP8266 SDK      Ref: https://espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
extern "C" {
#include "user_interface.h"
}

#define AP_WORKAROUND            // disable this define to eliminate the function to display the host IP address as an SSID
//#define SIMULATE_CLIENT_ACK          // *** COMMENT OUT FOR NORMAL OPERATION *** [TESTING] do not wait for client to ACK before confirming connection established

#define DEBUG 3                  // define as minimum desired debug level, or comment out to disable debug statements

#ifdef DEBUG

//#define SERIAL_DEBUG             // use Serial + separate serial line for debug messages as the hw serial is connected to the SAM3x - comment out for Serial I/O
              
#ifdef SERIAL_DEBUG

#define SerialIO      Serial1    // UART on GPIO2 - use a pullup (this is necessary for boot anyway)

#else // SERIAL_DEBUG

#define SerialIO      Serial

#endif // SERIAL_DEBUG

#define DEBUG_MSG(L, H, M)	       if ((L) <= DEBUG) {SerialIO.print("DEBUG> "); SerialIO.print(H); SerialIO.print(": "); SerialIO.println(M);}
#else // DEBUG
   
#define SerialIO      Serial
#define DEBUG_MSG(...) ;
#endif // DEBUG

// for LED status management

#define LED_STATUS_DELAY      30000UL            // how long to keep the connection status LED lights illuminated

os_timer_t  extinguishLED;                       // disable LED after timeout

typedef enum { OFF, ON, BLINK_OFF, BLINK_ON } LedState;

class Led {
public:
   Led(const int pin);                           // constructor
 
   // set the target LED state
   void     setState (const LedState ledState, const uint32_t interval = 0);
   // toggle current state - called by function set by setToggleFunction()
   void     toggleState(void);
   /*
    function to be called to change physical pin state

    this workaround is necessary since the ESP SDK is in C so we cannot get fancy with "this" etc.
    (the issue is pointers to member functions are not the same as pointers to functions [C++ naming, "this" is an implicit parameter in C++, etc.]

    need a function pointer for each instantiation of the class
    */
   void     setToggleFunction (void (*func)(void *pArg));


private:
   int        _ledPin;
   volatile    bool _illuminated = false;           // for blinking
   void       (*_toggle)(void *pArg) = nullptr;     // pointer to function to change pin value
   bool       _timerArmed = false;                  
   os_timer_t _timer;                               // ESP OS software timer (ESP8266 SDK)
};

/*
 Led constructor
 */
Led::Led (const int pin) {
   _ledPin = pin;
   pinMode(_ledPin, OUTPUT);
}


/*
 set the callback function for the os_timer for blinking
 */
void Led::setToggleFunction (void (*func)(void *pArg)) {
   _toggle = func;
   os_timer_setfn(&_timer, _toggle, nullptr);
}

/*
 This is called by the physical pin change "C" function to change the current (temporal) state for blinking
 */
void Led::toggleState (void) {
   os_intr_lock();                           // disable interrupts
   if ( _illuminated ) {
      _illuminated = false;
      digitalWrite(_ledPin, LOW);
   } else {
      _illuminated = true;
      digitalWrite(_ledPin, HIGH);
   }
   os_intr_unlock();                        // enable interrupts again
}


/*
 set a new target state of the LED
 for blinking, the state indicates the initial condition of the LED - this allows us to have alternating LEDs
 */
void Led::setState(const LedState ledState, const uint32_t interval) {                    
   if ( _timerArmed ) {
      // reset the timer whenever we change the state
      os_timer_disarm(&_timer);
      _timerArmed = false;
   }
   switch ( ledState ) {
   case OFF:
      digitalWrite(_ledPin, LOW);
      _illuminated = false;
      break;

   case ON:
      digitalWrite(_ledPin, HIGH);
      _illuminated = true;
      break;

   case BLINK_ON:
      // blink, with initial state ON
      digitalWrite(_ledPin, HIGH);
      _illuminated = true;
      // min interval is 5 - see ESP SDK documentation
      os_timer_arm(&_timer, interval >= 5 ? interval : 5, true);
      _timerArmed = true;
      break;

   case BLINK_OFF:
      // blink, with initial state OFF
      digitalWrite(_ledPin, LOW);
      _illuminated = false;
      os_timer_arm(&_timer, interval >= 5 ? interval : 5, true);
      _timerArmed = true;
      break;

   default:
      break;
   }
}

Led greenLED(4);
Led redLED(5);

/*
 LED toggle functions 
 see note in class declaration for why this is necessary
 */
void toggleRedLED (void *pArg) {
   redLED.toggleState();
}

void toggleGreenLED (void *pArg) {
   greenLED.toggleState();
}

/*
 disable the LED connection status indicators after a delay to save power
 */
void LEDTimeout (void *pArg ) { // arg not used
   greenLED.setState(OFF);
   redLED.setState(OFF);
}



// auto-discovery globals
#define MCAST_PORT          7247
#define MCAST_ADDRESS       239, 12, 17, 87
#define CA6_ANNOUNCE_ID     "CA6ANC"                      // announcement packet ID
#define AD_ANNOUNCE_DELAY   2000UL	                         // frequency of autodiscovery announcements  (msec)

NetDiscovery   discovery;
IPAddress      mcastIP(MCAST_ADDRESS);
os_timer_t     ADBeacon;


// WiFiManager globals

#define AP_PASSWORD         "ca6admin"                    // TODO: determine if this needs to be more secure

WiFiManager    wifiManager;

// client globals
#define UDP_PORT            4045

typedef enum { C_NOT_STARTED, C_PENDING, C_WAITING, C_ACKNOWLEDGED, C_ESTABLISHED } ClientState;

typedef enum { NO_MODE, STA_MODE, AP_MODE } ConnectionMode;

// all the state info we need to manage a client connection
typedef struct {
   uint16_t       port = UDP_PORT;                 // UDP port number
   uint16_t       udpSize = 0;                     // incoming data (UDP)
   uint16_t       serialSize = 0;                  // serial IO input pending
   uint16_t       packetSize = 0;                  // outgoing data
   WiFiUDP        stream;
   IPAddress      address;
   ConnectionMode mode = NO_MODE;
   ClientState    state = C_NOT_STARTED;
} CA6Client;

CA6Client client;

ConnectionMode connectToNetwork(void);             // IDE inserts the prototypes way above this, so we need this one to catch our typedef. Error results otherwise. IDE issue ... again :-(


/*
 Create and return a unique WiFi SSID using the ESP8266 WiFi MAC address
 Form the SSID as an IP address so the user knows what address to connect to when in AP mode just in case
 (Even though the config page typically comes up automatically without the user having to enter a URL)
*/
String createUniqueSSID (void) {
   uint8_t  mac[WL_MAC_ADDR_LENGTH];
   String   uSSID;

   WiFi.softAPmacAddress(mac);
   uSSID = String("CA6_") + String ("10.") + String(mac[WL_MAC_ADDR_LENGTH - 3]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 2]) + String(".") + String(mac[WL_MAC_ADDR_LENGTH - 1]);
   DEBUG_MSG(2, F("Derived SSID"), uSSID);
   return uSSID;
}

/*
 return a unique class A private IP address using the ESP8266 WiFi MAC address
 we use class A private addresses to have a large potential address space to avoid conflicts
*/
IPAddress createUniqueIP (void) {
   uint8_t   mac[WL_MAC_ADDR_LENGTH];
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
 autodiscovery handler

 currently tracks only one client device, which may change over time
 the client is always the most recent responder to the announcement

 See Extras\ADClientStub\ADClientStub.ino for the client-side logic part of this protocol 

 TODO: handle multiple clients (separate UDP ports)
*/
void autoDiscovery (void *pArg) {  //arg not used
   ND_Packet localPacket, remotePacket;

   // client must check this ID string to determine if this is a CA6 discovery packet
   DEBUG_MSG(4, F("autoDiscovery"), F(CA6_ANNOUNCE_ID));
   strcpy((char *)&localPacket.payload[0], CA6_ANNOUNCE_ID);
   if ( discovery.announce(&localPacket) ) {
#ifndef SIMULATE_CLIENT_ACK
      if ( (discovery.listen(&remotePacket) == ND_ACK) && (client.state == C_PENDING || client.state == C_WAITING) ) {
         // it is the responsibility of the client to only send a single ACK
         DEBUG_MSG(1, F("autoDiscovery"), F("ACK"));
         if ( strcmp((char *)&remotePacket.payload[0], CA6_ANNOUNCE_ID) == 0 ) {    // is this ACK for us?
            client.address = remotePacket.addressIP;
            client.state = C_ACKNOWLEDGED;
            DEBUG_MSG(1, F("Client acknowledged"), (IPAddress)client.address);
         }
      }
#else
      // simulate an ACK if not already set as connected
      if ( client.state == C_PENDING || client.state == C_WAITING ) {
         client.state = C_ACKNOWLEDGED; 
         DEBUG_MSG(1, F("Simulated client ACK"), "");
      }
#endif
   } else {
      DEBUG_MSG(1, F("autoDiscovery"), F("announce failed"));
   }
}


/*
 Connect to the network or establish a standalone AP network
 connection is fully established once the client sends an ACK via auto-discovery
 returns type of connection being established - STA or AP
 */
ConnectionMode connectToNetwork (void) {
   IPAddress      AP_Address = createUniqueIP();                 // prevent conflicts with multiple devices
   ConnectionMode mode = NO_MODE;

   // first check that there are WiFi networks to possibly connect to
   int netCount = WiFi.scanNetworks();
   bool connectToAP = false;
   if ( netCount > 0 ) {
      // try to connect (saved credentials or manual entry if not) and default to AP mode if this fails

      DEBUG_MSG(3, F("Network scan"), netCount);

      WiFi.softAPConfig(AP_Address, AP_Address, IPAddress(255, 0, 0, 0));	                   // workaround for callout issue

      if ( wifiManager.autoConnect(createUniqueSSID().c_str(), AP_PASSWORD) ) {	
         SerialIO.printf("STA mode connection at %s\n", WiFi.localIP().toString().c_str());
         mode = STA_MODE;

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

         SerialIO.print(F("AP+STA mode workaround ENABLED. AP address: "));
         SerialIO.println(WiFi.softAPIP().toString());
#if DEBUG >= 3
         SerialIO.println(F("Workaround AP diag:"));
         WiFi.printDiag(SerialIO);
#endif

#endif // AP_WORKAROUND

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
            SerialIO.println(F("OTA Start"));
         });
         ArduinoOTA.onEnd([]() {
            SerialIO.println(F("\nOTA End. Restarting ..."));
            delay(5000);
            ESP.restart();                          // the OTA library calls restart, but we never return, so force it.
         });
         ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            SerialIO.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
         });
         ArduinoOTA.onError([](ota_error_t error) {
            SerialIO.printf("OTA Error[%u]: ", error);
            switch ( error ) {
            case OTA_AUTH_ERROR:
               SerialIO.println(F("OTA Auth Failed"));
               break;

            case OTA_BEGIN_ERROR:
               SerialIO.println(F("OTA Begin Failed"));
               break;

            case OTA_CONNECT_ERROR:
               SerialIO.println(F("OTA Connect Failed"));
               break;

            case OTA_RECEIVE_ERROR:
               SerialIO.println(F("OTA Receive Failed"));
               break;

            case OTA_END_ERROR:
               SerialIO.println(F("OTA End Failed"));
               break;

            default:
               SerialIO.println(F("OTA Unknown error"));
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
         DEBUG_MSG(3, F("Did not connect to local WiFi"), F("using AP mode"));
         connectToAP = true;
      }
   } else {
      DEBUG_MSG(3, F("No local networks"), F("using AP mode"));
      connectToAP = true;
   }

   if ( connectToAP ) {
      // use AP mode	 - WiFiManager leaves the ESP in AP+STA mode if there was no local connection made 
      SerialIO.printf("AP mode connection at %s\n", AP_Address.toString().c_str());
      mode = AP_MODE;

      WiFi.mode(WIFI_AP);
      WiFi.softAP(createUniqueSSID().c_str(), AP_PASSWORD);
      WiFi.softAPConfig(AP_Address, AP_Address, IPAddress(255, 0, 0, 0));
#if DEBUG >= 3
      SerialIO.println(F("AP Diag:"));
      WiFi.printDiag(SerialIO);
#endif
   }
   return mode;
}


void setup (void) {
   Serial.begin(74880);                     // SAM3X
#ifdef SERIAL_DEBUG
   SerialIO.begin(74880);                   // console output
#endif
   EEPROM.begin(128);                       // allocates 128 bytes for wifiManager (required by the library)

#ifdef DEBUG
   wifiManager.setDebugOutput(true);                       // NOTE: wifi manager library debug output goes to Serial, NOT Serial1
#else
   wifiManager.setDebugOutput(false);
#endif

   wifiManager.setBreakAfterConfig(true);	                 // undocumented function to return if config unsuccessful/skipped
   wifiManager.setSaveCredentialsInEEPROM(true);           // [Local mod] forces credentials to be saved in EEPROM also
   wifiManager.setExitButtonLabel("Access Point Mode");    // [Local mod] sets the label on the exit button to clarify the meaning of exiting from the portal
                                                       
   greenLED.setToggleFunction(&toggleGreenLED);            // init LED toggle functions for LED blinking
   greenLED.setState(OFF);
   redLED.setToggleFunction(&toggleRedLED);
   redLED.setState(OFF);

   // establish timer callout functions - arm timers in loop()
   os_timer_setfn(&ADBeacon, &autoDiscovery, nullptr);     // auto-discovery announcement beacon
   os_timer_setfn(&extinguishLED, &LEDTimeout, nullptr);   // turn off LED after setup
}

#define PACKET_SIZE_SIZE 2

uint16_t genPacketSize(uint8_t b0, uint8_t b1) {
  uint16_t ret = uint16_t(b0) + (uint16_t(b1)<<8);
  return ret;
}

uint8_t getPacketSize(uint16_t val, uint8_t byteNumber) {
  if (byteNumber == 0) {
    return uint8_t(val & 0xFF);
  } else {
    return val >> 8;
  }
}

#if DEBUG >= 4
void hexDump (const uint8_t *buf, const int len) {
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

bool fatalError = false;                    // true if fatal error occurred 

void loop (void) {
   if ( fatalError ) {
      // TODO: any better way to recover (restart??)
      greenLED.setState(OFF);
      redLED.setState(ON);
      while (true) delay(1000);
   }
   if ( client.mode == NO_MODE ) {
      // initiate network connection - can only do this section once
      greenLED.setState(BLINK_ON, 500UL);
      redLED.setState(BLINK_OFF, 500UL);
      client.mode = connectToNetwork();
      client.state = C_PENDING;

      // initialize auto-discovery
      if ( discovery.begin(mcastIP, MCAST_PORT, client.mode == AP_MODE ? WiFi.softAPIP() : WiFi.localIP()) ) {
         os_timer_arm(&ADBeacon, AD_ANNOUNCE_DELAY, true);
         SerialIO.println(F("Auto-discovery started"));
      } else {
         SerialIO.println(F("Auto-discovery FAILED"));
         fatalError = true;
      }
   } else if ( client.state == C_PENDING ) {
      // connected to network but no ACK from client yet
      switch ( client.mode ) {
      case STA_MODE:
         greenLED.setState(BLINK_ON, 500UL);
         redLED.setState(OFF);
         break;

      case AP_MODE:
         greenLED.setState(BLINK_ON, 125UL);
         redLED.setState(OFF);
         break;

      default:
         break;
      }
      client.state = C_WAITING;       // like pending, but leaves LED state as-is
   } else if ( client.state == C_ACKNOWLEDGED ) {
      // ACK received
      if ( client.stream.begin(client.port) ) {
         /*
          Any device could have opened up this port, but we'll consider this OK and validate
          the addresses once we receive a packet (only time the address is available)
          */
         SerialIO.printf("Client connected on port %d\n", client.port);
         client.state = C_ESTABLISHED;
         greenLED.setState(ON);
         redLED.setState(OFF);
         os_timer_arm(&extinguishLED, LED_STATUS_DELAY, false);        // this is a 1-shot event
      } else {
         // don't change state - will keep trying to establish connection
         DEBUG_MSG(1, F("UDP connection"), F("Failed"));
         greenLED.setState(BLINK_ON, 125UL);
         redLED.setState(BLINK_OFF, 125UL);
      }
   } else if ( client.state == C_ESTABLISHED ) {
      // end-to-end connection in place  Note: no Serial I/O except to the SAM3x at this point
      if ( client.stream.parsePacket() > 0 ) {
         uint8_t buf[2048];
#ifdef SIMULATE_CLIENT_ACK
         client.address = client.stream.remoteIP();              // force this for testing
#endif
         // for security, verify that the client that sent the ACK is the same as the one that sent this packet
         if ( client.address == client.stream.remoteIP() ) {
            client.udpSize = client.stream.read(buf, sizeof(buf));
            DEBUG_MSG(2, F("UDP packet rcvd"), client.udpSize);
            size_t len = Serial.write(buf, client.udpSize);
#if DEBUG >= 4
            hexDump(buf, client.udpSize);
#endif
            if ( len != client.udpSize ) {
               DEBUG_MSG(1, F("Error writing packet to SAM3x"), len);
            }
         } else {
            DEBUG_MSG(1, F("UDP stream IP Address mismatch"), (IPAddress)client.stream.remoteIP());
         }
      }

      client.serialSize = Serial.available();
      if ( client.serialSize > 0 ) {
         // data avilable from the SAM3X for the client
         DEBUG_MSG(2, F("Client data available"), client.serialSize);
    
         if ( client.packetSize == 0 && (client.serialSize >= PACKET_SIZE_SIZE) ) {
            // calculate the size of the packet from the first 2 bytes in the stream
            uint8_t ibuf[PACKET_SIZE_SIZE];
            Serial.readBytes(ibuf, PACKET_SIZE_SIZE);
            client.packetSize = genPacketSize(ibuf[0], ibuf[1]);
            DEBUG_MSG(3, F("packet size"), client.packetSize);
         }
         /*
          the remaining bytes are the packet contents - copy from serial and send UDP packet
          it make take a couple of interations until all the bytes are available in the stream
          */
         if ( client.packetSize != 0 && (client.serialSize >= client.packetSize - PACKET_SIZE_SIZE) ) {
            uint8_t buf[2048];
            buf[0] = getPacketSize(client.packetSize, 0);
            buf[1] = getPacketSize(client.packetSize, 1);
            Serial.readBytes(buf+PACKET_SIZE_SIZE, client.packetSize-PACKET_SIZE_SIZE);
            /*
             already verified that the request's UDP IP source addr matches the ACKing client,
             so no need to further validate this
             */
            if ( client.stream.beginPacket(client.address, client.port + 1) ) {
               size_t length = client.stream.write(buf, client.packetSize);
               if ( length == client.packetSize ) {
                  if ( client.stream.endPacket() == 0 ) {
                     DEBUG_MSG(1, F("Packet send error"), (IPAddress)client.address);
                  }
               } else {
                  DEBUG_MSG(1, F("Packet write failed"), length);
               }
               client.packetSize = 0;
            } else {
               DEBUG_MSG(1, F("Cannot create packet"), (IPAddress)client.address);
            }
         }
      }
   }

   ArduinoOTA.handle();
   yield();
}
