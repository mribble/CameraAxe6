////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This services message from the webpage (including ajax messages from JS)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serviceUri() {
  gClient = gServer.available();
  if (!gClient || !gClient.connected()) {
    return;
  }

  String uri = gClient.readStringUntil('\r');
  boolean putRequest = false;
  gClient.flush();
  uri.replace("%20", " "); // The uri code converts spaces to %20, this undoes that transformation

  if (uri.indexOf("GET /getMenuDynamicIds ") != -1) {
    getMenuDynamicIds();
  }
  else if (uri.indexOf("GET /getMenuList ") != -1) {
    getMenuList();
  }
  else if (uri.indexOf("GET /getCurrentDynamicMenu ") != -1){
    getCurrentDynamicMenu(uri);
  }
  else if (uri.indexOf("GET /getAllCams ") != -1) {
    String str;
    readFileToString(gCamSettingsFilename, str);
    if (str.length() > 0)
    {
      gClient.print(str);
      setAllCams(str);
    }
    else {
      gClient.print(gCamSettingDefaults);
      setAllCams(gCamSettingDefaults);
    }
  }
  else if (uri.indexOf("PUT /setAllCams ") != -1) {
    String str = uri.substring(16, uri.length()-9);
    setAllCams(str);
    saveStringToFlash(gCamSettingsFilename, str);
    putRequest = true;
  }
  else if (uri.indexOf("GET /getInterval ") != -1) {
    String str;
    readFileToString(gIntervalFilename, str);
    if (str.length() > 0)
    {
      gClient.print(str);
      gPh.writePacketIntervalometer(str);
    } else {
      gClient.print(gIntervalometerDefaults);
      gPh.writePacketIntervalometer(gIntervalometerDefaults);
    }
  }
  else if (uri.indexOf("PUT /setDynamicMenu ") != -1) {  // This is the javascript to display the menu
    uint16_t nameEnd = uri.indexOf("&");
    String name = uri.substring(20, nameEnd);
    String str = uri.substring(nameEnd+1, uri.length()-9);
    setDynamicMenu(name, str);
    putRequest = true;
  }
  else if (uri.indexOf("GET /getDynamicMenuSettings ") != -1) {  // These are the settings stored in flash
    String name = uri.substring(28, uri.length()-9);
    String str = String("/data/") + name;
    sendFileToClient(str.c_str());
  }
  else if (uri.indexOf("GET /setDefaults ") != -1) {
    Dir dir = SPIFFS.openDir("/data");
    while (dir.next()) {
      SPIFFS.remove(dir.fileName());
      CA_LOG("Delete - %s\n", dir.fileName().c_str());
    }

    // todo look into this more -- Seems to be a bug where the dir doesn't always list all files
    //dir = SPIFFS.openDir("/data");
    //while (dir.next()) {
    //  SPIFFS.remove(dir.fileName());
    //  CA_LOG("Delete - %s\n", dir.fileName().c_str());
    //}
    gClient.print("HTTP/1.1 200 OK");
  }
  else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    loadMainWebPage();
  }
//  else if (uri.indexOf("GET /favicon.ico") != -1) {
//    // Ignore this case
//  }
  else if (uri.indexOf("~") != -1) {    // This is the dynamic state case
    String packetStr = uri.substring(5, uri.length()-9);
    sendPacket(packetStr);
    putRequest = true;
  }
  else {
    CA_INFO("ERROR - Unknown URI - ", uri.c_str());
  }

  if (putRequest) {
    gClient.print("HTTP/1.1 200 OK");
  }

  gClient.stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are a bunch of simple helper functions used by serviceUri() 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void getMenuDynamicIds() {
  String val;

  for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
    val += String("id") + gDynamicMessages.id[i] + "~" + gDynamicMessages.str[i] + "~";
  }
  gDynamicMessages.numMessages = 0;
  if (val.length() == 0) {
    val = "null";
  }
  gClient.print(val);
}

void getMenuList() {
  Dir dir = SPIFFS.openDir("/menus/");
  String str;
  while (dir.next()) {
    str += String(dir.fileName()).substring(7) + "~";
  }
  gClient.print(str);
}

void getCurrentDynamicMenu(String& uri) {
  String str;
  uri.replace("GET /getCurrentDynamicMenu ", "");
  uri.replace(" HTTP/1.1", "");
  str = String("/menus/") + uri;
  sendFileToClient(str.c_str());  // Sends file with dynamic menu to javascript
}

void loadMainWebPage() {
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


