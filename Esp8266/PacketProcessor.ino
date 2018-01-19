////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Handles sending and receiving serial packets to/from sam3x
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
      for(uint8_t i=0; i<gDynamicMessages.numMessages; ++i) {
        gDynamicMessages.str[i].remove(0);
      }
      gDynamicMessages.numMessages = 0;
      gDynamicUint32s.remove(0);
      break;
    case PID_CAM_SETTINGS:
      gPh.writePacketCamSettings(packetStr);
      break;
    case PID_INTERVALOMETER:
      gPh.writePacketIntervalometer(packetStr);
      saveStringToFlash(gIntervalFilename, packetStr);
      break;
    case PID_CAM_TRIGGER:
      gPh.writePacketCamTrigger(packetStr);
      break;
    default:
      CA_LOG(CA_ERROR, "Invalid packet PID: %s\n", packetStr.c_str());
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This takes a binary packet from sam3x and sends a string message to JS (via ajax hooks)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void receivePacket() {
  CAPacket &mUnpacker = gPh.getUnpacker();

  if (gPh.readOnePacket()) {
    bool packetGuard = mUnpacker.unpackGuard();
    uint16_t packetSize = mUnpacker.unpackSize();
    uint8_t packetType = mUnpacker.unpackType();

    CA_ASSERT(packetGuard==true, "Failed guard check");

    // Currently only string data is allowed to be passed from sam3x to esp8266
    if (packetType == PID_STRING) {
      bool found = false;
      CAPacketString unpack(mUnpacker);
      unpack.unpack();
      CA_LOG(CA_ESP_IN_PACKETS, "PID_STRING(esp): %d %s\n", unpack.getClientHostId(), unpack.getString());

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
          CA_LOG(CA_INFO, "Exceeded max dynamic messages: %d\n", MAX_DYNAMIC_MESSAGES);
        }
      }
    }
    else if (packetType == PID_UINT32) {
      CAPacketUint32 unpack(mUnpacker);
      unpack.unpack();
      CA_LOG(CA_ESP_IN_PACKETS, "PID_UINT32(esp): %d %d\n", unpack.getClientHostId(), unpack.getValue());
      
      uint8_t curId = unpack.getClientHostId();
      uint32_t val = unpack.getValue();
      if (gDynamicUint32s.length() < 200) {
        gDynamicUint32s += "id" + String(curId) + "~" + String(val) + "~";
      }
      else {
        CA_LOG(CA_INFO, "Exceeded max uint32 values\n");  // If you hit this you are sending uint32s too frequently or forgetting to fetch the values from JS
      }
    }
    else if (packetType == PID_PERIODIC_DATA) {
      CAPacketPeriodicData unpack(mUnpacker);
      unpack.unpack();
      CA_LOG(CA_ESP_IN_PACKETS, "PID_PERIODIC_DATA(esp): %d %d\n", unpack.getClientHostId(), unpack.getVoltage());
      gVoltage = unpack.getVoltage();
    }
    else {
      CA_LOG(CA_ERROR, "Error, unknown packet (esp): %d %d\n", packetType, packetSize);
    }
    mUnpacker.resetBuffer();
  }
}

