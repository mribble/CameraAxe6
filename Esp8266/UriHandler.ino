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

  if (uri.indexOf("GET /updateDynamicState ") != -1) {
    updateDynamicState();
  }
  else if (uri.indexOf("GET /updateMenuList ") != -1) {
    updateMenuList();
  }
  else if (uri.indexOf("GET /loadDynamicMenu") != -1){
    loadDynamicMenu(uri);
  }
  else if (uri.indexOf("PUT /updateAllCams") != -1) {
    String str = uri.substring(18, uri.length()-9);
    updateAllCams(str);
    putRequest = true;
  }
  else if (uri.indexOf("PUT /updateDynamicMenu") != -1) {
    uint16_t nameEnd = uri.indexOf("&");
    String name = uri.substring(22, nameEnd);
    String str = uri.substring(nameEnd+1, uri.length()-9);
    updateDynamicMenu(name, str);
    putRequest = true;
  }
  else if (uri.indexOf("GET /updateSaveStateInterval") != -1) {
    sendFileToClient("/d/interval");
  }
  else if (uri.indexOf("GET /updateSaveStateCams") != -1) {
    sendFileToClient("/d/cams");
  }
  else if (uri.indexOf("GET /updateDynamicSettings") != -1) {
    String name = uri.substring(26, uri.length()-9);
    String str = String("/d/") + name;
    sendFileToClient(str.c_str());
  }
  else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    loadMainWebPage();
  }
  else if (uri.indexOf("GET /favicon.ico") != -1) {
    // Ignore this case
  }
  else if (uri.indexOf("~") != -1) {    // This is the dynamic state case
    String packetStr = uri.substring(5, uri.length()-9);
    sendPacket(packetStr);
    putRequest = true;
  }
  else {
    CA_INFO("ERROR - Unknown URI - ", uri);
  }

  if (putRequest) {
    gClient.print("HTTP/1.1 200 OK");
  }

  gClient.stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Below are a bunch of simple helper functions used by serviceUri() 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateDynamicState() {
  String val;
  for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
    val += String("id") + gDynamicMessages.id[i] + "~" + gDynamicMessages.str[i] + "~";
  }
  gDynamicMessages.numMessages = 0;
  gClient.print(val);
}

void updateMenuList() {
  Dir dir = SPIFFS.openDir("/menus/");
  String str;
  while (dir.next()) {
    str += String(dir.fileName()).substring(7) + "~";
  }
  gClient.print(str);
}

void loadDynamicMenu(String& uri) {
  String str;
  uri.replace("GET /loadDynamicMenu", "");
  uri.replace(" HTTP/1.1", "");
  str = String("/menus/") + uri;
  sendFileToClient(str.c_str());  // Sends file with dynamic menu to javascript
}

void loadMainWebPage() {
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  sendFileToClient("/Index.html");
}

void updateDynamicMenu(String& name, String& packets) {
  int16_t startOffset = 0;
  String subStr;
  String name2 = "/d/"+name;

  saveStringToFlash(name2.c_str(), packets);

  while (startOffset != -1) {
    startOffset = getPacketSubstring(packets, subStr, startOffset);
    if (startOffset != -1) {
      sendPacket(subStr);
    }
  }
}

void updateAllCams(String& packets) {
  int16_t startOffset = 0;
  String subStr;

  saveStringToFlash("/d/cams", packets);

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


