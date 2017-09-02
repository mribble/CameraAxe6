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
      gPh.writePacketMenuSelect(packetStr);
      break;
    case PID_CAM_SETTINGS:
      gPh.writePacketCamSettings(packetStr);
      break;
    case PID_INTERVALOMETER:
      gPh.writePacketIntervalometer(packetStr);
      break;
    default:
      CA_ASSERT(0, "Invalid type found");
      break;
  }
}

void receivePacket() {
  CAPacket &mUnpacker = gPh.getUnpacker();
  uint8_t *mData = gPh.getData();

  if (gPh.readOnePacket(mData)) {
    bool packetGuard = mUnpacker.unpackGuard();
    uint8_t packetSize = mUnpacker.unpackSize();
    uint8_t packetType = mUnpacker.unpackType();

    CA_ASSERT(packetGuard==true, "Failed guard check");

    switch (packetType) {
      case PID_STRING: {
        bool found = false;
        CAPacketString unpack(mUnpacker);
        unpack.unpack();
        uint8_t curId = unpack.getClientHostId();

        for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
          if (curId == gDynamicMessages.id[i]) {
            gDynamicMessages.str[i] = unpack.getString();
            found = true;
          }
        }
        if (!found) {
          if (gDynamicMessages.numMessages < MAX_DYNAMIC_MESSAGES) {
            uint8_t x = gDynamicMessages.numMessages++;
            gDynamicMessages.id[x] = curId;
            gDynamicMessages.str[x] = unpack.getString();
          }
          else {
            CA_INFO("Exceeded max dynamic messages", MAX_DYNAMIC_MESSAGES);
          }
        }
        break;
      }
      default: {
        CA_ASSERT(0, "Unknown packet");
        break;
      }
    }
    mUnpacker.resetBuffer();
  }
}

