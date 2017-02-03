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
    }
  }
}

