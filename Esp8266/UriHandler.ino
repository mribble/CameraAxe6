////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Handles URI strings from javascript
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

boolean jscaMatch(String& jsca, const char* ref) {
  boolean ret;
  ret = (jsca.indexOf(ref) != -1) ? true : false;
  return ret;
}

void sendResponce(const char* jsaaToken, String &val) {
  gClient.print(jsaaToken);
  gClient.print("&~&");
  gClient.print(val + "&~&");
}

void sendResponce(const char* jsaaToken, const char* val) {
  String temp = val;
  sendResponce(jsaaToken, temp);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This services message from the webpage (including ajax messages from JS)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serviceUri() {
  gClient = gServer.available();
  if (!gClient || !gClient.connected()) {
    return;
  }

  String uri = gClient.readStringUntil('\r');
  String jsca;
  int16_t jscaOffset = 0;
  gClient.flush();
  uri.replace("%20", " "); // The uri code converts spaces to %20, this undoes that transformation

  CA_LOG(CA_JS_URI, "Full URI: %s\n", uri.c_str());

  if (jscaMatch(uri, "GET / HTTP/1.1") || jscaMatch(uri, "GET /index.html")) {
    CA_LOG(CA_JS_URI, " ProcURI: LoadMainPage\n");
    loadMainWebPage();
    gClient.flush();
    return;
  }

  while(jscaOffset != -1) {
    jscaOffset = getNextSubstring(uri, jsca, jscaOffset);
    if (jscaOffset == -1) {
      break;
    }

    if (jscaMatch(jsca, "JSCA_GET_MENU_LIST")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_MENU_LIST\n");
      Dir dir = SPIFFS.openDir("/menus/");
      String str;
      while (dir.next()) {
        str += String(dir.fileName()).substring(7) + "~";
      }
      sendResponce("JSCA_GET_MENU_LIST", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_MENU_DYNAMIC_STRING_IDS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_MENU_DYNAMIC_STRING_IDS\n");
      String str;
      for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
        str += String("id") + gDynamicMessages.id[i] + "~" + gDynamicMessages.str[i] + "~";
      }
      gDynamicMessages.numMessages = 0;
      if (str.length() == 0) {
        str = "null";
      }
      sendResponce("JSCA_GET_MENU_DYNAMIC_STRING_IDS", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_MENU_DYNAMIC_UINT32_IDS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_MENU_DYNAMIC_UINT32_IDS\n");
      String str;
      if (gDynamicUint32s.length() == 0) {
        str = "null";
      }
      else {
        str = gDynamicUint32s;
      }
      gDynamicUint32s.remove(0);
      sendResponce("JSCA_GET_MENU_DYNAMIC_UINT32_IDS", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_CURRENT_DYNAMIC_MENU")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_CURRENT_DYNAMIC_MENU\n");
      String name, str, str2;
      jscaOffset = getNextSubstring(uri, name, jscaOffset);
      name.replace(" HTTP/1.1", "");
      str = String("/menus/") + name;
      if (!readFileToString(str.c_str(), str2)) {
        str2 = "null";
      }
      sendResponce("JSCA_GET_CURRENT_DYNAMIC_MENU", str2);
    }
    else if (jscaMatch(jsca, "JSCA_GET_INTERVAL")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_INTERVAL\n");
      String str;
      if (!readFileToString(gIntervalFilename, str)) {
        str = gIntervalometerDefaults;
      }
      gPh.writePacketIntervalometer(str);
      sendResponce("JSCA_GET_INTERVAL", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_ALL_CAMS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_ALL_CAMS\n");
      String str;
      if (!readFileToString(gCamSettingsFilename, str)) {
        str = gCamSettingDefaults;
      }
      setAllCams(str);
      sendResponce("JSCA_GET_ALL_CAMS", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_DYNAMIC_MENU_SETTINGS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_DYNAMIC_MENU_SETTINGS\n");
      String name, str2;
      jscaOffset = getNextSubstring(uri, name, jscaOffset);
      name.replace(" HTTP/1.1", "");
      String str = String("/data/") + name;
      if (!readFileToString(str.c_str(), str2)) {
        str2 = "null";
      }
      sendResponce("JSCA_GET_DYNAMIC_MENU_SETTINGS", str2);
    }
    else if (jscaMatch(jsca, "JSCA_GET_START_LOCATION")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_START_LOCATION\n");
      String str;
      if (!readFileToString(gStartLocation, str)) {
        str = gStartLocationDefaults;
      }
      sendResponce("JSCA_GET_START_LOCATION", str);
    }
    else if (jscaMatch(jsca, "JSCA_GET_VOLTAGE")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_GET_VOLTAGE\n");
      String str = String(gVoltage/100) + "." + String(gVoltage%100/10) + String(gVoltage%10);
      sendResponce("JSCA_GET_VOLTAGE", str);
    }
    else if (jscaMatch(jsca, "JSCA_SET_DEFAULTS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_SET_DEFAULTS\n");
      Dir dir = SPIFFS.openDir("/data");
      while (dir.next()) {
        SPIFFS.remove(dir.fileName());
        CA_LOG("Delete - %s\n", dir.fileName().c_str());
      }
      sendResponce("JSCA_SET_DEFAULTS", "done");
    }
    else if (jscaMatch(jsca, "JSCA_RAW_PACKET")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_RAW_PACKET\n");
      String packetStr;
      jscaOffset = getNextSubstring(uri, packetStr, jscaOffset);
      packetStr.replace(" HTTP/1.1", "");
      sendPacket(packetStr);
      sendResponce("JSCA_RAW_PACKET", "done");
    }
    else if (jscaMatch(jsca, "JSCA_CLEAR_WIFI_NETWORKS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_CLEAR_WIFI_NETWORKS\n");
      ESP.eraseConfig();
      sendResponce("JSCA_CLEAR_WIFI_NETWORKS", "done");
    }
    else if (jscaMatch(jsca, "JSCA_SET_ALL_CAMS")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_SET_ALL_CAMS\n");
      String str;
      jscaOffset = getNextSubstring(uri, str, jscaOffset);
      str.replace(" HTTP/1.1", "");
      setAllCams(str);
      saveStringToFlash(gCamSettingsFilename, str);
      sendResponce("JSCA_SET_ALL_CAMS", "done");
    }
    else if (jscaMatch(jsca, "JSCA_SET_START_LOCATION")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_SET_START_LOCATION\n");
      String name;
      jscaOffset = getNextSubstring(uri, name, jscaOffset);
      name.replace(" HTTP/1.1", "");
      saveStringToFlash(gStartLocation, name);
      sendResponce("JSCA_SET_START_LOCATION", "done");
    }
    else if (jscaMatch(jsca, "JSCA_SET_DYNAMIC_MENU")) {
      CA_LOG(CA_JS_URI, " ProcURI: JSCA_SET_DYNAMIC_MENU\n");
      String str;
      jscaOffset = getNextSubstring(uri, str, jscaOffset);
      str.replace(" HTTP/1.1", "");
      uint16_t nameEnd = str.indexOf("&");
      String name = str.substring(0, nameEnd);
      str = str.substring(nameEnd+1);
      setDynamicMenu(name, str);
      sendResponce("JSCA_SET_DYNAMIC_MENU", "done");
    }
    else {
      CA_LOG(CA_ERROR, "ERROR - Unknown URI: %s\n", uri.c_str());
    }
  }
  gClient.flush();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are a bunch of simple helper functions used by serviceUri() 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loadMainWebPage() {
  if (WiFi.getMode() == WIFI_AP_STA) {
    // If both access point and station mode are active that means we are using station mode.
    // The access point mode is just used to display the IP address, but once the user connects
    // we can turn off the access point to save power.
    WiFi.mode(WIFI_STA);
  }
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  sendFileToClient(gMainPageFilename);
}

void setDynamicMenu(String& name, String& packets) {
  int16_t startOffset = 0;
  String subStr;
  String name2 = "/data/"+name;

  while (startOffset != -1) {
    startOffset = getPacketSubstring(packets, subStr, startOffset);
    if (startOffset != -1) {
      sendPacket(subStr);
    }
  }
  saveStringToFlash(name2.c_str(), packets);
}

void setAllCams(String& packets) {
  int16_t startOffset = 0;
  String subStr;

  while (startOffset != -1) {
    startOffset = getPacketSubstring(packets, subStr, startOffset);
    if (startOffset != -1) {
      sendPacket(subStr);
    }
  }
}

int16_t getPacketSubstring(const String &str, String& subStr, int16_t startOffset) {
  int16_t endOffset = str.indexOf('&', startOffset);
  if (endOffset != -1) {
    subStr = str.substring(startOffset, endOffset);  // Drop the trailing & per packet
    endOffset++;
  }

  if (subStr.length() == 0) {  // Remove the trailing &
    endOffset = -1;
  }
  
  return endOffset;
}

int16_t getNextSubstring(const String &str, String& subStr, int16_t startOffset) {
  int16_t endOffset = str.indexOf("&~&", startOffset);
  if (endOffset != -1) {
    subStr = str.substring(startOffset, endOffset);  // Drop the trailing & per packet
    endOffset += sizeof("&~&")-1;
  }

  if (subStr.length() == 0) {  // Remove the trailing delimiter
    endOffset = -1;
  }
  
  return endOffset;
}


