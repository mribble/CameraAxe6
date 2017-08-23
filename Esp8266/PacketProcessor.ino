void sendPacket(String &packetStr) {
  uint8_t index = packetStr.indexOf('~');                   // This gets index to end of type
  uint8_t type = (packetStr.substring(0, index)).toInt();   // Assumes packet type is first element in string
      
  switch (type) {
    case PID_STRING:
      gPh.writePacketString(packetStr);
      break;
    case PID_UINT32:
      gPh.writePacketUint32(packetStr);
      break;
    case PID_TIME_BOX:
      gPh.writePacketTimeBox(packetStr);
      break;
    case PID_MENU_SELECT:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_MENU_LIST:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_MODULE_LIST:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_CAM_STATE:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_CAM_SETTINGS:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_INTERVALOMETER:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_CONTROL_FLAGS:
      CA_ASSERT(0, "Not implemented");
      break;
    case PID_ECHO:
      CA_ASSERT(0, "Not implemented");
      break;
    default:
      CA_ASSERT(0, "Invalid type found");
      break;
  }
}

void recievePacket() {
  // todo
  // read serial for packets
  // put latest packet data into buffer to be processed by html handler
}

