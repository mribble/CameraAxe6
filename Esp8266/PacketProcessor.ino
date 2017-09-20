////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Maurice Ribble
// Copyright 2017
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Takes the incoming string from JS (input string is a packet in string form) and converts it to a binary packet.
//  Then send that binary packet to sam3x.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
      gDynamicMessages.numMessages = 0;
      break;
    case PID_CAM_SETTINGS:
      gPh.writePacketCamSettings(packetStr);
      break;
    case PID_INTERVALOMETER:
      saveStringToFlash("/d/interval", packetStr);
      gPh.writePacketIntervalometer(packetStr);
      break;
    case PID_CAM_TRIGGER:
      gPh.writePacketCamTrigger(packetStr);
      break;
    default:
      CA_ASSERT(0, "Invalid type found");
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This takes a binary packet from sam3x and sends a string message to JS (via ajax hooks)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void receivePacket() {
  CAPacket &mUnpacker = gPh.getUnpacker();
  uint8_t *mData = gPh.getData();

  if (gPh.readOnePacket(mData)) {
    bool packetGuard = mUnpacker.unpackGuard();
    mUnpacker.unpackSize();
    uint8_t packetType = mUnpacker.unpackType();

    CA_ASSERT(packetGuard==true, "Failed guard check");

    // Currently only string data is allowed to be passed from sam3x to esp8266
    if (packetType == PID_STRING) {
      bool found = false;
      CAPacketString unpack(mUnpacker);
      unpack.unpack();
      uint8_t curId = unpack.getClientHostId();

      // We store a limited number of dynamic messages per ajax refresh
      // This code sees if the message exists and if it does it overwrites it
      // otherwise it adds a new message to the list
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
    }
    else {
      CA_ASSERT(0, "Unknown packet");
    }
    mUnpacker.resetBuffer();
  }
}

