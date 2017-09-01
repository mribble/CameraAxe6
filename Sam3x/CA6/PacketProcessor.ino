// processIncomingPacket() handles packets coming from the client (Android) to the host (sam3x).
// Some packets always do the same thing so we handle those in this function and return null.
// Other packets are menu specific and have to be handled outside this function.  In those cases we return
// the packet from this function.  In these cases the caller must delete tthe packet when it's done.
 
CAPacketElement* processIncomingPacket() {
  CAPacketHelper &ph = g_ctx.packetHelper;
  CAPacket &mUnpacker = ph.getUnpacker();
  uint8_t *mData = ph.getData();
  CAPacketElement *ret = NULL;

  if (ph.readOnePacket(mData)) {
    bool packetGuard = mUnpacker.unpackGuard();
    uint8_t packetSize = mUnpacker.unpackSize();
    uint8_t packetType = mUnpacker.unpackType();

    CA_ASSERT(packetGuard==true, "Failed guard check");

    switch (packetType) {
      case PID_STRING: {
        CAPacketString unpack(mUnpacker);
        unpack.unpack();
        CA_LOG("%d PID_STRING - %d %s\n", packetSize, unpack.getClientHostId(), unpack.getString());
        break;
      }
      case PID_UINT32: {
        CAPacketUint32 *unpack = new CAPacketUint32(mUnpacker);
        unpack->unpack();
        CA_LOG("%d PID_UINT32 - %d %d\n", packetSize, unpack->getClientHostId(), unpack->getValue());
        ret = unpack;
        break;
      }
      case PID_TIME_BOX: {
        CAPacketTimeBox unpack(mUnpacker);
        unpack.unpack();
        CA_LOG("%d PID_TIME_BOX - %d %d %d\n", packetSize, unpack.getClientHostId(), 
                  unpack.getNanoseconds(), unpack.getSeconds() );
        break;
      }
      case PID_MENU_SELECT: {
        CAPacketMenuSelect unpack(mUnpacker);
        unpack.unpack();
        CA_LOG("%d PID_MENU_SELECT - %d %s\n", packetSize, unpack.getMenuMode(), unpack.getMenuName());

        uint8_t index = 0;
        if (strcmp(unpack.getMenuName(), "null") == 0) {
          index = 0;
        } else {
          for (uint8_t i=1; i<NUM_MENUS; ++i) {
            if (strcmp(unpack.getMenuName(), g_ctx.procTable.funcName[i]()) == 0) {
              index = i;
            }
          }
          if (index == 0) {
            CA_ERROR("No menu name match", 0);
          }
        }
        g_ctx.menuId = index;

        if (g_ctx.menuId == 0) {
          g_ctx.state = CA_STATE_MENU_MODE;
        } else if (unpack.getMenuMode() == 0) {
          g_ctx.procTable.funcMenuInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_MENU_MODE;
        } else {
          g_ctx.procTable.funcPhotoInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_PHOTO_MODE;
        }
        break;
      }
      case PID_CAM_SETTINGS: {
        CAPacketCamSettings unpack(mUnpacker);
        unpack.unpack();
        CA_LOG("%d PID_CAM_SETTINGS - %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", packetSize, unpack.getCamPortNumber(),
                  unpack.getMode(), unpack.getDelayHours(), unpack.getDelayMinutes(), unpack.getDelaySeconds(),
                  unpack.getDelayMilliseconds(), unpack.getDelayMicroseconds(), unpack.getDurationHours(), unpack.getDurationMinutes(),
                  unpack.getDurationSeconds(), unpack.getDurationMilliseconds(), unpack.getDurationMicroseconds(), unpack.getSequencer(),
                  unpack.getApplyIntervalometer(), unpack.getSmartPreview(), unpack.getMirrorLockupEnable(),
                  unpack.getMirrorLockupMinutes(), unpack.getMirrorLockupSeconds(), unpack.getMirrorLockupMilliseconds());
        break;
      }
      case PID_INTERVALOMETER: {
        CAPacketIntervalometer unpack(mUnpacker);
        unpack.unpack();
        CA_LOG("%d PID_INTERVALOMETER - %d %d %d %d %d %d\n", packetSize, unpack.getEnable(), unpack.getStartSeconds(),
                unpack.getStartNanoseconds(), unpack.getIntervalSeconds(), unpack.getIntervalNanoseconds(), unpack.getRepeats());
        break;
      }
      default: {
        CA_ERROR("Unknown packet", 0);
        break;
      }
    }
    mUnpacker.resetBuffer();
  }
  return ret;
}

CAPacketElement* incomingPacketCheckUint32(CAPacketElement* base, uint8_t clientHostId, uint32_t &val) {
  if (base != NULL) {
    if (base->getPacketType() == PID_UINT32) {
      if (base->getClientHostId() == clientHostId) {
        CAPacketUint32 *p = (CAPacketUint32*) base;
        val = p->getValue();
        delete base;
        return NULL;
      }
    }
  }
  return base;
}

void incomingPacketFinish(CAPacketElement* base) {
  if (base != NULL) {
    delete base;
    CA_ERROR("Unprocessed packet found during incomingPacketCheckFinish()", 0);
  }
}



