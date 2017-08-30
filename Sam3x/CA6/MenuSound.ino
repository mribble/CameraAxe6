typedef struct {
  uint32_t nextSendUpdate;
  hwPortPin ppSound;
  uint32_t triggerVal;
} MenuSoundData;

MenuSoundData gMenuSoundData;

const char* MenuSound_Name() {
  return "Sound Menu";
}

void MenuSound_MenuInit() {
  gMenuSoundData.nextSendUpdate = millis();
  gMenuSoundData.ppSound = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppSound, ANALOG_INPUT);
}

void MenuSound_PhotoInit() {
  gMenuSoundData.nextSendUpdate = millis();
  gMenuSoundData.ppSound = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppSound, ANALOG_INPUT);
}

void MenuSound_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  uint32_t nextUpdate = gMenuSoundData.nextSendUpdate;

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gMenuSoundData.triggerVal);
  incomingPacketFinish(packet);

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    uint16_t val = CAU::analogRead(gMenuSoundData.ppSound);
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
    gMenuSoundData.nextSendUpdate = curTime + updateFrequency;
  }
}

void MenuSound_PhotoRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  uint32_t nextUpdate = gMenuSoundData.nextSendUpdate;
  uint16_t triggerVal = gMenuSoundData.triggerVal;
  uint16_t val = CAU::analogRead(gMenuSoundData.ppSound);

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  incomingPacketFinish(packet);

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    g_ctx.packetHelper.writePacketString(0, String(val).c_str());
    gMenuSoundData.nextSendUpdate = curTime + updateFrequency;
  }

  // Handle triggering
  uint8_t trigger = (val >= triggerVal) ? true : false;
  if (trigger) {
    triggerCameras();
    CA_LOG("Trigger\n");
  }
  
}

