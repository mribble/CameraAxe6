typedef struct {
  uint32 nextSendUpdate;
  hwPortPin ppSound;
  uint16 triggerVal;
} MenuSoundData;

MenuSoundData gMenuSoundData;

void MenuSound_Info() {
  g_ctx.packetHelper.writePacketMenuList(1, 2, 0x1, 0, 0x0, 0, 0x0, 0, 0x0, 0, 0x0, 0, 0x0, "Sound Menu");
}

void MenuSound_MenuInit() {
  const uint8 sDataMenu[] PROGMEM = {
  13,0,PID_MENU_HEADER,0,0,1,0,'S','o','u','n','d',0,  // MENU_HEADER 0 1 "Sound"
  38,0,PID_EDIT_NUMBER,0,0,3,0,0,0,0,255,1,0,0,100,0,0,0,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',0,  // EDIT_NUMBER 0 3 0 0 511 100 "Sound Trigger Level"  **gClientHostId_0**
  37,0,PID_TEXT_DYNAMIC,1,0,'C','u','r','r','e','n','t',' ','S','o','u','n','d',' ','L','e','v','e','l',' ','(','0','-','5','1','1',')',0,'?','?','?',0,  // TEXT_DYNAMIC 0 "Current Sound Level (0-511)" "???"  **gClientHostId_1**
  3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 91

  gMenuSoundData.nextSendUpdate = 0;
  gMenuSoundData.ppSound = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppSound, ANALOG_INPUT);
  g_ctx.packetHelper.writeMenu(sDataMenu, 91);
}

void MenuSound_PhotoInit() {
  const uint8 sDataActive[] PROGMEM = {
  22,0,PID_MENU_HEADER,0,0,1,0,'S','o','u','n','d',' ','-',' ','A','c','t','i','v','e',0,  // MENU_HEADER 0 1 "Sound - Active"
  30,0,PID_TEXT_DYNAMIC,2,0,'S','o','u','n','d',' ','T','r','i','g','g','e','r',' ','L','e','v','e','l',':',0,'?','?','?',0,  // TEXT_DYNAMIC 0 "Sound Trigger Level:" "???"  **gClientHostId_2**
  24,0,PID_TEXT_DYNAMIC,3,0,'C','u','r','r','e','n','t',' ','L','e','v','e','l',':',0,'?','?','?',0,  // TEXT_DYNAMIC 0 "Current Level:" "???"  **gClientHostId_3**
  3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 79

  gMenuSoundData.nextSendUpdate = 0;
  gMenuSoundData.ppSound = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppSound, ANALOG_INPUT);
  g_ctx.packetHelper.writeMenu(sDataActive, 79);
}

void MenuSound_MenuRun() {
  uint32 updateFrequency = 500;  // 500 ms
  uint32 curTime = millis();
  uint32 nextUpdate = gMenuSoundData.nextSendUpdate;
  
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*2)) { // Handles wraparounds
    uint16 val = CAU::analogRead(gMenuSoundData.ppSound);
    g_ctx.packetHelper.writePacketTextDynamic(1, 0, String(val).c_str());
    gMenuSoundData.nextSendUpdate = curTime;
  }
}

void MenuSound_PhotoRun() {
  uint16 triggerVal = gMenuSoundData.triggerVal;
  uint16 val = CAU::analogRead(gMenuSoundData.ppSound);
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

