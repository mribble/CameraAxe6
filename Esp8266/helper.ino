////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Handles various helper functions dealing with flash file system (spiffs)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send contents of a SPIFFS file to the client (webpage)
//   The reason we break up writes is because the full webpage is many KBs (over 30 KB at this point) and it would not
//   be good to load that full page into RAM so instead we break it into chuncks
//   The size of the buffer has a big impact on perf (small buffers like 64 bytes make page loading slow)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendFileToClient(const char *fileName) {
  const int32_t bufSize = 1024;
  uint8_t buf[bufSize];
  int32_t sz;

  CA_LOG(CA_ESP_SPIFFS, "SPIFFS sendFileToClient: %s\n", fileName);
  File f = SPIFFS.open(fileName, "r");

  if (!f) {
    gClient.print("null");
    return;
  }

  sz = f.size();
  do {
    if (sz > bufSize) {
      // This is the case where we need to break up the writes
      f.read(buf, bufSize);
      gClient.write((uint8_t*)buf, bufSize);
      sz -= bufSize;
    }
    else {
      // This is the case where we can write the full/rest buffer
      f.read(buf, sz);
      gClient.write((uint8_t*)buf, sz);
      sz = 0;
    }
  } while (sz);

  f.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copies the contents of a file in flash memory to a string
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
boolean readFileToString(const char* fileName, String &str) {
  File f = SPIFFS.open(fileName, "r");

  CA_LOG(CA_ESP_SPIFFS, "SPIFFS readFileToString: %s\n", fileName);
  if (!f) {
    return false;
  }

  //str = f.readStringUntil(0);  // This is simpler version of code below, but is slow

  const uint16_t bufSize = 256;
  static uint8_t buf[bufSize];
  int16_t sz;
  
  sz = f.size();
  do {
    if (sz > bufSize-1) {
      f.read(buf, bufSize-1);
      buf[bufSize-1] = 0;
      str += (char*)buf;
      sz -= bufSize-1;
    }
    else {
      f.read(buf, sz);
      buf[sz] = 0;
      str += (char*)buf;
      sz = 0;
    }
  } while (sz);

  f.close();
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Writes a string to flash memory
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void saveStringToFlash(const char* fileName, String& packetStr) {
  bool match = false;
  String fileStr;

  // Read contents and compare to avoid needless flash writes (flash has a limited number of writes -- generally ~50K)

  CA_LOG(CA_ESP_SPIFFS, "SPIFFS saveStringToFlash: %s\n", fileName);
  readFileToString(fileName, fileStr);
  if (fileStr == packetStr) {
    CA_LOG(CA_ESP_SPIFFS, " SPIFFS saveStringToFlash: match\n");
    match = true;
  }
  
  if (!match) {
    CA_LOG(CA_ESP_SPIFFS, " SPIFFS saveStringToFlash: write\n");
    File f = SPIFFS.open(fileName, "w");
    if (f) {
      f.print(packetStr);
      f.close();
    } else {
      CA_LOG(CA_ERROR, "Failed to load file: %s\n", fileName);
    }
  }
}
