// Send contents of a SPIFFS file to the client (browser viewing the page)
// This function uses a buffer to control the amount of memory used here
// The size of this buffer has a big impact on performance
void printFile(const char *fileName) {
  const uint16_t bufSize = 512;
  static uint8_t buf[bufSize];
  int16_t sz;
  File f = SPIFFS.open(fileName, "r");

  if (!f) {
    if (!f.available()) {
      f.close();
    }
    CA_INFO("Failed to load file: ", fileName);
    return;
  }

  sz = f.size();
  do {
    if (sz > bufSize) {
      f.read(buf, bufSize);
      gClient.write((uint8_t*)buf, bufSize);\
      sz -= bufSize;
    }
    else {
      f.read(buf, sz);
      gClient.write((uint8_t*)buf, sz);
      sz = 0;
    }
  } while (sz);

  f.close();
}

void parseUri(String &uri, const char* title) {
  uri.replace("%20", " ");
  
  if (uri.indexOf("GET /updateDynamicState ") != -1) {
    String val;
    for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
      val += String("id") + gDynamicMessages.id[i] + "~" + gDynamicMessages.str[i] + "~";
    }
    gDynamicMessages.numMessages = 0;
    gClient.print(val);
  } else if (uri.indexOf("GET /updateMenuList ") != -1) {
    Dir dir = SPIFFS.openDir("/menus/");
        String str;
    while (dir.next()) {
      str += String(dir.fileName()).substring(7) + "~";
    }
    gClient.print(str);
  } else if (uri.indexOf("GET /updateDynamicMenu") != -1){
      String str;
      uri.replace("GET /updateDynamicMenu", "");
      uri.replace(" HTTP/1.1", "");
      str = String("/menus/") + uri;
      printFile(str.c_str());  // Sends file with dynamic menu to javascript
  } else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    // Initial page load
    sendHtml(title);
  } else if (uri.indexOf("GET /favicon.ico") != -1) {
    // ignore this case
  } else if (uri.indexOf("~") != -1) {    // This is the dynamic state case
    String packetStr = uri.substring(5, uri.length()-9);
    sendPacket(packetStr);
  } else {
    CA_INFO("ERROR - Unknown URI - ", uri);
  }
}

void processHtml(const char* title) {
  gClient = gServer.available();
  if (!gClient || !gClient.connected()) {
    return;
  }

  String uri = gClient.readStringUntil('\r');
  gClient.flush();
  parseUri(uri, title);

  gClient.stop();
}

void sendHtml(const char* title) {
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  printFile("/Index.html");
}

