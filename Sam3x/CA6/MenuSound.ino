
void MenuSoundInit(uint8 modId)
{
  const uint8 sData[] PROGMEM = {
  PID_MENU_HEADER,1,2,'S','o','u','n','d',0,  // MENU_HEADER 1 2 "Sound"
  PID_NEW_ROW,  // NEW_ROW
  PID_NEW_CELL_LEFT,70,  // NEW_CELL_LEFT 70
  PID_TEXT_STATIC,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',0,  // TEXT_STATIC "Sound Trigger Level"
  PID_NEW_CELL_RIGHT,30,  // NEW_CELL_RIGHT 30
  PID_EDIT_NUMBER,0,3,0,0,0,0,255,1,0,0,100,0,0,0,  // EDIT_NUMBER 3 0 0 511 100  **gClientHostId_0**
  PID_NEW_ROW,  // NEW_ROW
  PID_NEW_CELL_LEFT,70,  // NEW_CELL_LEFT 70
  PID_TEXT_STATIC,'C','u','r','r','e','n','t',' ','S','o','u','n','d',' ','L','e','v','e','l',' ','(','0','-','5','1','1',')',0,  // TEXT_STATIC "Current Sound Level (0-511)"
  PID_NEW_CELL_RIGHT,30,  // NEW_CELL_RIGHT 30
  PID_TEXT_DYNAMIC,1,'?','?','?',0,  // TEXT_DYNAMIC "???"  **gClientHostId_1**
  PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 91
  
  const uint8 sDataActive[] PROGMEM = {
  PID_MENU_HEADER,1,2,'S','o','u','n','d',' ','-',' ','A','c','t','i','v','e',0,  // MENU_HEADER 1 2 "Sound - Active"
  PID_NEW_ROW,  // NEW_ROW
  PID_NEW_CELL_LEFT,100,  // NEW_CELL_LEFT 100
  PID_TEXT_STATIC,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',':',' ',0,  // TEXT_STATIC "Sound Trigger Level: "
  PID_TEXT_DYNAMIC,0,'?',0,  // TEXT_DYNAMIC "?"  **gClientHostId_0**
  PID_NEW_ROW,  // NEW_ROW
  PID_NEW_CELL_LEFT,100,  // NEW_CELL_LEFT 100
  PID_TEXT_STATIC,'C','u','r','r','e','n','t',' ','L','e','v','e','l',':',' ',0,  // TEXT_STATIC "Current Level: "
  PID_TEXT_DYNAMIC,1,'?',0,  // TEXT_DYNAMIC "?"  **gClientHostId_1**
  PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 73
  
  g_ctx.modules[modId].modStore.menuSoundData.nextSendUpdate = 0;
  g_ctx.modules[modId].modStore.menuSoundData.ppSound = CAU::getModulePin(modId, 0);
  CAU::pinMode(g_ctx.modules[modId].modStore.menuSoundData.ppSound, ANALOG_INPUT);
  
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
      CA_ASSERT(0, "Not a valid cclientHostId");
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

