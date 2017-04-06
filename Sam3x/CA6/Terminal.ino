void processTerminalCmds() {
  if (SerialUSB.available()) {
    uint8_t command = SerialUSB.read();
    switch (command) {
    case 'p': // Program the ESP8266
      g_ctx.esp8266.reprogramESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 'r': // Reset the ESP8266
      g_ctx.esp8266.resetESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 't': // Program a test module in module port 3
    {
      CAEeprom eeprom(CA_MODULE3);
      const uint8_t testModuleId = 2;
      uint8_t val = 0;

      if (eeprom.writeModuleId(testModuleId)) {
        SerialUSB.print("Success - Writing test module\n");
        if (eeprom.readModuleId(&val)) {
          if (val == testModuleId) {
            SerialUSB.print("Success - Reading test module\n");
          } else {
            SerialUSB.print("Fail - Reading test module\n");
          }
        } else {
          SerialUSB.print("Fail - Reading test module 2\n");
        }
      } else {
        SerialUSB.print("Fail - Writing test module\n");
      }
      break;
    }
    case 's': // Serial commands from wireless get passed through to USB Serial (useful for testing) 
    {
      bool done = false;
      HardwareSerial *modSerial = g_ctx.esp8266.getSerial();
      SerialUSB.print("Press 'd' to terminate wireless serial passthrough mode\n");
      while (!done) {
        if (modSerial->available()) {
          char c = modSerial->read();
          SerialUSB.write(c);
        }
        if (SerialUSB.available()) {
          char c = SerialUSB.read();
          if (c == 'd') {
            done = true;
          } else {
            modSerial->write(c);
          }
        }
      }
      SerialUSB.print("Exited wireless serial passthrough mode\n");
      break;
    }
    default:
      break;
    }
  }
}

