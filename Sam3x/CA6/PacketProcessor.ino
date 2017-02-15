
 // processIncomingPacket() handles packets coming from the client (Android) to the host (sam3x).
 // Some packets always do the same thing so we handle those in this function and return null.
 // Other packets are menu specific and have to be handled outside this function.  In those cases we return
 // the packet from this function.  In these cases the caller must delete tthe packet when it's done.
 
 CAPacketElement* processIncomingPacket() {
  CAPacketHelper &ph = g_ctx.packetHelper;
  CAPacket &mUnpacker = ph.getUnpacker();
  uint8 *mData = ph.getData();

  if (ph.readOnePacket(mData)) {
    uint8 packetSize = mUnpacker.unpackSize();
    uint8 packetType = mUnpacker.unpackType();
    CAPacketElement *ret = NULL;

    switch (packetType) {
      case PID_MENU_HEADER: {
        CAPacketMenuHeader unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_MENU_HEADER - %d %d %s\n", packetSize, unpack.getMajorVersion(), unpack.getMinorVersion(), unpack.getMenuName());
        CAU::log("Should never be called.\n");
        break;
      }
      case PID_TEXT_STATIC: {
        CAPacketTextStatic unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_TEXT_STATIC - %s\n", packetSize, unpack.getText0());
        break;
      }
      case PID_TEXT_DYNAMIC: {
        CAPacketTextDynamic unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_TEXT_DYNAMIC - %d %d %s %d\n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getText0(), unpack.getText1());
        break;
      }
      case PID_BUTTON: {
        CAPacketButton unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_Button - %d %d %d %d %s %s\n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getType(), unpack.getValue(), unpack.getText0(), unpack.getText1());
        break;
      }
      case PID_CHECK_BOX: {
        CAPacketCheckBox unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_CHECK_BOX - %d %d %d %s\n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getValue(), unpack.getText0());
        break;
      }
      case PID_DROP_SELECT: {
        CAPacketDropSelect unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_DROP_SELECT - %d %d %d %s %s \n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getValue(), unpack.getText0(), unpack.getText1());
        break;
      }
      case PID_EDIT_NUMBER: {
        CAPacketEditNumber *unpack = new CAPacketEditNumber(mUnpacker);
        unpack->unpack();
        CAU::log("%d PID_EDIT_NUMBER - %d %d %d %d %d %d %d %s \n", packetSize, unpack->getClientHostId(), unpack->getModAttribute(), unpack->getDigitsBeforeDecimal(),
                  unpack->getDigitsAfterDecimal(), unpack->getMinValue(), unpack->getMaxValue(), unpack->getValue(), unpack->getText0());
        ret = unpack;
        break;
      }
      case PID_TIME_BOX: {
        CAPacketTimeBox unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_TIME_BOX - %d %d %d %d %d %d %d %d %d %s\n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getEnableMask(),
                  unpack.getHours(), unpack.getMinutes(), unpack.getSeconds(), unpack.getMilliseconds(), unpack.getMicroseconds(),
                  unpack.getNanoseconds(), unpack.getText0());
        break;
      }
      case PID_SCRIPT_END: {
        CAPacketScriptEnd unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_SCRIPT_END\n", packetSize);
        CAU::log("Should never be called.\n");
        break;
      }
      case PID_MENU_SELECT: {
        CAPacketMenuSelect unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_MENU_SELECT - %d %d\n", packetSize, unpack.getMode(), unpack.getMenuNumber());
        g_ctx.menuId = unpack.getMenuNumber();
        g_ctx.state = CA_STATE_LOADING_MENU;
        if (unpack.getMode() == 0) {
          g_ctx.procTable.funcMenuInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_MENU_MODE;
        } else {
          g_ctx.procTable.funcPhotoInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_PHOTO_MODE;
        }
        break;
      }
      case PID_MENU_LIST: {
        CAPacketMenuList unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_MENU_LIST - %d %d %x %d %x %d %x %d %x %d %x %d %x %s\n", packetSize, unpack.getMenuId(), unpack.getModuleId0(), unpack.getModuleMask0(),
                  unpack.getModuleId1(), unpack.getModuleMask1(), unpack.getModuleId2(), unpack.getModuleMask2(), unpack.getModuleId3(), unpack.getModuleMask3(),
                  unpack.getModuleTypeId0(), unpack.getModuleTypeMask0(), unpack.getModuleTypeId1(), unpack.getModuleTypeMask1(), unpack.getMenuName());
        for(int i=1; i<NUM_MENUS; ++i) {
          g_ctx.procTable.funcInfo[i]();
        }
        break;
      }
      case PID_MODULE_LIST: {
        CAPacketModuleList unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_MODULE_LIST - %d %d %s\n", packetSize, unpack.getModuleId(), unpack.getModuleTypeId(), unpack.getModuleName());
        break;
      }
      case PID_LOGGER: {
        CAPacketLogger unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_LOGGER - %s\n", packetSize, unpack.getLog());
        break;
      }
      case PID_CAM_STATE: {
        CAPacketCamState unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_CAM_STATE - %d %d %d\n", packetSize, unpack.getMultiplier(), unpack.getFocus(), unpack.getShutter());
        break;
      }
      case PID_CAM_SETTINGS: {
        CAPacketCamSettings unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_CAM_SETTINGS - %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", packetSize, unpack.getCamPortNumber(),
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
        CAU::log("%d PID_INTERVALOMETER - %d %d %d %d %d %d %d %d %d %d %d\n", packetSize, unpack.getStartHours(), unpack.getStartMinutes(),
        unpack.getStartSeconds(), unpack.getStartMilliseconds(), unpack.getStartMicroseconds(), unpack.getIntervalHours(),
        unpack.getIntervalMinutes(), unpack.getIntervalSeconds(), unpack.getIntervalMilliseconds(), unpack.getIntervalMicroseconds(),
        unpack.getRepeats());
        break;
      }
      case PID_CONTROL_FLAGS: {
        CAPacketControlFlags unpack(mUnpacker);
        unpack.unpack();
        CAU::log("%d PID_CONTROL_FLAGS - %d %d\n", packetSize, unpack.getSlaveModeEnable(), unpack.getExtraMessagesEnable());
        break;
      }
      break;
    }
    mUnpacker.resetBuffer();
    return ret;
  }
}

CAPacketElement* incomingPacketCheckEditNumber(CAPacketElement* p, uint8 clientHostId, uint32 &val) {
  if (p != NULL) {
    if (p->getPacketType() == PID_EDIT_NUMBER) {
      if (p->getClientHostId() == clientHostId) {
        CAPacketEditNumber *p1 = (CAPacketEditNumber*) p;
        val = p1->getValue();
        delete p;
        return NULL;
      }
    }
  }
  return p;
}

void incomingPacketFinish(CAPacketElement* p) {
  if (p != NULL) {
    delete p;
    CAU::log("Invalid packet found during incomingPacketCheckFinish()\n");
  }
}



