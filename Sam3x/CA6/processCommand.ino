void processCommand() {
  byte command = SerialUSB.read();
  switch (command) {
  case 'p': // Program the ESP8266
    ca6_esp.reprogramESP();
    break;
  case 'r': // Reset the ESP8266
    ca6_esp.resetESP();
    break;
  }
}