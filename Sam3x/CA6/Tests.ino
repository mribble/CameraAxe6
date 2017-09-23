void caRunTests()
{
  //caTestSerialWritePerf();  // look into this
  //caTestTickTimer();
  //caTestPerf();
//  caTestPackets();
//  caTestModulePorts();
//  caTestAuxAndCamPorts();
//  caTestEeprom();
//  caTestAnalog();
//  delay(5000); // wait 5000 ms
}

void caTestSerialWritePerf() {
  const uint32_t bufSize = 20;
  uint32_t startTime = 0;
  uint32_t endTime = 0;
  uint8_t writeChars[bufSize];

  Serial1.end();
  Serial1.begin(9600);

  // fill the write buffer
  for (uint8_t i = 0; i < bufSize; i++) {
    writeChars[i] = char((i % 10) + '0');
  }

  startTime = micros();
  Serial1.write(writeChars, bufSize);
  endTime = micros();
  delay(500); // Wait for values to finish being written

  CA_LOG("Serial Write perf (%d bytes): %d us\n", bufSize, endTime-startTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestTickTimer - Tests the CATickTimer class
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestTickTimer()
{
  uint64_t ticks;
  CATickTimer timer(0);
  ticks = timer.convertTimeToTicks(0,100000000);
  timer.start(toggleCamPort0, ticks, true);
  delay(200);
  timer.stop();
}
void toggleCamPort0()
{
  static uint8_t toggle = 1;

  hwPortPin cam0 = CAU::getCameraPin(0, SHUTTER);
  CAU::pinMode(cam0, OUTPUT);

  if (toggle)
  {
    CAU::digitalWrite(cam0, HIGH);
    toggle = 0;
  }
  else
  {
    CAU::digitalWrite(cam0, LOW);
    toggle = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestPackets - Tests packing and unpacking of data packets
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestPackets()
{
  bool unpackGuard;
  uint8_t unpackType;
  uint16_t packSize, unpackSize;
  uint8_t data[512];

  memset(data, 0, 512);
  CAPacket unpackBase(STATE_UNPACKER, data, 512);
  CAPacket packBase(STATE_PACKER, data, 512);

  { // STRING Packet Test
    CAPacketString unpack0(unpackBase);             // Update per type
    CAPacketString pack0(packBase);                 // Update per type
    pack0.set(1, "This is a string");               // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
          packSize != unpackSize ||
          unpackType != PID_STRING ||               // Update per type
          unpack0.getClientHostId() != 1 ||
          strcmp(unpack0.getString(), "This is a string") != 0) {
      CA_LOG("ERROR - STRING test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - STRING test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  {  // UINT32 Packet Test
    CAPacketUint32 unpack0(unpackBase);             // Update per type
    CAPacketUint32 pack0(packBase);                 // Update per type
    pack0.set(2, 123);                              // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
        packSize != unpackSize ||
        unpackType != PID_UINT32 ||                 // Update per type
        unpack0.getClientHostId() != 2 ||
        unpack0.getValue() != 123) {
      CA_LOG("ERROR - UINT32 test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - UINT32 test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  {  // TIME_BOX Packet Test
    CAPacketTimeBox unpack0(unpackBase);            // Update per type
    CAPacketTimeBox pack0(packBase);                // Update per type
    pack0.set(3, 99, 888888);                       // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
        packSize != unpackSize ||
        unpackType != PID_TIME_BOX ||               // Update per type
        unpack0.getClientHostId() != 3 ||
        unpack0.getSeconds() != 99 ||
        unpack0.getNanoseconds() != 888888) 
        {
      CA_LOG("ERROR - TIME_BOX test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - TIME_BOX test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  { // MENU_SELECT Packet Test
    CAPacketMenuSelect unpack0(unpackBase);         // Update per type
    CAPacketMenuSelect pack0(packBase);             // Update per type
    pack0.set(1, "Test");                           // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
          packSize != unpackSize ||
          unpackType != PID_MENU_SELECT ||          // Update per type
          unpack0.getMenuMode() != 1 ||
          strcmp(unpack0.getMenuName(), "Test") != 0) {
      CA_LOG("ERROR - MENU_SELECT test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - MENU_SELECT test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  { // CAM_SETTINGS Packet Test
    CAPacketCamSettings unpack0(unpackBase);        // Update per type
    CAPacketCamSettings pack0(packBase);            // Update per type
    pack0.set(6, 1, 123123, 456456, 111222, 333444, 678678, 1234567, 0xbe, 1);  // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
          packSize != unpackSize ||
          unpackType != PID_CAM_SETTINGS ||         // Update per type
          unpack0.getCamPortNumber() != 6 ||
          unpack0.getMode() != 1 ||
          unpack0.getDelaySeconds() != 123123 ||
          unpack0.getDelayNanoseconds() != 456456 ||
          unpack0.getDurationSeconds() != 111222 ||
          unpack0.getDurationNanoseconds() != 333444 ||
          unpack0.getPostDelaySeconds() != 678678 ||
          unpack0.getPostDelayNanoseconds() != 1234567 ||
          unpack0.getSequencer() != 0xbe ||
          unpack0.getMirrorLockup() != 1) {
      CA_LOG("ERROR - CAM_SETTINGS test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - CAM_SETTINGS test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  { // INTERVALOMETER Packet Test
    CAPacketIntervalometer unpack0(unpackBase);     // Update per type
    CAPacketIntervalometer pack0(packBase);         // Update per type
    pack0.set(1, 1234567, 2345678, 3456789, 4567890, 10); // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
          packSize != unpackSize ||
          unpackType != PID_INTERVALOMETER ||       // Update per type
          unpack0.getEnable() != 1 ||
          unpack0.getStartSeconds() != 1234567 ||
          unpack0.getStartNanoseconds() != 2345678 ||
          unpack0.getIntervalSeconds() != 3456789 ||
          unpack0.getIntervalNanoseconds() != 4567890 ||
          unpack0.getRepeats() != 10 ) {
      CA_LOG("ERROR - INTERVALOMETER test failed\n");
    }
    
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - INTERVALOMETER test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }

  {  // CAM_TRIGGER Packet Test
    CAPacketCamTrigger unpack0(unpackBase);         // Update per type
    CAPacketCamTrigger pack0(packBase);             // Update per type
    pack0.set(0, 0, 0);                             // Update per type
    packSize = pack0.pack();
    unpackGuard = unpackBase.unpackGuard();
    unpackSize = unpackBase.unpackSize();
    unpackType = unpackBase.unpackType();
    unpack0.unpack();
    if (unpackGuard != true ||
        packSize != unpackSize ||
        unpackType != PID_CAM_TRIGGER ||            // Update per type
        unpack0.getMode() != 0 ||
        unpack0.getFocus() != 0 ||
        unpack0.getShutter() != 0) 
        {
      CA_LOG("ERROR - CAM_TRIGGER test failed\n");
    }
    String str0, str1;
    unpack0.packetToString(str0);
    unpack0.set(str0);
    unpack0.packetToString(str1);
    if (str0.compareTo(str1) != 0) {
      CA_LOG("ERROR - CAM_TRIGGER test failed2 %s ** %s\n", str0.c_str(), str1.c_str());
    }
  }
  CA_LOG("Done - testPackets\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestPerf - Various performance tests for low level CA6 functions
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestPerf()
{
  uint16_t count = 0;
  uint8_t val8=0;
  uint16_t val16=0;
  uint32_t startTime, endTime;
  hwPortPin pp0 = CAU::getModulePin(0, 2); // actually Mod0_SDA
  
  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    val8 = CAU::digitalRead(pp0);
  }
  endTime = micros();
  CA_LOG("  10,000 digitalRead() = %dus\n", endTime-startTime);

  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    CAU::digitalWrite(pp0, LOW);
  }
  endTime = micros();
  CA_LOG("  10,000 digitalWrite() = %dus\n", endTime-startTime);

  pp0 = CAU::getOnboardDevicePin(LV_DETECT_PIN);
  CAU::initializeAnalog();
  CAU::pinMode(pp0, ANALOG_INPUT);
  startTime = micros();
  for(count=0; count<10000; ++count)
  {
    val16 = CAU::analogRead(pp0);
  }
  endTime = micros();
  CA_LOG("  10,000 analogRead() = %dus  --junk(%d%d)\n", endTime-startTime, val16, val8);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestModulePorts - Tests all the module ports.  Reports which ones failed. - Requires a special test dongle.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestModulePorts()
{
  uint8_t module;
  uint8_t pin;

  for(module=0; module<4; ++module)
  {
    for(pin=0; pin<6; pin+=2)
    {
      hwPortPin pp0, pp1;
      pp0 = CAU::getModulePin(module, pin);
      pp1 = CAU::getModulePin(module, pin+1);
      if (!caTestTwoPins(pp0, pp1))
      {
        CA_LOG("  Module Port %d:%d failed\n", module, pin);
      }
      
      if (!caTestTwoPins(pp1, pp0))
      {
        CA_LOG("  Module Port %d:%d failed\n", module, pin+1);
      }
    }
  }
  CA_LOG("Done - module ports\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestAuxAndCamPorts - Tests the aux and cameraa ports.  Reports which pins failed. - Requires a special test dongle.
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAuxAndCamPorts()
{
  uint8_t pin;
  hwPortPin pp0, pp1;

  for(pin=0; pin<46; pin+=2)
  {
    pp0 = CAU::getAuxPin(pin);
    pp1 = CAU::getAuxPin(pin+1);
    if (!caTestTwoPins(pp0, pp1))
    {
      CA_LOG("  Aux Port %d failed\n", pin);
    }
    if (!caTestTwoPins(pp1, pp0))
    {
      CA_LOG("  Aux Port %d failed\n", pin+1);
    }
  }

  hwPortPin ppFocus, ppShutter;
  uint8_t cam;
  hwPortPin linkFocus = CAU::getAuxPin(47);
  hwPortPin linkShutter = CAU::getAuxPin(46);
  uint8_t val0, val1, val2, val3, val4, val5;

  for(cam=0; cam<8; ++cam)
  {
    caAllPortsLow(); // Must reset all cam ports here
    CAU::pinMode(linkFocus,   INPUT_PULLUP);
    CAU::pinMode(linkShutter, INPUT_PULLUP);

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
      CA_LOG("  Camera/Aux Port Cam:%d failed (%d,%d,%d,%d,%d,%d)\n", cam, val0, val1, val2, val3, val4, val5);
    }
  }
  
  CA_LOG("Done - aux and camera ports\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestEeprom - Tests the eeprom
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestEeprom()
{
  const char *str0 = "This is just a test string1.";
  const char *str1 = "Blah blah blah blah 2.";
  uint8_t buf[128];
  unioReadStatus statusReg;

  CAEeprom eeprom(CA_INTERNAL);
  
  if (!eeprom.statusWrite(EEPROM_WRITE_PROTECTED_ALL))
  {
    CA_LOG("  Writing status register FAILED\n");
  }
  if(!eeprom.statusRead(&statusReg))
  {
    CA_LOG("  Reading status register FAILED\n");
  }
  if (!eeprom.statusWrite(EEPROM_WRITE_PROTECTED_NONE))
  {
    CA_LOG("  Writing status register 2 FAILED\n");
  }

  if (!eeprom.write((uint8_t*)str0, 0x20, strlen(str0)+1))
  {
    CA_LOG("  Write1 FAILED\n");
  }
  memset(buf, 0, 128);
  if (!eeprom.read(buf, 0x20, strlen(str0)+1))
  {
    CA_LOG("  Read1 FAILED\n");
  }
  if (!eeprom.write((uint8_t*)str1, 0x20, strlen(str1)+1))
  {
    CA_LOG("  Write2 FAILED\n");
  }
  memset(buf, 0, 128);

  if (!eeprom.read(buf, 0x20, strlen(str1)+1))
  {
    CA_LOG("  Read2 FAILED\n");
  }

  CA_LOG("Done - eeprom\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// caTestAnalog - Tests the different analog pins
// returns  - NA
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAnalog()
{
  uint16_t val;
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
  ppAn = CAU::getAuxPin(39);
  ppDig = CAU::getAuxPin(38);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(40);
  ppDig = CAU::getAuxPin(41);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(41);
  ppDig = CAU::getAuxPin(40);
  caTestAnalogPin(ppAn, ppDig);
  ppAn = CAU::getAuxPin(42);
  ppDig = CAU::getAuxPin(43);
  caTestAnalogPin(ppAn, ppDig);

  ppAn = CAU::getOnboardDevicePin(LV_DETECT_PIN);
  CAU::pinMode(ppAn, ANALOG_INPUT);
  val = CAU::analogRead(ppAn); // expected value is 5V/4 = 1.25V -- 1.25*4095/3.3 =1551
  if ((val < 1350) || (val > 1650))
  {
    CA_LOG("  Failed Analog Test -- LV_DETECT_PIN %d\n", val);
  }
  
  CA_LOG("Done - analog\n");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void caTestAnalogPin(hwPortPin ppAn, hwPortPin ppDig)
{
  uint16_t valLow, valHigh;

  caAllPortsLow(); // Helps detect shorted pins
  CAU::pinMode(ppAn, ANALOG_INPUT);
  CAU::pinMode(ppDig, OUTPUT);

  CAU::digitalWrite(ppDig, LOW);
  delayMicroseconds(20);  // I think this is needed due to capacitance in ribbon cable
  valLow = CAU::analogRead(ppAn);
  CAU::digitalWrite(ppDig, HIGH);
  delayMicroseconds(20);  // I think this is needed due to capacitance in ribbon cable
  valHigh = CAU::analogRead(ppAn);
  if (valLow >= 70 || valHigh <=4020)
  {
    CA_LOG("  Failed Analog Test pp(%d, %d) - %d %d\n", ppAn.port, ppAn.pin, valLow, valHigh);
  }
}

boolean caTestTwoPins(hwPortPin ppIn, hwPortPin ppOut)
{
  uint8_t val0, val1;
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
  uint8_t i, j;

  // Aux Port
  for(i=0; i<48; ++i)
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
}

