#ifndef TERMINAL_H
#define TERMINAL_H

void processTerminalCmds() {
  if (SerialUSB.available()) {
    uint8_t command = SerialUSB.read();
    switch (command) {
    case '1': // Program ESP8266 through sam3x
    {
      while(!g_ctx.esp8266.getSerial()) {}
      while(!SerialUSB) {}
      CA_LOG("Sam3x passthrough ESP8266 reprogramming mode, cycle power when finished\n");
      g_ctx.esp8266.end();
      g_ctx.esp8266.init(115200);
      g_ctx.esp8266.reprogramESP();
      g_ctx.packetHelper.flushGarbagePackets();

      HardwareSerial *s = g_ctx.esp8266.getSerial();
      // Setup serial passthrough.  Must use power switch after programming is complete
      while (1) {
        if(SerialUSB.available()){
          s->write(SerialUSB.read());
        }
        if(s->available()){
          SerialUSB.write(s->read());
        }
      }
    }
    case 'p': // Program the ESP8266 using FTDI cable
      g_ctx.esp8266.reprogramESP();
      CA_LOG("FTDI ESP8266 reprogramming mode, type y when programming complete\n");
      while (command != 'y') {
        command = SerialIO.read();
      }
      SerialIO.print("ESP8266 Programing complete\n");
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 'r': // Reset the ESP8266
      CA_LOG("ESP8266 is being reset\n");
      g_ctx.esp8266.resetESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
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
#endif //TERMINAL_H

