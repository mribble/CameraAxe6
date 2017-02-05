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
      const uint8 testModuleId = 2;
      uint8 val = 0;

      if (eeprom.writeModuleId(testModuleId)) {
        CAU::log("Success - Writing test module\n");
        if (eeprom.readModuleId(&val)) {
          if (val == testModuleId) {
            CAU::log("Success - Reading test module\n");
          } else {
            CAU::log("Fail - Reading test module\n");
          }
        } else {
          CAU::log("Fail - Reading test module 2\n");
        }
      } else {
        CAU::log("Fail - Writing test module\n");
      }
      break;
    }
  }
}

