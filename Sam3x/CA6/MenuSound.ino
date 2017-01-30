
void MenuSoundInit(uint8 modId)
{
  const uint8 sDataMenu[] PROGMEM = {
  13,0,PID_MENU_HEADER,0,0,1,0,'S','o','u','n','d',0,  // MENU_HEADER 0 1 "Sound"
  38,0,PID_EDIT_NUMBER,0,0,3,0,0,0,0,255,1,0,0,100,0,0,0,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',0,  // EDIT_NUMBER 0 3 0 0 511 100 "Sound Trigger Level"  **gClientHostId_0**
  37,0,PID_TEXT_DYNAMIC,1,0,'C','u','r','r','e','n','t',' ','S','o','u','n','d',' ','L','e','v','e','l',' ','(','0','-','5','1','1',')',' ','?','?','?',0,  // TEXT_DYNAMIC 0 "Current Sound Level (0-511) ???"  **gClientHostId_1**
  3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 91

  const uint8 sDataActive[] PROGMEM = {
  22,0,PID_MENU_HEADER,0,0,1,0,'S','o','u','n','d',' ','-',' ','A','c','t','i','v','e',0,  // MENU_HEADER 0 1 "Sound - Active"
  28,0,PID_TEXT_DYNAMIC,2,0,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',':',' ','?',0,  // TEXT_DYNAMIC 0 "Sound Trigger Level: ?"  **gClientHostId_2**
  22,0,PID_TEXT_DYNAMIC,3,0,'C','u','r','r','e','n','t',' ','L','e','v','e','l',':',' ','?',0,  // TEXT_DYNAMIC 0 "Current Level: ?"  **gClientHostId_3**
  3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 75
  
  g_ctx.modules[modId].modStore.menuSoundData.nextSendUpdate = 0;
  CAU::pinMode(g_ctx.modules[modId].modStore.menuSoundData.ppSound, ANALOG_INPUT);
  g_ctx.modules[modId].modStore.menuSoundData.ppSound = CAU::getModulePin(modId, 0);
  uint8 *data = 0;
  g_ctx.packetHelper.writeMenu(sDataMenu, 91);
}

// Sends packets to host
void MenuSoundSendPackets(uint8 modId)
{
  if (!g_ctx.active)
  {
    uint32 updateFrequency = 100;  // 100 ms
    uint32 curTime = millis();
    uint32 nextUpdate = g_ctx.modules[modId].modStore.menuSoundData.nextSendUpdate;
    
    if ((curTime >= nextUpdate) && (curTime+updateFrequency >= nextUpdate)) // Handles wraparounds
    {
      uint16 val = CAU::analogRead(g_ctx.modules[modId].modStore.menuSoundData.ppSound);
      // todo convert val to string
      // todo send packet to rfduino
      g_ctx.modules[modId].modStore.menuSoundData.nextSendUpdate = curTime;
    }
  }
  else
  {
    //todo
  }
}

// Receives packets from host
void MenuSoundReceivePackets(uint8 modId, uint8 *packet)
{
  if (!g_ctx.active)
  {
    if (packet[0] == 0) // first byte must be clientHostId
    {
      //CAPacket packetProcessor;
      //PacketEditNumber packEditNumber;
      //packetProcessor.unpackEditNumber(packet, &packEditNumber);
      //g_ctx.modules[modId].modStore.menuSoundData.triggerVal = packEditNumber.value;
    }
    else
    {
      CA_ASSERT(0, "Not a valid clientHostId");
    }
  }
}

void MenuSoundActiveInit(uint8 modId)
{
  
}

// Handle the actual triggering
uint8 MenuSoundTriggerCheck(uint8 modId)
{
  uint16 triggerVal = g_ctx.modules[modId].modStore.menuSoundData.triggerVal;
  uint16 val = CAU::analogRead(g_ctx.modules[modId].modStore.menuSoundData.ppSound);
  uint8 ret;
  
  if (val > 512)
  {
    ret = (val-512) > triggerVal ? CA_TRUE : CA_FALSE;
  }
  else
  {
    ret = val > triggerVal ? CA_TRUE : CA_FALSE;
  }
}

