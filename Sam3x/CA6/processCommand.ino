void processCommand() {
  byte command = SerialUSB.read();
  switch (command) {
  case 'b': // Put the bluetooth module into Pairing mode
    // raise key pin then send AT command or whatever is needed
    break;
  case 'p': // Program the ESP8266
    ca6_esp.reprogramESP();
    break;
  case 'r': // Reset the ESP8266
    ca6_esp.resetESP();
    break;
  }
}