////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Implements a simple serial terminal.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef TERMINAL_H
#define TERMINAL_H

#define CA_QUANTUM 10  // This is the base time quantium in microseconds

// This uses a simple protocal to transfer 1 byte (8 bits).  The protocal can only send data one way becuase it is
// designed to work over camera trigger ports which are basically just 2 switched on the transmitter.  Then the
// receiver has two lines connected to those switches with pullup resistors so when when the transmitter is
// low the receiver is high and when the transmitter is high the reciever is low.
// 
// This protocal transfer data over a 3.5mm jack.  There are 3 connections (tip transfers data, ring enables 
// data transfers, and base is common/ground).  Here is how data is transfered:
// 1) Ring is set high
// 2) Tip set low for 4 quantums (init period)
// 3) For each bit:
// 4)   if bit is 0: tip set high for 1 quantums, then tip set low for 2 quantums
// 5)   if bit is 1: tip set high for 2 quantums, then tip set low for 1 quantums
// 6) Tip set high for 4 quantums (end period)
// 7) Ring is set low
//
void txCaByte(uint8_t, val) {
  hwPortPin ppTip = CAU::getModulePin(0, FOCUS);   // 3.5mm ring
  hwPortPin ppRing = CAU::getModulePin(0, SHUTTER); // 3.5mm tip
  
  CAU::pinMode(pp, OUTPUT);  // Set pins to high impedance
}

void processTerminalCmds() {
  if (SerialIO.available()) {
    uint8_t command = SerialIO.read();
    switch (command) {
    case '1': // Program ESP8266 through sam3x
    {
      while(!g_ctx.esp8266.getSerial()) {}
      while(!SerialIO) {}
      CA_LOG(CA_INFO, "Sam3x passthrough ESP8266 reprogramming mode, cycle power when finished\n");
      g_ctx.esp8266.end();
      g_ctx.esp8266.init(115200);
      g_ctx.esp8266.reprogramESP();
      g_ctx.packetHelper.flushGarbagePackets();

      HardwareSerial *s = g_ctx.esp8266.getSerial();
      // Setup serial passthrough.  Must use power switch after programming is complete
      while (1) {
        if(SerialIO.available()){
          s->write(SerialIO.read());
        }
        if(s->available()){
          SerialIO.write(s->read());
        }
      }
    }
    case 'p': // Program the ESP8266 using FTDI cable
      g_ctx.esp8266.reprogramESP();
      CA_LOG(CA_INFO, "FTDI ESP8266 reprogramming mode, type y when programming complete\n");
      while (command != 'y') {
        command = SerialIO.read();
      }
      SerialIO.print("ESP8266 Programing complete\n");
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 'r': // Reset the ESP8266
      CA_LOG(CA_INFO, "ESP8266 is being reset\n");
      g_ctx.esp8266.resetESP();
      g_ctx.packetHelper.flushGarbagePackets();
      break;
    case 's': // Serial commands from wireless get passed through to USB Serial (useful for testing) 
    {
      bool done = false;
      HardwareSerial *modSerial = g_ctx.esp8266.getSerial();
      SerialIO.print("Press 'd' to terminate wireless serial passthrough mode\n");
      while (!done) {
        if (modSerial->available()) {
          char c = modSerial->read();
          SerialIO.write(c);
        }
        if (SerialIO.available()) {
          char c = SerialIO.read();
          if (c == 'd') {
            done = true;
          } else {
            modSerial->write(c);
          }
        }
      }
      SerialIO.print("Exited wireless serial passthrough mode\n");
      break;
    }
    default:
      break;
    }
  }
}
#endif //TERMINAL_H

