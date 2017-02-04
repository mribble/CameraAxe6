void processTerminalCmds() {
  if (SerialUSB.available()) {
    uint8 command = SerialUSB.read();
    switch (command) {
    case 'b': // Put the bluetooth module into Pairing mode
      // raise key pin then send AT command or whatever is needed
      break;
    case 'p': // Program the ESP8266
      g_ctx.esp8266.reprogramESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 'r': // Reset the ESP8266
      g_ctx.esp8266.resetESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 't': // Program a test module in module port 3
      CAEeprom eeprom(CA_MODULE3);
      uint8 testModuleId = 2;
      if (eeprom.write(&testModuleId, 0x02, 1)) {
        CAU::log("Success - Writting test module\n");
        testModuleId = 0;
        if (eeprom.read(&testModuleId, 0x02, 1)) {
          if (testModuleId == 2) {
            CAU::log("Success - Reading test module\n");
          } else {
            CAU::log("Fail - Reading test module\n");
          }
        } else {
          CAU::log("Fail - Reading test module 2\n");
        }
      } else {
        CAU::log("Fail - Writting test module\n");
      }
      break;
    }
  }
}

