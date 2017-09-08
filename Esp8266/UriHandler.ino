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
  gClient.flush();
  uri.replace("%20", " "); // The uri code converts spaces to %20, this undoes that transformation

  if (uri.indexOf("GET /updateDynamicState ") != -1) {
    updateDynamicState();
  }
  else if (uri.indexOf("GET /updateMenuList ") != -1) {
    updateMenuList();
  }
  else if (uri.indexOf("GET /updateDynamicMenu") != -1){
    updateDynamicMenu(uri);
  }
  else if (uri.indexOf("GET /updateAllCams") != -1) {
    String str = uri.substring(18, uri.length()-9);
    updateAllCams(str);
  }
  else if (uri.indexOf("GET /updateSaveStateInterval") != -1) {
    sendFileToClient("/intervalData");
  }
  else if (uri.indexOf("GET /updateSaveStateCams") != -1) {
    sendFileToClient("/camsData");
  }
  else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    loadMainWebPage();
  }
  else if (uri.indexOf("GET /favicon.ico") != -1) {
    // ignore this case
  }
  else if (uri.indexOf("~") != -1) {    // This is the dynamic state case
    String packetStr = uri.substring(5, uri.length()-9);
    sendPacket(packetStr);
  }
  else {
    CA_INFO("ERROR - Unknown URI - ", uri);
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

void updateDynamicMenu(String& uri) {
  String str;
  uri.replace("GET /updateDynamicMenu", "");
  uri.replace(" HTTP/1.1", "");
  str = String("/menus/") + uri;
  sendFileToClient(str.c_str());  // Sends file with dynamic menu to javascript
}

void loadMainWebPage() {
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  sendFileToClient("/Index.html");
}

void updateAllCams(String& packets) {
  int16_t startOffset = 0;
  String subStr;

  saveStringToFlash("/camsData", packets);

  while (startOffset != -1) {
    startOffset = getPacketSubstring(packets, subStr, startOffset);
    sendPacket(subStr);
  }
}
int16_t getPacketSubstring(const String &str, String& subStr, int16_t startOffset) {
  int16_t pos = str.indexOf('&', startOffset);
  if (pos != -1) {
    subStr = str.substring(startOffset, pos);  // Drop the trailing &
    pos++;
  }
  else {
    subStr = str.substring(startOffset);
  }
  return pos;
}


