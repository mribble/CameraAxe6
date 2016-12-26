void caRunTests()
{
  //caTestTickTimer();
  caTestPackets();
  //caTestBlinkLed();
  //caTestPerf();
  //caTestModulePorts();
  //caTestLinkAndCamPorts();
  //caTestAuxPort();
  //caTestEeprom();
  //caTestAnalog();
  //caTestRFD();
  //caTestEsp8266();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestRFD - Test communications with BLE module
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestRFD()
{
  // This test sends a packet from SAM to RFduino.  Then RFduino returns it with the current code there.
  uint8 bufSize;
  uint8 header;
  uint8 data[64];
  uint8 data2[64];
  CAPacket packetProcessor;
  PacketTextDynamic packTextDynamic;
  uint8 packetType;

  packTextDynamic.client_host_id = 1;
  packTextDynamic.text_string = "SAM456";
  packetProcessor.packTextDynamic(&packTextDynamic, data);
  g_ctx.bleSerial.write(data);
  delay(4000);

  if (g_ctx.bleSerial.read(&bufSize, &header, data))
  {
    packetProcessor.unpackTextDynamic((uint8*)data, &packTextDynamic, (char*)data2);
    if (strcmp((char*)packTextDynamic.text_string, "SAM456") != 0)
    {
      CAU::log("  Failed strcmp - %s\n", packTextDynamic.text_string);
    }
    else
    {
      CAU::log("Done - RFD\n");
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestEsp8266 - Test communications with ESP8266 module
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestEsp8266()
{
  char buf[64];
  static bool first = true;

  if (first) {
    first = false;
    Serial1.begin(9600);
  }

  if (Serial1.available()) {
    SerialUSB.write(Serial1.read());
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
  CAPacket packetProcessor;
  uint8 data[112];
  uint8 dataA[128];
  uint8 dataB[128];
  char strBuf[64];
  uint8 packetType, packetSize;
  const uint8 *dataPtr = data;

  memset(dataA, 0, 128);
  memset(dataB, 0, 128); 

  const uint8 sData[] PROGMEM = {
  17,PID_MENU_HEADER,1,2,'U','I',' ','T','e','s','t',' ','M','e','n','u',0,  // MENU_HEADER 1 2 "UI Test Menu"
  2,PID_NEW_ROW,  // NEW_ROW
  3,PID_NEW_CELL_LEFT,40,  // NEW_CELL_LEFT 40
  3,PID_NEW_CELL_RIGHT,10,  // NEW_CELL_RIGHT 10
  3,PID_NEW_CELL_CENTER,50,  // NEW_CELL_CENTER 50
  14,PID_TEXT_STATIC,'S','t','a','t','i','c',' ','T','e','x','t',0,  // TEXT_STATIC "Static Text"
  16,PID_TEXT_DYNAMIC,0,'D','y','n','a','m','i','c',' ','T','e','x','t',0,  // TEXT_DYNAMIC "Dynamic Text"  **gClientHostId_0**
  4,PID_BUTTON,1,17,  // BUTTON 1 1  **gClientHostId_1**
  4,PID_CHECK_BOX,2,0,  // CHECK_BOX 0  **gClientHostId_2**
  11,PID_DROP_SELECT,3,1,'n','o','|','y','e','s',0,  // DROP_SELECT 1 "no|yes"  **gClientHostId_3**
  16,PID_EDIT_NUMBER,4,50,0,0,0,0,159,134,1,0,80,195,0,0,  // EDIT_NUMBER 2 3 0 99999 50000  **gClientHostId_4**
  4,PID_COND_START,5,0,  // COND_START 0 0  **gClientHostId_5**
  11,PID_TIME_BOX,6,223,24,59,122,62,125,144,1,  // TIME_BOX 1 1 1 1 1 0 99 59 40 999 500 400  **gClientHostId_6**
  2,PID_COND_END,  // COND_END
  2,PID_SCRIPT_END,  // SCRIPT_END
  };  // Total Bytes = 112

  // Move progmem data to a buffer for this test
  for(uint16 i=0; i<112; ++i)
  {
   data[i] = pgm_read_byte_near(sData+i);
  }

  // MENU_HEADER 1 2 "UI Test Menu"
  CAPacketMenuHeader p0a, p0b;
  dataPtr = p0a.getPacketSize(dataPtr, &packetSize);
  dataPtr = p0a.getPacketType(dataPtr, &packetType);
  dataPtr = p0a.unpack(dataPtr, strBuf);
  p0b.load(1, 2, "UI Test Menu");
  p0a.pack(dataA);
  p0b.pack(dataB);
  if (memcmp(dataA, dataB, 128) != 0) {
    CAU::log("ERROR - MENU_HEADER test failed");
  }


/*  PacketMenuHeader packMenuHeader;
  PacketNewCell packNewCell;
  PacketTextStatic packStaticText;
  PacketTextDynamic packTextDynamic, packTextDynamic2;
  PacketButton packButton, packButton2;
  PacketCheckBox packCheckBox, packCheckBox2;
  PacketDropSelect packDropSelect, packDropSelect2;
  PacketEditNumber packEditNumber, packEditNumber2;
  PacketCondStart packCondStart;
  PacketTimeBox packTimeBox, packTimeBox2;
  PacketActivate packActivate, packActivate2;
  PacketLog packLog, packLog2;
  PacketCamState packCamState, packCamState2;
  PacketCamSettings packCamSettings, packCamSettings2;
  PacketIntervalometer packIntervalometer, packIntervalometer2;
  PacketInterModuleLogic packInterModuleLogic, packInterModuleLogic2;
  PacketControlFlags packControlFlags, packControlFlags2;


  // MENU_HEADER 1 2 "UI Test Menu"
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackMenuHeader(dataPtr, &packMenuHeader, strBuf);
  if ((packetType != PID_MENU_HEADER) || (packMenuHeader.major_version != 1) || (packMenuHeader.minor_version != 2) || (strcmp(packMenuHeader.menu_string, "UI Test Menu") != 0))
  {
    CAU::log("ERROR - MENU_HEADER - %d, %d, %d, %s\n", packetType, packMenuHeader.major_version, packMenuHeader.minor_version, packMenuHeader.menu_string);
    return;
  }

  // NEW_ROW
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  if (packetType != PID_NEW_ROW)
  {
    CAU::log("ERROR - NEW_ROW - %d\n", packetType);
    return;
  }

  // NEW_CELL_LEFT 40
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackNewCell(dataPtr, &packNewCell);
  if ((packetType != PID_NEW_CELL_LEFT) || (packNewCell.column_percentage != 40))
  {
    CAU::log("ERROR - NEW_CELL_LEFT - %d, %d\n", packetType, packNewCell.column_percentage);
    return;
  }

  // NEW_CELL_RIGHT 10
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackNewCell(dataPtr, &packNewCell);
  if ((packetType != PID_NEW_CELL_RIGHT) || (packNewCell.column_percentage != 10))
  {
    CAU::log("ERROR - NEW_CELL_RIGHT - %d, %d\n", packetType, packNewCell.column_percentage);
    return;
  }

  // NEW_CELL_CENTER 50
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackNewCell(dataPtr, &packNewCell);
  if ((packetType != PID_NEW_CELL_CENTER) || (packNewCell.column_percentage != 50))
  {
    CAU::log("ERROR - NEW_CELL_CENTER - %d, %d\n", packetType, packNewCell.column_percentage);
    return;
  }

  // TEXT_STATIC "Static Text"
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackTextStatic(dataPtr, &packStaticText, strBuf);
  if ((packetType != PID_TEXT_STATIC) || (strcmp(packStaticText.text_string, "Static Text")!=0))
  {
    CAU::log("ERROR - STATIC_TEXT - %d, %s\n", packetType, packStaticText.text_string);
    return;
  }

  // TEXT_DYNAMIC "Dynamic Text"  **gClientHostId_0**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackTextDynamic(dataPtr, &packTextDynamic, strBuf);
  if ((packetType != PID_TEXT_DYNAMIC) || (packTextDynamic.client_host_id != 0) || (strcmp(packTextDynamic.text_string, "Dynamic Text")!=0))
  {
    CAU::log("ERROR - DYNAMIC_TEXT - %d, %d, %s\n", packetType, packTextDynamic.client_host_id, packTextDynamic.text_string);
    return;
  }

  // BUTTON 1 1  **gClientHostId_1**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackButton(dataPtr, &packButton);
  if ((packetType != PID_BUTTON) || (packButton.client_host_id != 1) || (packButton.type != 1) || (packButton.value != 1))
  {
    CAU::log("ERROR - BUTTON - %d, %d, %d, %d\n", packetType, packButton.client_host_id, packButton.type, packButton.value);
    return;
  }

  // CHECK_BOX 0  **gClientHostId_2**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackCheckBox(dataPtr, &packCheckBox);
  if ((packetType != PID_CHECK_BOX) || (packCheckBox.client_host_id != 2) || (packCheckBox.value != 0))
  {
    CAU::log("ERROR - CHECK_BOX - %d, %d, %d\n", packetType, packCheckBox.client_host_id, packCheckBox.value);
    return;
  }

  // DROP_SELECT 1 "no|yes"  **gClientHostId_3**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackDropSelect(dataPtr, &packDropSelect, strBuf);
  if ((packetType != PID_DROP_SELECT) || (packDropSelect.client_host_id != 3) || (packDropSelect.value != 1) || (strcmp(packDropSelect.drop_value_string, "no|yes")!=0))
  {
    CAU::log("ERROR - DROP_SELECT - %d, %d, %d, %s\n", packetType, packDropSelect.client_host_id, packDropSelect.value, packDropSelect.drop_value_string);
    return;
  }

  // EDIT_NUMBER 2 3 0 99999 50000  **gClientHostId_4**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackEditNumber(dataPtr, &packEditNumber);
  if ((packetType != PID_EDIT_NUMBER) || (packEditNumber.client_host_id != 4) || (packEditNumber.digits_before_decimal != 2) || (packEditNumber.digits_after_decimal != 3) || 
      (packEditNumber.min_value != 0) || (packEditNumber.max_value != 99999) || (packEditNumber.value != 50000))
  {
    CAU::log("ERROR - EDIT_NUMBER - %d, %d, %d, %d, %u, %u, %u\n", packetType, packEditNumber.client_host_id, packEditNumber.digits_before_decimal, packEditNumber.digits_after_decimal, packEditNumber.min_value, packEditNumber.max_value, packEditNumber.value);
    return;
  }

  // COND_START 0 0  **gClientHostId_5**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackCondStart(dataPtr, &packCondStart);
  if ((packetType != PID_COND_START) || (packCondStart.client_host_id != 5) || (packCondStart.mod_attribute != 0) || (packCondStart.value != 0))
  {
    CAU::log("ERROR - COND_START - %d, %d, %d, %d\n", packetType, packCondStart.client_host_id, packCondStart.mod_attribute, packCondStart.value);
    return;
  }

  // TIME_BOX 1 1 1 1 1 0 99 59 40 999 500 400  **gClientHostId_6**
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr = packetProcessor.unpackTimeBox(dataPtr, &packTimeBox);
  if ((packetType != PID_TIME_BOX) || (packTimeBox.client_host_id != 6) || (packTimeBox.enable_hours != 1) || (packTimeBox.enable_minutes != 1) || (packTimeBox.enable_seconds != 1) || (packTimeBox.enable_milliseconds != 1) || (packTimeBox.enable_microseconds != 1) || (packTimeBox.enable_nanoseconds != 0) || (packTimeBox.hours != 99) || (packTimeBox.minutes != 59) || (packTimeBox.seconds != 40) || (packTimeBox.milliseconds != 999) || (packTimeBox.microseconds != 500) || (packTimeBox.nanoseconds != 400))
  {

    CAU::log("ERROR - TIME_BOX - %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d \n", packetType, packTimeBox.client_host_id, packTimeBox.enable_hours, packTimeBox.enable_minutes, packTimeBox.enable_seconds, packTimeBox.enable_milliseconds, packTimeBox.enable_microseconds, packTimeBox.enable_nanoseconds, packTimeBox.hours, packTimeBox.minutes, packTimeBox.seconds, packTimeBox.milliseconds, packTimeBox.microseconds, packTimeBox.nanoseconds);
    return;
  }

  // COND_END
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  if (packetType != PID_COND_END)
  {
    CAU::log("ERROR - COND_END - %d\n", packetType);
    return;
  }

  // SCRIPT_END
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  if (packetType != PID_SCRIPT_END)
  {
    CAU::log("ERROR - SCRIPT_END - %d\n", packetType);
    return;
  }

  // Pack/Unpack for TEXT_DYNAMIC
  dataPtr = data;
  memset(&packTextDynamic, 0, sizeof(packTextDynamic));
  memset(&packTextDynamic2, 0, sizeof(packTextDynamic2));
  packTextDynamic.client_host_id = 4;
  packTextDynamic.text_string = "hello123";
  dataPtr2 = packetProcessor.packTextDynamic(&packTextDynamic, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackTextDynamic(dataPtr, &packTextDynamic2, strBuf);
  if ((packetType != PID_TEXT_DYNAMIC) || (memcmp(&packTextDynamic, &packTextDynamic2, sizeof(packTextDynamic)-4) != 0) || strcmp(strBuf, "hello123")!=0)
  {
    CAU::log("ERROR - Pack/unpack TEXT_DYNAMIC %d\n", packetType);
    return;
  }

  // Pack/Unpack for BUTTON
  dataPtr = data;
  memset(&packButton, 0, sizeof(packButton));
  memset(&packButton2, 0, sizeof(packButton2));
  packButton.client_host_id = 3;
  packButton.type = 1;
  packButton.value = 0;
  dataPtr2 = packetProcessor.packButton(&packButton, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackButton(dataPtr, &packButton2);
  if ((packetType != PID_BUTTON) || (memcmp(&packButton, &packButton2, sizeof(packButton)) != 0))
  {
    CAU::log("ERROR - Pack/unpack BUTTON %d\n", packetType);
    return;
  }

  // Pack/Unpack for CHECK_BOX
  dataPtr = data;
  memset(&packCheckBox, 0, sizeof(packCheckBox));
  memset(&packCheckBox2, 0, sizeof(packCheckBox2));
  packCheckBox.client_host_id = 5;
  packCheckBox.value = 99;
  dataPtr2 = packetProcessor.packCheckBox(&packCheckBox, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackCheckBox(dataPtr, &packCheckBox2);
  if ((packetType != PID_CHECK_BOX) || (memcmp(&packCheckBox, &packCheckBox2, sizeof(packCheckBox)) != 0))
  {
    CAU::log("ERROR - Pack/unpack CHECK_BOX %d\n", packetType);
    return;
  }

  // Pack/Unpack for DROP_SELECT
  dataPtr = data;
  memset(&packDropSelect, 0, sizeof(packDropSelect));
  memset(&packDropSelect2, 0, sizeof(packDropSelect2));
  packDropSelect.client_host_id = 6;
  packDropSelect.value = 1;
  packDropSelect.drop_value_string = "yes|no";
  dataPtr2 = packetProcessor.packDropSelect(&packDropSelect, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackDropSelect(dataPtr, &packDropSelect2, strBuf);
  if ((packetType != PID_DROP_SELECT) || (memcmp(&packDropSelect, &packDropSelect2, sizeof(packDropSelect)-4) != 0) || strcmp("yes|no", strBuf)!=0 )
  {
    CAU::log("ERROR - Pack/unpack DROP_SELECT %d\n", packetType);
    return;
  }

  // Pack/Unpack for EDIT_NUMBER
  dataPtr = data;
  memset(&packEditNumber, 0, sizeof(packEditNumber));
  memset(&packEditNumber2, 0, sizeof(packEditNumber2));
  packEditNumber.client_host_id = 3;
  packEditNumber.digits_before_decimal = 2;
  packEditNumber.digits_after_decimal = 1;
  packEditNumber.min_value = 0;
  packEditNumber.max_value = 99;
  packEditNumber.value = 80;
  dataPtr2 = packetProcessor.packEditNumber(&packEditNumber, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackEditNumber(dataPtr, &packEditNumber2);
  if ((packetType != PID_EDIT_NUMBER) || (memcmp(&packEditNumber, &packEditNumber2, sizeof(packEditNumber)) != 0))
  {
    CAU::log("ERROR - Pack/unpack EDIT_NUMBER %d\n", packetType);
    CAU::log(" %d, %d, %d, %u, %u, %u\n", packEditNumber.client_host_id, packEditNumber.digits_before_decimal, packEditNumber.digits_after_decimal, packEditNumber.min_value, packEditNumber.max_value, packEditNumber.value);
    CAU::log(" %d, %d, %d, %u, %u, %u\n", packEditNumber2.client_host_id, packEditNumber2.digits_before_decimal, packEditNumber2.digits_after_decimal, packEditNumber2.min_value, packEditNumber2.max_value, packEditNumber2.value);
    return;
  }

  // Pack/Unpack for TIME_BOX
  dataPtr = data;
  memset(&packTimeBox, 0, sizeof(packTimeBox));
  memset(&packTimeBox2, 0, sizeof(packTimeBox2));
  packTimeBox.client_host_id = 7;
  packTimeBox.enable_hours = 0;
  packTimeBox.enable_minutes = 1;
  packTimeBox.enable_seconds = 1;
  packTimeBox.enable_milliseconds = 1;
  packTimeBox.enable_microseconds = 1;
  packTimeBox.enable_nanoseconds = 1;
  packTimeBox.hours = 10;
  packTimeBox.minutes = 59;
  packTimeBox.seconds = 999;
  packTimeBox.milliseconds = 998;
  packTimeBox.microseconds = 997;
  packTimeBox.nanoseconds = 996;
  dataPtr2 = packetProcessor.packTimeBox(&packTimeBox, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackTimeBox(dataPtr, &packTimeBox2);
  if ((packetType != PID_TIME_BOX) || (memcmp(&packTimeBox, &packTimeBox2, sizeof(packTimeBox)) != 0))
  {
    CAU::log("ERROR - Pack/unpack TIME_BOX %d\n", packetType);
    return;
  }

  // Pack/Unpack for ACTIVATE
  dataPtr = data;
  memset(&packActivate, 0, sizeof(packActivate));
  memset(&packActivate2, 0, sizeof(packActivate2));
  packActivate.active = 1;
  dataPtr2 = packetProcessor.packActivate(&packActivate, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackActivate(dataPtr, &packActivate2);
  if ((packetType != PID_ACTIVATE) || (memcmp(&packActivate, &packActivate2, sizeof(packActivate)) != 0))
  {
    CAU::log("ERROR - Pack/unpack ACTIVATE %d\n", packetType);
    return;
  }

  // Pack/Unpack for LOG
  dataPtr = data;
  memset(&packLog, 0, sizeof(packLog));
  memset(&packLog2, 0, sizeof(packLog2));
  packLog.log_string = "foo";
  dataPtr2 = packetProcessor.packLog(&packLog, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackLog(dataPtr, &packLog2, strBuf);
  if ((packetType != PID_LOG) || (memcmp(&packLog, &packLog2, sizeof(packLog)-4) != 0) || strcmp("foo", strBuf)!=0 )
  {
    CAU::log("ERROR - Pack/unpack LOG %d\n", packetType);
    return;
  }

  // Pack/Unpack for CAM_STATE
  dataPtr = data;
  memset(&packCamState, 0, sizeof(packCamState));
  memset(&packCamState2, 0, sizeof(packCamState2));
  packCamState.cam_multiplier = 0;
  packCamState.cam0_focus     = 1;
  packCamState.cam0_shutter   = 0;
  packCamState.cam1_focus     = 1;
  packCamState.cam1_shutter   = 1;
  packCamState.cam2_focus     = 0;
  packCamState.cam2_shutter   = 0;
  packCamState.cam3_focus     = 0;
  packCamState.cam3_shutter   = 1;
  packCamState.cam4_focus     = 1;
  packCamState.cam4_shutter   = 1;
  packCamState.cam5_focus     = 1;
  packCamState.cam5_shutter   = 1;
  packCamState.cam6_focus     = 1;
  packCamState.cam6_shutter   = 0;
  packCamState.cam7_focus     = 1;
  packCamState.cam7_shutter   = 0;
  dataPtr2 = packetProcessor.packCamState(&packCamState, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackCamState(dataPtr, &packCamState2);
  if ((packetType != PID_CAM_STATE) || (memcmp(&packCamState, &packCamState2, sizeof(packCamState)) != 0))
  {
    CAU::log("ERROR - Pack/unpack CAM_STATE %d\n", packetType);
    return;
  }

  // Pack/Unpack for CAM_SETTINGS
  dataPtr = data;
  memset(&packCamSettings, 0, sizeof(packCamSettings));
  memset(&packCamSettings2, 0, sizeof(packCamSettings2));
  packCamSettings.cam_port_number = 1;
  packCamSettings.mode = 2;
  packCamSettings.delay_hours = 877;
  packCamSettings.delay_minutes = 30;
  packCamSettings.delay_seconds = 55;
  packCamSettings.delay_milliseconds = 998;
  packCamSettings.delay_microseconds = 122;
  packCamSettings.duration_hours = 744;
  packCamSettings.duration_minutes = 22;
  packCamSettings.duration_seconds = 32;
  packCamSettings.duration_milliseconds = 31;
  packCamSettings.duration_microseconds = 333;
  packCamSettings.sequencer0 = 1;
  packCamSettings.sequencer1 = 1;
  packCamSettings.sequencer2 = 0;
  packCamSettings.sequencer3 = 0;
  packCamSettings.sequencer4 = 0;
  packCamSettings.sequencer5 = 0;
  packCamSettings.sequencer6 = 0;
  packCamSettings.sequencer7 = 0;
  packCamSettings.apply_intervalometer = 1;
  packCamSettings.smart_preview = 0;
  packCamSettings.mirror_lockup_enable = 1;
  packCamSettings.mirror_lockup_minutes = 45;
  packCamSettings.mirror_lockup_seconds = 33;
  packCamSettings.mirror_lockup_milliseconds = 525;
  dataPtr2 = packetProcessor.packCamSettings(&packCamSettings, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackCamSettings(dataPtr, &packCamSettings2);
  if ((packetType != PID_CAM_SETTINGS) || (memcmp(&packCamSettings, &packCamSettings2, sizeof(packCamSettings)) != 0))
  {
    CAU::log("ERROR - Pack/unpack CAM_SETTINGS %d\n", packetType);
    return;
  }

  // Pack/Unpack for INTERVALOMETER
  dataPtr = data;
  memset(&packIntervalometer, 0, sizeof(packIntervalometer));
  memset(&packIntervalometer2, 0, sizeof(packIntervalometer2));
  packIntervalometer.start_hours = 22;
  packIntervalometer.start_minutes = 55;
  packIntervalometer.start_seconds = 11;
  packIntervalometer.start_milliseconds = 322;
  packIntervalometer.start_microseconds = 422;
  packIntervalometer.interval_hours = 522;
  packIntervalometer.interval_minutes = 46;
  packIntervalometer.interval_seconds = 12;
  packIntervalometer.interval_milliseconds = 56;
  packIntervalometer.interval_microseconds = 43;
  packIntervalometer.repeats = 43999;
  dataPtr2 = packetProcessor.packIntervalometer(&packIntervalometer, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackIntervalometer(dataPtr, &packIntervalometer2);
  if ((packetType != PID_INTERVALOMETER) || (memcmp(&packIntervalometer, &packIntervalometer2, sizeof(packIntervalometer)) != 0))
  {
    CAU::log("ERROR - Pack/unpack INTERVALOMETER %d\n", packetType);
    return;
  }

  // Pack/Unpack for INTER_MODULE_LOGIC
  dataPtr = data;
  memset(&packInterModuleLogic, 0, sizeof(packInterModuleLogic));
  memset(&packInterModuleLogic2, 0, sizeof(packInterModuleLogic2));
  packInterModuleLogic.enable_latch = 0;
  packInterModuleLogic.logic = 3;
  dataPtr2 = packetProcessor.packInterModuleLogic(&packInterModuleLogic, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackInterModuleLogic(dataPtr, &packInterModuleLogic2);
  if ((packetType != PID_INTER_MODULE_LOGIC) || (memcmp(&packInterModuleLogic, &packInterModuleLogic2, sizeof(packInterModuleLogic)) != 0))
  {
    CAU::log("ERROR - Pack/unpack INTER_MODULE_LOGIC %d\n", packetType);
    return;
  }
  
  // Pack/Unpack for CONTROL_FLAGS
  dataPtr = data;
  memset(&packControlFlags, 0, sizeof(packControlFlags));
  memset(&packControlFlags2, 0, sizeof(packControlFlags2));
  packControlFlags.enable_slave_mode = 0;
  packControlFlags.enable_extra_messages = 1;
  dataPtr2 = packetProcessor.packControlFlags(&packControlFlags, (uint8*)dataPtr);
  dataPtr = packetProcessor.getPacketSize(dataPtr, &packetSize);
  dataPtr = packetProcessor.getPacketType(dataPtr, &packetType);
  dataPtr2 = packetProcessor.unpackControlFlags(dataPtr, &packControlFlags2);
  if ((packetType != PID_CONTROL_FLAGS) || (memcmp(&packControlFlags, &packControlFlags2, sizeof(packControlFlags)) != 0))
  {
    CAU::log("ERROR - Pack/unpack CONTROL_FLAGS %d\n", packetType);
    return;
  }
*/
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

