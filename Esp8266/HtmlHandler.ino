// Send contents of a SPIFFS file to the client (browser viewing the page)
// This function uses a buffer to control the amount of memory used here
// The size of this buffer has a big impact on performance
void printFile(const char *fileName) {
  const uint16_t bufSize = 1024;
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
  if (uri.indexOf("GET /updateAll ") != -1) {
    // Reload dynamic data
    static int val = 0;
    ++val;
    gClient.println(val);
  } else if (uri.indexOf("GET /button0 ") != -1) {
    // Button pressed
        
  } else if ((uri.indexOf("GET / HTTP/1.1") != -1) || (uri.indexOf("GET /index.html") != -1) ) {
    // Initial page load
    sendHtml(title);
  } else if (uri.indexOf("GET /favicon.ico") != -1) {
    // ignore this case
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
  CA_INFO("URI: ", uri.c_str());
  parseUri(uri, title);

  gClient.stop();
}

void sendHtml(const char* title) {
  gClient.println("HTTP/1.1 200 OK");
  gClient.println("Content-Type: text/html\r\n");
  gClient.println("<!DOCTYPE HTML> <HTML> <HEAD> <TITLE>");
  gClient.println(title);
  gClient.println("</TITLE> <meta charset=\"UTF-8\">");
  printFile("/css.html");
  //gClient.print(gCssString);

  //Serial.print(gScriptString);
//  uint32_t i = 0;
//  while(i<gScriptString.length()) {
//    gClient.print(gScriptString.substring(i,i+1024));
//    i+=1024;
//  }
  //gClient.print(gScriptString);
  printFile("/script.html");
 
  gClient.println("</HEAD>");
  gClient.println("<BODY>");
  gClient.println("<H1 id=\"title\"></H1>");
  gClient.println("<SCRIPT>");
  //gClient.print(gMenuString);
  printFile("/testMenu.html");
  gClient.println("</SCRIPT>");
  gClient.println("</BODY>");
  gClient.println("</HTML>");
}

