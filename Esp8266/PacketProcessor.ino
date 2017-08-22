void sendPacket(String &packetStr) {
  uint8_t index = packetStr.indexOf('~');                   // This gets index to end of type
  uint8_t type = (packetStr.substring(0, index)).toInt();   // Assumes packet type is first element in string
      
  switch (type) {
    case PID_STRING:
      break;
    case PID_UINT32:
      gPh.writePacketUint32(packetStr.c_str());
      break;
    case PID_TIME_BOX:
      break;
    case PID_MENU_SELECT:
      break;
    case PID_MENU_LIST:
      break;
    case PID_MODULE_LIST:
      break;
    case PID_CAM_STATE:
      break;
    case PID_CAM_SETTINGS:
      break;
    case PID_INTERVALOMETER:
      break;
    case PID_CONTROL_FLAGS:
      break;
    case PID_ECHO:
      break;
    default:
      CA_ASSERT(0, "Invalid type found");
      break;
  }

  
}

