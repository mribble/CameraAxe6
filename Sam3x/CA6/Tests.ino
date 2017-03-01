void caRunTests()
{
  //caTestSerialWritePerf();
  //caTestNetworkEcho();
  //caTestTickTimer();
  //caTestPackets();
  //caTestBlinkLed();
  //caTestPerf();
  //caTestModulePorts();
  //caTestLinkAndCamPorts();
  //caTestAuxPort();
  //caTestEeprom();
  //caTestAnalog();
}

void caTestSerialWritePerf() {
  const uint32 bufSize = 20;
  uint32 startTime = 0;
  uint32 endTime = 0;
  uint8 writeChars[bufSize];

  Serial1.end();
  Serial1.begin(9600);

  // fill the write buffer
  for (uint8 i = 0; i < bufSize; i++) {
    writeChars[i] = char((i % 10) + '0');
  }

  startTime = micros();
  Serial1.write(writeChars, bufSize);
  endTime = micros();
  delay(500); // Wait for values to finish being written

  CAU::log("Serial Write perf (%d bytes): %d us\n", bufSize, endTime-startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestNetworkEcho - Tests performance of a round trip network
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestNetworkEcho() {
  bool done = false;

  g_ctx.echoReceived = 0;

  uint32 endTime;
  uint32 startTime = millis();
  g_ctx.packetHelper.writePacketEcho(0, "01234");  // 0 means back to sam3x
  for(uint16 i=0; i<5000; ++i) {
    processIncomingPacket();
    if (g_ctx.echoReceived) {
      endTime = millis();
      done = true;
      break;
    }
    delay(1);
  }

  if (done) {
    CAU::log("Time in milliseconds: %d\n", endTime-startTime);
  } else {
    CAU::log("ERROR - Packet was never returned.\n");
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestTickTimer - Tests the CATickTimer class
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestTickTimer()
{
  uint64 ticks;
  CATickTimer bob(0);
  ticks = bob.convertTimeToTicks(0,0,0,100,0,0);
  bob.start(toggleLed, ticks, CA_TRUE);
  delay(5000);
  bob.stop();
}
void toggleLed()
{
  static uint8 toggle = 1;

  hwPortPin led = CAU::getOnboardDevicePin(LED_GREEN_PIN);
  CAU::pinMode(led, OUTPUT);

  if (toggle)
  {
    CAU::digitalWrite(led, HIGH);
    toggle = 0;
  }
  else
  {
    CAU::digitalWrite(led, LOW);
    toggle = 1;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestPackets - Tests packing and unpacking of data packets
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestPackets()
{
  const uint8 sDataMenu[] PROGMEM = {
  20,0,PID_MENU_HEADER,1,0,2,0,'U','I',' ','T','e','s','t',' ','M','e','n','u',0,  // MENU_HEADER 1 2 "UI Test Menu"
  15,0,PID_TEXT_STATIC,'S','t','a','t','i','c',' ','T','e','x','t',0,  // TEXT_STATIC "Static Text"
  22,0,PID_TEXT_DYNAMIC,0,0,'D','y','n','a','m','i','c',' ','T','e','x','t',0,'1','2','3',0,  // TEXT_DYNAMIC 0 "Dynamic Text" "123"  **gClientHostId_0**
  30,0,PID_BUTTON,1,0,17,'T','h','i','s',' ','i','s',' ','a',' ','b','u','t','t','o','n',0,'B','u','t','t','o','n',0,  // BUTTON 0 1 1 "This is a button" "Button"  **gClientHostId_1**
  26,0,PID_CHECK_BOX,2,1,0,'T','h','i','s',' ','i','s',' ','a',' ','c','h','e','c','k',' ','b','o','x',0,  // CHECK_BOX 1 0 "This is a check box"  **gClientHostId_2**
  35,0,PID_DROP_SELECT,3,2,1,'T','h','i','s',' ','i','s',' ','a',' ','d','r','o','p',' ','s','e','l','e','c','t',0,'n','o','|','y','e','s',0,  // DROP_SELECT 2 1 "This is a drop select" "no|yes"  **gClientHostId_3**
  41,0,PID_EDIT_NUMBER,4,0,50,0,0,0,0,159,134,1,0,80,195,0,0,'T','h','i','s',' ','i','s',' ','a','n',' ','e','d','i','t',' ','n','u','m','b','e','r',0,  // EDIT_NUMBER 0 2 3 0 99999 50000 "This is an edit number"  **gClientHostId_4**
  32,0,PID_TIME_BOX,5,0,223,24,59,122,62,125,144,1,'T','h','i','s',' ','i','s',' ','a',' ','t','i','m','e',' ','b','o','x',0,  // TIME_BOX 0 1 1 1 1 1 0 99 59 40 999 500 400 "This is a time box"  **gClientHostId_5**
  3,0,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 224
  
  const uint8 BUF_SIZE = 224;
  uint8 data[BUF_SIZE];
  const uint8 *dataPtr = data;
  uint8 totalUnpackSize=0;
  // Move progmem data to a buffer for this test
  for(uint16 i=0; i<BUF_SIZE; ++i)
  {
   data[i] = pgm_read_byte_near(sDataMenu+i);
  }

  uint8 dataA[512];
  uint8 packType;
  uint16 packSize, unpackSize;
  memset(dataA, 0, 512);
  CAPacket unpack0(STATE_UNPACKER, data, BUF_SIZE);
  CAPacket pack0(STATE_PACKER, dataA, 512);

  {  // MENU_HEADER 1 2 "UI Test Menu"
    CAPacketMenuHeader unpack1(unpack0);          // Update per type
    CAPacketMenuHeader pack1(pack0);              // Update per type
  
    pack1.set(1, 2, "UI Test Menu");              // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_MENU_HEADER ||            // Update per type
        unpack1.getMajorVersion() != 1 ||
        unpack1.getMinorVersion() != 2 ||
        strcmp(unpack1.getMenuName(), "UI Test Menu") != 0) {
      CAU::log("ERROR - MENU_HEADER test failed\n");
    }
  }

  {  // TEXT_STATIC "Static Text"
    CAPacketTextStatic unpack1(unpack0);          // Update per type
    CAPacketTextStatic pack1(pack0);              // Update per type
  
    pack1.set("Static Text");                     // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_TEXT_STATIC ||            // Update per type
        strcmp(unpack1.getText0(), "Static Text") != 0) {
      CAU::log("ERROR - TEXT_STATIC test failed\n");
    }
  }

  {  // TEXT_DYNAMIC 0 "Dynamic Text" "123"  **gClientHostId_0**
    CAPacketTextDynamic unpack1(unpack0);         // Update per type
    CAPacketTextDynamic pack1(pack0);             // Update per type
  
    pack1.set(0, 0, "Dynamic Text", "123");       // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_TEXT_DYNAMIC ||           // Update per type
        unpack1.getClientHostId() != 0 ||
        unpack1.getModAttribute() != 0 ||
        strcmp(unpack1.getText0(), "Dynamic Text") != 0 ||
        strcmp(unpack1.getText1(), "123") != 0) {
      CAU::log("ERROR - TEXT_DYNAMIC test failed\n");
    }
  }

  { // BUTTON 0 1 1 "This is a button" "Button"  **gClientHostId_1**
    CAPacketButton unpack1(unpack0);              // Update per type
    CAPacketButton pack1(pack0);                  // Update per type
  
    pack1.set(1, 0, 1, 1, "This is a button", "Button"); // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_BUTTON ||                 // Update per type
        unpack1.getClientHostId() != 1 ||
        unpack1.getModAttribute() != 0 ||
        unpack1.getType() != 1 ||
        unpack1.getValue() != 1 ||
        strcmp(unpack1.getText0(), "This is a button") != 0 ||
        strcmp(unpack1.getText1(), "Button") != 0) {
      CAU::log("ERROR - BUTTON test failed\n");
    }
  }

  {  // CHECK_BOX 1 0 "This is a check box"  **gClientHostId_2**
    CAPacketCheckBox unpack1(unpack0);            // Update per type
    CAPacketCheckBox pack1(pack0);                // Update per type
  
    pack1.set(2, 1, 0, "This is a check box");    // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_CHECK_BOX ||              // Update per type
        unpack1.getClientHostId() != 2 ||
        unpack1.getModAttribute() != 1 ||
        unpack1.getValue() != 0 ||
        strcmp(unpack1.getText0(), "This is a check box") != 0) {
      CAU::log("ERROR - CHECK_BOX test failed\n");
    }
  }

  {  // DROP_SELECT 2 1 "This is a drop select" "no|yes"  **gClientHostId_3**
    CAPacketDropSelect unpack1(unpack0);          // Update per type
    CAPacketDropSelect pack1(pack0);              // Update per type
  
    pack1.set(3, 2, 1, "This is a drop select", "no|yes"); // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_DROP_SELECT ||            // Update per type
        unpack1.getClientHostId() != 3 ||
        unpack1.getModAttribute() != 2 ||
        unpack1.getValue() != 1 ||
        strcmp(unpack1.getText0(), "This is a drop select") != 0 ||
        strcmp(unpack1.getText1(), "no|yes") != 0) {
      CAU::log("ERROR - DROP_SELECT test failed\n");
    }
  }

  {  // EDIT_NUMBER 0 2 3 0 99999 50000 "This is an edit number"  **gClientHostId_4**
    CAPacketEditNumber unpack1(unpack0);          // Update per type
    CAPacketEditNumber pack1(pack0);              // Update per type
  
    pack1.set(4, 0, 2, 3, 0, 99999, 50000, "This is an edit number"); // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_EDIT_NUMBER ||            // Update per type
        unpack1.getClientHostId() != 4 ||
        unpack1.getModAttribute() != 0 ||
        unpack1.getDigitsBeforeDecimal() != 2 ||
        unpack1.getDigitsAfterDecimal() != 3 ||
        unpack1.getMinValue() != 0 ||
        unpack1.getMaxValue() != 99999 ||
        unpack1.getValue() != 50000 ||
        strcmp(unpack1.getText0(), "This is an edit number") != 0) {
      CAU::log("ERROR - EDIT_NUMBER test failed\n");
    }
  }

  {  // TIME_BOX 0 1 1 1 1 1 0 99 59 40 999 500 400 "This is a time box"  **gClientHostId_5**
    CAPacketTimeBox unpack1(unpack0);             // Update per type
    CAPacketTimeBox pack1(pack0);                 // Update per type
  
    pack1.set(5, 0, 0x1f, 99, 59, 40, 999, 500, 400, "This is a time box"); // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_TIME_BOX ||               // Update per type
        unpack1.getClientHostId() != 5 ||
        unpack1.getModAttribute() != 0 ||
        unpack1.getEnableMask() != 0x1f ||
        unpack1.getHours() != 99 ||
        unpack1.getMinutes() != 59 ||
        unpack1.getSeconds() != 40 ||
        unpack1.getMilliseconds() != 999 ||
        unpack1.getMicroseconds() != 500 ||
        unpack1.getNanoseconds() != 400 ||
        strcmp(unpack1.getText0(), "This is a time box") != 0) {
      CAU::log("ERROR - TIME_BOX test failed\n");
    }
  }

  {  // SCRIPT_END
    CAPacketScriptEnd unpack1(unpack0);           // Update per type
    CAPacketScriptEnd pack1(pack0);               // Update per type
  
    //pack1.set();                                // Update per type
    unpackSize = unpack0.unpackSize();
    packType = unpack0.unpackType();
    //unpack1.unpack();
    packSize = pack1.pack();
    totalUnpackSize += unpackSize;
    if (memcmp(data, dataA, totalUnpackSize) != 0 ||
        packSize != unpackSize ||
        packType != PID_SCRIPT_END) {             // Update per type
      CAU::log("ERROR - SCRIPT_END test failed\n");
    }
  }

  // The following are packet types that don't exist in the script files
  memset(dataA, 0, 256);
  CAPacket unpack10(STATE_UNPACKER, dataA, 256);
  CAPacket pack10(STATE_PACKER, dataA, 256);

  { // MENU_SELECT Packet Test
    CAPacketMenuSelect unpack11(unpack10);        // Update per type
    CAPacketMenuSelect pack11(pack10);            // Update per type
    
    pack11.set(1, 23);                            // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_MENU_SELECT ||          // Update per type
          unpack11.getMode() != 1 ||
          unpack11.getMenuNumber() != 23) {
      CAU::log("ERROR - MENU_SELECT test failed\n");
    } 
  }

  { // MENU_LIST Packet Test
    CAPacketMenuList unpack11(unpack10);          // Update per type
    CAPacketMenuList pack11(pack10);              // Update per type
    
    pack11.set(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, "menuList"); // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_MENU_LIST ||            // Update per type
          unpack11.getMenuId() != 1 ||
          unpack11.getModuleId0() != 2 ||
          unpack11.getModuleMask0() != 3 ||
          unpack11.getModuleId1() != 4 ||
          unpack11.getModuleMask1() != 5 ||
          unpack11.getModuleId2() != 6 ||
          unpack11.getModuleMask2() != 7 ||
          unpack11.getModuleId3() != 8 ||
          unpack11.getModuleMask3() != 9 ||
          unpack11.getModuleTypeId0() != 10 ||
          unpack11.getModuleTypeMask0() != 11 ||
          unpack11.getModuleTypeId1() != 12 ||
          unpack11.getModuleTypeMask1() != 13 ||
          strcmp(unpack11.getMenuName(), "menuList") != 0) {
      CAU::log("ERROR - MENU_LIST test failed\n");
    } 
  }

  { // MODULE_LIST Packet Test
    CAPacketModuleList unpack11(unpack10);        // Update per type
    CAPacketModuleList pack11(pack10);            // Update per type
    
    pack11.set(33, 44, "moduleList");             // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_MODULE_LIST ||          // Update per type
          unpack11.getModuleId() != 33 ||
          unpack11.getModuleTypeId() != 44 ||
          strcmp(unpack11.getModuleName(), "moduleList") != 0) {
      CAU::log("ERROR - MODULE_LIST test failed\n");
    } 
  }
  
  { // LOGGER Packet Test
    CAPacketLogger unpack11(unpack10);            // Update per type
    CAPacketLogger pack11(pack10);                // Update per type
    
    pack11.set("This is a log");                  // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_LOGGER ||               // Update per type
          strcmp(unpack11.getLog(), "This is a log") != 0) {
      CAU::log("ERROR - LOG test failed\n");
    } 
  }

  { // CAM_STATE Packet Test
    CAPacketCamState unpack11(unpack10);          // Update per type
    CAPacketCamState pack11(pack10);              // Update per type
    
    pack11.set(2, 0xc1, 0xf0);                    // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_CAM_STATE ||            // Update per type
          unpack11.getMultiplier() != 2 ||
          unpack11.getFocus() != 0xc1 ||
          unpack11.getShutter() != 0xf0) {
      CAU::log("ERROR - CAM_STATE test failed\n");
    } 
  }

  { // CAM_SETTINGS Packet Test
    CAPacketCamSettings unpack11(unpack10);       // Update per type
    CAPacketCamSettings pack11(pack10);           // Update per type
    
    pack11.set(50, 1, 999, 59, 58, 998, 997, 996, 57, 56, 995, 994, 0xbe, 1, 5, 1, 40, 41, 900);  // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_CAM_SETTINGS ||       // Update per type
          unpack11.getCamPortNumber() != 50 ||
          unpack11.getMode() != 1 ||
          unpack11.getDelayHours() != 999 ||
          unpack11.getDelayMinutes() != 59 ||
          unpack11.getDelaySeconds() != 58 ||
          unpack11.getDelayMilliseconds() != 998 ||
          unpack11.getDelayMicroseconds() != 997 ||
          unpack11.getDurationHours() != 996 ||
          unpack11.getDurationMinutes() != 57 ||
          unpack11.getDurationSeconds() != 56 ||
          unpack11.getDurationMilliseconds() != 995 ||
          unpack11.getDurationMicroseconds() != 994 ||
          unpack11.getSequencer() != 0xbe ||
          unpack11.getApplyIntervalometer() != 1 ||
          unpack11.getSmartPreview() != 5 ||
          unpack11.getMirrorLockupEnable() != 1 ||
          unpack11.getMirrorLockupMinutes() != 40 ||
          unpack11.getMirrorLockupSeconds() != 41 ||
          unpack11.getMirrorLockupMilliseconds() != 900 ) {
      CAU::log("ERROR - CAM_SETTINGS test failed\n");
    } 
  }

  { // INTERVALOMETER Packet Test
    CAPacketIntervalometer unpack11(unpack10);    // Update per type
    CAPacketIntervalometer pack11(pack10);        // Update per type
    
    pack11.set(900, 50, 51, 901, 902, 903, 52, 53, 904, 905, 9999); // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_INTERVALOMETER ||            // Update per type
          unpack11.getStartHours() != 900 ||
          unpack11.getStartMinutes() != 50 ||
          unpack11.getStartSeconds() != 51 ||
          unpack11.getStartMilliseconds() != 901 ||
          unpack11.getStartMicroseconds() != 902 ||
          unpack11.getIntervalHours() != 903 ||
          unpack11.getIntervalMinutes() != 52 ||
          unpack11.getIntervalSeconds() != 53 ||
          unpack11.getIntervalMilliseconds() != 904 ||
          unpack11.getIntervalMicroseconds() != 905 ||
          unpack11.getRepeats() != 9999 ) {
      CAU::log("ERROR - INTERVALOMETER test failed\n");
    } 
  }

  { // CONTROL_FLAGS Packet Test
    CAPacketControlFlags unpack11(unpack10);      // Update per type
    CAPacketControlFlags pack11(pack10);          // Update per type
    
    pack11.set(1, 1);                             // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_CONTROL_FLAGS ||        // Update per type
          unpack11.getSlaveModeEnable() != 1 ||
          unpack11.getExtraMessagesEnable() != 1 ) {
      CAU::log("ERROR - CONTROL_FLAGS test failed\n");
    } 
  }

  { // ECHO Packet Test
    CAPacketEcho unpack11(unpack10);              // Update per type
    CAPacketEcho pack11(pack10);                  // Update per type
    
    pack11.set(1, "Echo Packet");                 // Update per type
    uint8 packSize = pack11.pack();
    uint8 unpackSize = unpack10.unpackSize();
    uint8 packType = unpack10.unpackType();
    unpack11.unpack();
    if (packSize != unpackSize ||
          packType != PID_ECHO ||                 // Update per type
          unpack11.getMode() != 1 ||
          strcmp(unpack11.getString(), "Echo Packet") != 0) {
      CAU::log("ERROR - ECHO test failed\n");
    } 
  }

  CAU::log("Done - testPackets\n");
  delay(2000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestBlinkLed - Test blinks LEDs.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestBlinkLed()
{
  hwPortPin led = CAU::getOnboardDevicePin(LED_GREEN_PIN);
  CAU::pinMode(led, OUTPUT);
  CAU::digitalWrite(led, HIGH);
  delay(1000);
  CAU::digitalWrite(led, LOW);
  delay(200);

  led = CAU::getOnboardDevicePin(LED_RED_PIN);
  CAU::pinMode(led, OUTPUT);
  CAU::digitalWrite(led, HIGH);
  delay(200);
  CAU::digitalWrite(led, LOW);
  delay(200);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestPerf - Various performance tests for low level CA6 functions
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestPerf()
{
  uint16 count = 0;
  uint8 val8=0;
  uint16 val16=0;
  uint32 startTime, endTime;
  hwPortPin pp0 = CAU::getModulePin(0, 2); // actually Mod0_SDA
  
  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    val8 = CAU::digitalRead(pp0);
  }
  endTime = micros();
  CAU::log("  10,000 digitalRead() = %dus\n", endTime-startTime);

  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    CAU::digitalWrite(pp0, LOW);
  }
  endTime = micros();
  CAU::log("  10,000 digitalWrite() = %dus\n", endTime-startTime);

  pp0 = CAU::getOnboardDevicePin(LV_DETECT_PIN);
  CAU::initializeAnalog();
  CAU::pinMode(pp0, ANALOG_INPUT);
  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    val16 = CAU::analogRead(pp0);
  }
  endTime = micros();
  CAU::log("  10,000 analogRead() = %dus\n", endTime-startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestModulePorts - Tests all the module ports.  Reports which ones failed. - Requires a special test dongle.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestModulePorts()
{
  uint8 module;
  uint8 pin;

  for(module=0; module<4; ++module)
  {
    for(pin=0; pin<6; pin+=2)
    {
      hwPortPin pp0, pp1;
      pp0 = CAU::getModulePin(module, pin);
      pp1 = CAU::getModulePin(module, pin+1);
      if (!caTestTwoPins(pp0, pp1))
      {
        CAU::log("  Module Port %d:%d failed\n", module, pin);
      }
      
      if (!caTestTwoPins(pp1, pp0))
      {
        CAU::log("  Module Port %d:%d failed\n", module, pin+1);
      }
    }
  }
  CAU::log("Done - module ports\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestLinkAndCamPorts - Tests link and camera ports.  Reports which ones failed. - Requires a special test dongle.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestLinkAndCamPorts()
{
  hwPortPin ppFocus, ppShutter;
  uint8 cam, i;
  hwPortPin linkFocus   = CAU::getLinkPin(FOCUS);
  hwPortPin linkShutter = CAU::getLinkPin(SHUTTER);
  uint8 val0, val1, val2, val3, val4, val5;
  
  for(cam=0; cam<8; ++cam)
  {
    caAllPortsLow(); // Must reset all cam ports here
    CAU::pinMode(linkFocus,   INPUT);
    CAU::pinMode(linkShutter, INPUT);

    delayMicroseconds(90);  // Without this delay optoisolator trigger time causes failure
    val0 = CAU::digitalRead(linkFocus);    // Should be high due to pullup resistor
    val1 = CAU::digitalRead(linkShutter);  // Should be high due to pullup resistor
    ppFocus   = CAU::getCameraPin(cam, FOCUS);
    ppShutter = CAU::getCameraPin(cam, SHUTTER);
    CAU::digitalWrite(ppFocus,   HIGH);
    CAU::digitalWrite(ppShutter, LOW);
    delayMicroseconds(90);  // Without this delay optoisolator trigger time causes failure
    val2 = CAU::digitalRead(linkFocus);    // Should be low
    val3 = CAU::digitalRead(linkShutter);  // Should be high due to pullup resistor
    CAU::digitalWrite(ppFocus,   LOW);
    CAU::digitalWrite(ppShutter, HIGH);
    delayMicroseconds(90);  // Without this delay optoisolator trigger time causes failure
    val4 = CAU::digitalRead(linkFocus);    // Should be high due to pullup resistor
    val5 = CAU::digitalRead(linkShutter);  // Should be low
  
    if ((val0 != HIGH) || (val1 != HIGH) ||
        (val2 != LOW)  || (val3 != HIGH) ||
        (val4 != HIGH) || (val5 != LOW))
    {
      CAU::log("  Camera/Link Port Cam:%d failed (%d,%d,%d,%d,%d,%d)\n", cam, val0, val1, val2, val3, val4, val5);
    }
  }
  
  CAU::log("Done - camera/linkports\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestLinkAndCamPorts - Tests the aux port.  Reports which pins failed. - Requires a special test dongle.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAuxPort()
{
  uint8 pin;
  hwPortPin pp0, pp1;

  for(pin=0; pin<46; pin+=2)
  {
    pp0 = CAU::getAuxPin(pin);
    pp1 = CAU::getAuxPin(pin+1);
    if (!caTestTwoPins(pp0, pp1))
    {
      CAU::log("  Aux Port %d failed\n", pin);
    }
    if (!caTestTwoPins(pp1, pp0))
    {
      CAU::log("  Aux Port %d failed\n", pin+1);
    }
  }
  
  CAU::log("Done - aux port\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestEeprom - Tests the eeprom
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestEeprom()
{
  char *str0 = "This is just a test string1.";
  char *str1 = "Blah blah blah blah 2.";
  uint8 buf[128];
  unioReadStatus statusReg;

  CAEeprom eeprom(CA_INTERNAL);
  
  if (!eeprom.statusWrite(EEPROM_WRITE_PROTECTED_ALL))
  {
    CAU::log("  Writing status register FAILED\n");
  }
  if(!eeprom.statusRead(&statusReg))
  {
    CAU::log("  Reading status register FAILED\n");
  }
  if (!eeprom.statusWrite(EEPROM_WRITE_PROTECTED_NONE))
  {
    CAU::log("  Writing status register 2 FAILED\n");
  }

  if (!eeprom.write((uint8*)str0, 0x20, strlen(str0)+1))
  {
    CAU::log("  Write1 FAILED\n");
  }
  memset(buf, 0, 128);
  if (!eeprom.read(buf, 0x20, strlen(str0)+1))
  {
    CAU::log("  Read1 FAILED\n");
  }
  if (!eeprom.write((uint8*)str1, 0x20, strlen(str1)+1))
  {
    CAU::log("  Write2 FAILED\n");
  }
  memset(buf, 0, 128);

  if (!eeprom.read(buf, 0x20, strlen(str1)+1))
  {
    CAU::log("  Read2 FAILED\n");
  }

  CAU::log("Done - eeprom\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestAnalog - Tests the different analog pins
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAnalog()
{
  uint16 val;
  hwPortPin ppAn;   // Analog Port
  hwPortPin ppDig;  // Port used to change analog's voltage level

  CAU::initializeAnalog();

  ppAn = CAU::getModulePin(0, 0);
  ppDig = CAU::getModulePin(0, 1);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(0, 1);
  ppDig = CAU::getModulePin(0, 0);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(0, 2);
  ppDig = CAU::getModulePin(0, 3);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(1, 0);
  ppDig = CAU::getModulePin(1, 1);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(2, 0);
  ppDig = CAU::getModulePin(2, 1);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(2, 1);
  ppDig = CAU::getModulePin(2, 0);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(2, 2);
  ppDig = CAU::getModulePin(2, 3);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(2, 4);
  ppDig = CAU::getModulePin(2, 5);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(2, 5);
  ppDig = CAU::getModulePin(2, 4);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getModulePin(3, 0);
  ppDig = CAU::getModulePin(3, 1);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(33);
  ppDig = CAU::getAuxPin(32);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(34);
  ppDig = CAU::getAuxPin(35);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(35);
  ppDig = CAU::getAuxPin(34);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(36);
  ppDig = CAU::getAuxPin(37);
  caTestAnalogPin(ppAn, ppDig);

  ppAn = CAU::getOnboardDevicePin(LV_DETECT_PIN);
  CAU::pinMode(ppAn, ANALOG_INPUT);
  val = CAU::analogRead(ppAn); // expected value is 5V/4 = 1.25V -- 1.25*4095/3.3 =1551
  if ((val < 1350) || (val > 1650))
  {
    CAU::log("  Failed Analog Test -- LV_DETECT_PIN %d\n", val);
  }
  
  CAU::log("Done - analog\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAnalogPin(hwPortPin ppAn, hwPortPin ppDig)
{
  uint valLow, valHigh;

  caAllPortsLow(); // Helps detect shorted pins
  CAU::pinMode(ppAn, ANALOG_INPUT);
  CAU::pinMode(ppDig, OUTPUT);

  CAU::digitalWrite(ppDig, LOW);
  delayMicroseconds(20);  // I think this is needed due to capacitance in ribbon cable
  valLow = CAU::analogRead(ppAn);
  CAU::digitalWrite(ppDig, HIGH);
  delayMicroseconds(20);  // I think this is needed due to capacitance in ribbon cable
  valHigh = CAU::analogRead(ppAn);
  if (valLow >= 70 || valHigh <=4050)
  {
    CAU::log("  Failed Analog Test pp(%d, %d) - %d %d\n", ppAn.port, ppAn.pin, valLow, valHigh);
  }
}

boolean caTestTwoPins(hwPortPin ppIn, hwPortPin ppOut)
{
  uint8 val0, val1;
  boolean ret = true;

  caAllPortsLow(); // Helps detect shorted pins
  CAU::pinMode(ppOut, OUTPUT);
  CAU::digitalWrite(ppOut, LOW);
  CAU::pinMode(ppIn, INPUT);
  val0 = CAU::digitalRead(ppIn);
  CAU::digitalWrite(ppOut, HIGH);
  val1 = CAU::digitalRead(ppIn);
  if ((val0 != LOW) || (val1 != HIGH))
  {
    ret = false;
  }
  
  return ret;
}

// Set all the ports to output low
void caAllPortsLow()
{
  hwPortPin pp0;
  uint8 i, j;

  // Aux Port
  for(i=0; i<46; ++i)
  {
    pp0 = CAU::getAuxPin(i);
    CAU::pinMode(pp0, OUTPUT);
    CAU::digitalWrite(pp0, LOW);
  }
  
  // Module Ports
  for(i=0; i<4; ++i)
  {
    for(j=0; j<6; ++j)
    {
      pp0 = CAU::getModulePin(i, j);
      CAU::pinMode(pp0, OUTPUT);
      CAU::digitalWrite(pp0, LOW);
    }
  }

  // Camera Ports
  for(i=0; i<8; ++i)
  {
    pp0 = CAU::getCameraPin(i, FOCUS);
    CAU::pinMode(pp0, OUTPUT);
    CAU::digitalWrite(pp0, LOW);
    pp0 = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(pp0, OUTPUT);
    CAU::digitalWrite(pp0, LOW);
  }

  // Link Port
  pp0 = CAU::getLinkPin(FOCUS);
  CAU::pinMode(pp0, OUTPUT);
  CAU::digitalWrite(pp0, LOW);
  pp0 = CAU::getLinkPin(SHUTTER);
  CAU::pinMode(pp0, OUTPUT);
  CAU::digitalWrite(pp0, LOW);
}

