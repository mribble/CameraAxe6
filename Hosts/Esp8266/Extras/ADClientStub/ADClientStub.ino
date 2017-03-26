/*
 Test/demo code for CA6 auto-discovery
 
 This sketch is used to demonstrate the client side of the auto-discovery protocol
 It can also be used for testing to ACK the announcement from the host in place of an actual client
 
 This sketch will work on either lcoal networks or in AP mode (set network credentials accordingly)
 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "NetDiscovery.h"
#include <array>

#define CA_DEBUG

#include <CAStd.h>

#define MCAST_PORT          7247
#define MCAST_ADDRESS       239, 12, 17, 87
#define CA6_ANNOUNCE_ID     "CA6ANC"                      // announcement packet ID
#define SSID                "CA6_10.215.126.227"          // *** change this to the IP address of the ESP8266 under test ***
#define PASSWORD            "ca6admin"

NetDiscovery       discovery;
IPAddress          mcastIP(MCAST_ADDRESS);               // multicast address

void setup(void)
{
	Serial.begin(74880);
	WiFi.begin(SSID, PASSWORD);
	while ( WiFi.status() != WL_CONNECTED ){
		delay(500);
		Serial.print(".");
	}
	Serial.println();
	CA_INFO(F("CLIENT Connected. Local IP"), WiFi.localIP());

	if ( !discovery.begin(mcastIP, MCAST_PORT, WiFi.localIP()) ) {       // join mcast group
		CA_ERROR(F("Cannot initialize discovery mcast group"), mcastIP);
		while ( true ) delay(1000);
	}
}

// this example just loops forever, not what a real client would do
void loop(void)
{
	ND_Packet remotePacket, localPacket;
	uint8_t   addressMAC[WL_MAC_ADDR_LENGTH];
	std::array<bool, 256> senderACK;

	CA_INFO(F("CLIENT listening"), "");
	senderACK.fill(false);	
	while ( true ) {
		// listen for announcement packets 
		Serial.write(".");
		if ( discovery.listen(&remotePacket) == ND_ANNOUNCE ) {
			// examine payload for the ID string 
			CA_INFO(F("Announcement from"), (IPAddress)remotePacket.addressIP);
			if ( strcmp((char *)&remotePacket.payload[0], CA6_ANNOUNCE_ID) == 0 ) {
				uint8_t senderID = remotePacket.addressIP[3];           // use the last octet in the IP address for unique identification
				// only send an ACK once
				if ( !senderACK[senderID] ) {
					CA_INFO(F("Sending ACK"), (IPAddress)remotePacket.addressIP);
					strcpy((char *)&localPacket.payload[0], CA6_ANNOUNCE_ID);
					if ( discovery.ack(&localPacket) ) {
						Serial.print(F("Discovered device at "));
						Serial.println((IPAddress)remotePacket.addressIP);
						senderACK[senderID] = true;
						Serial.println();
					}
				} 
			}
		}
		yield();
		delay(1000);
	}
}
