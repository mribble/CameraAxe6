////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Setup wifi ssid and ip
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
  CA_LOG(CA_INFO, "Derived SSID: %s\n", uSSID.c_str());
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
  CA_LOG(CA_INFO, "Derived IP: %s\n", result.toString().c_str());
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
    CA_LOG(CA_INFO, "Network scan count: %d\n", netCount);
    wifiManager.setDebugOutput(false);
    wifiManager.setBreakAfterConfig(true);              // Return if config unsuccessful
    //wifiManager.setExitButtonLabel("Standalone Mode");  // [Local mod] sets the label on the exit button to clarify the meaning of exiting from the portal

    //wifiManager.setAPStaticIPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));    // use native WiFi class call below instead
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));                    // Force a static ip
    WiFi.setAutoConnect(true);

    if ( wifiManager.autoConnect(createUniqueSSID().c_str(), CA_AP_PASSWORD) ) {
      // We are connected as a client and the ESP is in STA mode Ref: https://github.com/esp8266/Arduino/issues/2352
      CA_LOG(CA_INFO, "Connected(STA mode) local WiFi ip: %s\n", WiFi.localIP().toString().c_str());

      // We also need to know the address of the client as it will be running our HTTP server
      // Switch to mixed mode and broadcast the local IP address in the AP SSID name.
      // The user can thus find the local address by looking at the scanned SSIDs on their device
      WiFi.mode(WIFI_AP_STA);
      String ssid = String("CA6_STA_") + WiFi.localIP().toString();
      CA_LOG(CA_INFO, "Local IP as SSID: %s\n", ssid.c_str());
      WiFi.softAP(ssid.c_str());
      WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));
      WiFi.reconnect();  // supposedly required, but does not work if this is called
    }
    else {
      // We get here if the credentials on the setup page are incorrect, blank, or the "Exit" button was used
      CA_LOG(CA_INFO, "Did not connect to local WiFi\n");
      connectToAP = true;
    }
  }
  else {
    CA_LOG(CA_INFO, "No WiFI networks\n");
    connectToAP = true;
  }

  if (connectToAP) {
    // use AP mode   - WiFiManager leaves the ESP in AP+STA mode at this point
    CA_LOG(CA_INFO, "Using AP Mode: %s\n", myIPAddress.toString().c_str());
    WiFi.softAP(createUniqueSSID().c_str(), CA_AP_PASSWORD);
    WiFi.softAPConfig(myIPAddress, myIPAddress, IPAddress(255, 0, 0, 0));
    WiFi.mode(WIFI_AP);
  }

  // start the server
  gServer.begin();
}

void pollWifiMode() {
  bool statusSet = false;
  //CA_LOG(CA_INFO, "%d %d\n", WiFi.getMode(), WiFi.status());
  if ((WiFi.getMode() == WIFI_AP_STA) && (WiFi.status() == WL_CONNECTED)) {
    gLed.set(CALed::GREEN_BLINK);
    statusSet = true;
  }
  else if ((WiFi.getMode() == WIFI_STA) && (WiFi.status() == WL_CONNECTED)) {
    gLed.set(CALed::GREEN_ON);
    statusSet = true;
  }
  // WL_DISCONNECTED means not connected in STA mode (odd, but it's what the docs say -- probably this way to be compatible with Arduino wifi library)
  else if ((WiFi.getMode() == WIFI_AP) && (WiFi.status() == WL_DISCONNECTED)) {
    gLed.set(CALed::GREEN_RED_BLINK);
    statusSet = true;
  }
  
  if (statusSet == false) {
    gLed.set(CALed::RED_ON);
  }
}

