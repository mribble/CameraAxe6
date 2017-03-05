/*
 Test code for CA6 auto-discovery
 Simply listens for announcement packets and displays the IP and MAC address of the initiator
 This code assumes local WiFI network only
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "NetDiscovery.h"


#define DEBUG 3                  // define as minimum desired debug level, or comment out to disable debug statements

#ifdef DEBUG
#define DEBUG_MSG(L, H, M)	       if ((L) <= DEBUG) {Serial.print("DEBUG> "); Serial.print(H); Serial.print(": "); Serial.println(M);}
#else
#define DEBUG_MSG(...)            ;
#endif

#define MCAST_PORT          7247
#define MCAST_ADDRESS       239, 12, 17, 87
#define CA6_ANNOUNCE_ID     "CA6ANC"                      // announcement packet ID
#define SSID                "<ssid>"
#define PASSWORD            "<password>"

NetDiscovery       discovery;
IPAddress          mcastIP(MCAST_ADDRESS);               // multicast address

void setup(void)
{
	Serial.begin(115200);
	WiFi.begin(SSID, PASSWORD);
	while ( WiFi.status() != WL_CONNECTED ){
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	DEBUG_MSG(1, F("RECEIVER Connected. Local IP"), WiFi.localIP());

	if ( !discovery.begin(mcastIP, MCAST_PORT) ) {       // join mcast group
		DEBUG_MSG(1, F("Cannot initialize discovery mcast group"), mcastIP);
		while ( true ) delay(1000);
	}
}


void loop(void)
{
	ND_Packet remotePacket;

	DEBUG_MSG(1, F("RECEIVER listening"), "");

	while ( true ) {
		// listen for announcement packets 
		Serial.print(".");
		if ( discovery.listen(&remotePacket) == ND_ANNOUNCE ) {
			if ( strcmp((char *)&remotePacket.payload[0], CA6_ANNOUNCE_ID) == 0 ) {
				Serial.print(F("Discovered device at "));
				Serial.println((IPAddress)remotePacket.addressIP);
				Serial.print(F("Remote MAC: "));
				for ( int i = 0; i < WL_MAC_ADDR_LENGTH; i++ ) {
					Serial.print(remotePacket.addressMAC[i], HEX);
					if ( i < WL_MAC_ADDR_LENGTH - 1 ) {
						Serial.print(".");
					}
				}
				Serial.println();
			}
		}
		yield();
		delay(1000);
	}
}
