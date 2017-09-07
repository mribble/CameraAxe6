////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create and return a unique WiFi SSID using the ESP8266 WiFi MAC address
// Form the SSID as an IP address so the user knows what address to connect to when in AP mode just in case
// (The config page only comes up automatically without the user having to enter a URL on some devices)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
String createUniqueSSID() {
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  String uSSID;

  WiFi.softAPmacAddress(mac);
  uSSID = String("CA6_AP_") + "10." + mac[WL_MAC_ADDR_LENGTH - 3] + "." + mac[WL_MAC_ADDR_LENGTH - 2] + "." + mac[WL_MAC_ADDR_LENGTH - 1];
  CA_INFO("Derived SSID", uSSID);
  return uSSID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Return a unique class A private IP address using the ESP8266 WiFi MAC address
// we use class A private addresses to have a large potential address space to avoid conflicts
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IPAddress createUniqueIP() {
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize the wifi router
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setupWiFi() {
  IPAddress myIPAddress = createUniqueIP();
  bool connectToAP = false;
  int netCount = WiFi.scanNetworks();  // Number of networks that we might be able to connect to

  if ( netCount > 0 ) {
    // try to connect (saved credentials or manual entry if not) and default to AP mode if this fails
    WiFiManager wifiManager;
    CA_INFO("Network scan count", netCount);
//#ifdef CA_DEBUG_INFO
#if 0
    wifiManager.setDebugOutput(true);
#else
    wifiManager.setDebugOutput(false);
#endif
    wifiManager.setBreakAfterConfig(true);              // undocumented function to return if config unsuccessful
    wifiManager.setSaveCredentialsInEEPROM(true);       // [Local mod] forces credentials to be saved in EEPROM also
    wifiManager.setExitButtonLabel("Standalone Mode");  // [Local mod] sets the label on the exit button to clarify the meaning of exiting from the portal

    //wifiManager.setAPStaticIPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));    // use native WiFi class call below instead
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));                    // Force a static ip
    WiFi.setAutoConnect(true);

    if ( wifiManager.autoConnect(createUniqueSSID().c_str(), CA_AP_PASSWORD) ) {
      // We are connected as a client and the ESP is in STA mode Ref: https://github.com/esp8266/Arduino/issues/2352
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
      gLed.set(CALed::GREEN_BLINK);
    }
    else {
      // We get here if the credentials on the setup page are incorrect, blank, or the "Exit" button was used
      CA_INFO("Did not connect to local WiFi", "");
      connectToAP = true;
    }
  }
  else {
    CA_INFO("No WiFI networks", "");
    connectToAP = true;
  }

  if (connectToAP) {
    // use AP mode   - WiFiManager leaves the ESP in AP+STA mode at this point
    CA_INFO("Using AP Mode", myIPAddress.toString());
    WiFi.softAP(createUniqueSSID().c_str(), CA_AP_PASSWORD);
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));
    WiFi.mode(WIFI_AP);
    gLed.set(CALed::ORANGE_BLINK);
  }

  // start the server & init the SPIFFS file system
  gServer.begin();
}
