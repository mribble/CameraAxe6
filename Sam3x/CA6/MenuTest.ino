typedef struct {
  uint32_t nextSendUpdate;
  uint32_t count;
} MenuTestData;

MenuTestData gMenuTestData;

void MenuTest_Info() {
  //todo
}

void MenuTest_MenuInit() {
  gMenuTestData.nextSendUpdate = millis();
  gMenuTestData.count = 0;
  // This menu has no IO to setup
}

void MenuTest_PhotoInit() {
  // This menu has no IO to setup
}

void MenuTest_MenuRun() {
  uint32_t updateFrequency = 1000;  // 1000 ms
  uint32_t curTime = millis();
  static bool firstRun = true;

  // remove once we get init hooked up
  if (firstRun) {
    gMenuTestData.nextSendUpdate = millis();
    gMenuTestData.count = 0;
    firstRun = false;
  }
  uint32_t nextUpdate = gMenuTestData.nextSendUpdate;

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  uint32_t val;
  packet = incomingPacketCheckUint32(packet, 4, val);
  incomingPacketFinish(packet);

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*256)) { // Handles wraparounds
    ++gMenuTestData.count;
    g_ctx.packetHelper.writePacketString(0, String(gMenuTestData.count).c_str());
    gMenuTestData.nextSendUpdate = curTime + updateFrequency;
  }
}

void MenuTest_PhotoRun() {
  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  incomingPacketFinish(packet);

  // This menu does nothing
}


