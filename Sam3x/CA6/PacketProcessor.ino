void processIncomingPackets() {
  uint8* pData = g_ctx.pData;

  if (g_ctx.bleSerial.readOnePacket(pData)) {
    uint8 packetSize = g_ctx.unpacker.unpackSize();
    uint8 packetType = g_ctx.unpacker.unpackType();

    switch (packetType) {
      case PID_MENU_HEADER: {
        CAPacketMenuHeader unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_MENU_HEADER - %d %d %s\n", packetSize, unpack.getMajorVersion(), unpack.getMinorVersion(), unpack.getMenuName());
        break;
      }
      case PID_NEW_ROW: {
        CAPacketNewRow unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_NEW_ROW\n", packetSize);
        break;
      }
      case PID_NEW_CELL: {
        CAPacketNewCell unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_NEW_CELL - %d %d\n", packetSize, unpack.getColumnPercentage(), unpack.getJustification());
        break;
      }
      case PID_COND_START: {
        CAPacketCondStart unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_COND_START - %d %d %d\n", packetSize, unpack.getClientHostId(), unpack.getModAttribute(), unpack.getValue());
        break;
      }
      case PID_COND_END: {
        CAPacketCondEnd unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_COND_END\n", packetSize);
        break;
      }
      case PID_TEXT_STATIC: {
        CAPacketTextStatic unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_TEXT_STATIC - %s\n", packetSize, unpack.getText());
        break;
      }
      case PID_TEXT_DYNAMIC: {
        CAPacketTextDynamic unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_TEXT_DYNAMIC - %d %s\n", packetSize, unpack.getClientHostId(), unpack.getText());
        break;
      }
      case PID_BUTTON: {
        CAPacketButton unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_Button - %d %d %d %s\n", packetSize, unpack.getClientHostId(), unpack.getType(), unpack.getValue(), unpack.getText());
        break;
      }
      case PID_CHECK_BOX: {
        CAPacketCheckBox unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_CHECK_BOX - %d %d\n", packetSize, unpack.getClientHostId(), unpack.getValue());
        break;
      }
      case PID_DROP_SELECT: {
        CAPacketDropSelect unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_DROP_SELECT - %d %d %s\n", packetSize, unpack.getClientHostId(), unpack.getValue(), unpack.getText());
        break;
      }
      case PID_EDIT_NUMBER: {
        CAPacketEditNumber unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_EDIT_NUMBER - %d %d %d %d %d %d\n", packetSize, unpack.getClientHostId(), unpack.getDigitsBeforeDecimal(),
                  unpack.getDigitsAfterDecimal(), unpack.getMinValue(), unpack.getMaxValue(), unpack.getValue());
        break;
      }
      case PID_TIME_BOX: {
        CAPacketTimeBox unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_TIME_BOX - %d %d %d %d %d %d %d %d\n", packetSize, unpack.getClientHostId(), unpack.getEnableMask(),
                  unpack.getHours(), unpack.getMinutes(), unpack.getSeconds(), unpack.getMilliseconds(), unpack.getMicroseconds(),
                  unpack.getNanoseconds());
        break;
      }
      case PID_SCRIPT_END: {
        CAPacketScriptEnd unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_SCRIPT_END\n", packetSize);
        break;
      }
      case PID_MENU_SELECT: {
        CAPacketMenuSelect unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_MENU_SELECT - %d %d\n", packetSize, unpack.getMode(), unpack.getMenuNumber());

        CAPacketLogger loggerPacket(g_ctx.packer);
        loggerPacket.set("Log from sam3x\n");
        uint8 packSize = loggerPacket.pack();
        g_ctx.bleSerial.writeOnePacket(pData);
        g_ctx.packer.resetBuffer();
        break;
      }
      case PID_LOGGER: {
        CAPacketLogger unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_LOGGER - %s\n", packetSize, unpack.getLog());
        g_ctx.unpacker.resetBuffer();
        break;
      }
      case PID_CAM_STATE: {
        CAPacketCamState unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_CAM_STATE - %d %d %d\n", packetSize, unpack.getMultiplier(), unpack.getFocus(), unpack.getShutter());
        break;
      }
      case PID_CAM_SETTINGS: {
        CAPacketCamSettings unpack(g_ctx.unpacker);
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
        CAPacketIntervalometer unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_INTERVALOMETER - %d %d %d %d %d %d %d %d %d %d %d\n", packetSize, unpack.getStartHours(), unpack.getStartMinutes(),
        unpack.getStartSeconds(), unpack.getStartMilliseconds(), unpack.getStartMicroseconds(), unpack.getIntervalHours(),
        unpack.getIntervalMinutes(), unpack.getIntervalSeconds(), unpack.getIntervalMilliseconds(), unpack.getIntervalMicroseconds(),
        unpack.getRepeats());
        break;
      }
      case PID_INTER_MODULE_LOGIC: {
        CAPacketInterModuleLogic unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_INTER_MODULE_LOGIC - %d %d\n", packetSize, unpack.getLatchEnable(), unpack.getLogic());
        break;
      }
      case PID_CONTROL_FLAGS: {
        CAPacketControlFlags unpack(g_ctx.unpacker);
        unpack.unpack();
        CAU::log("%d PID_CONTROL_FLAGS - %d %d\n", packetSize, unpack.getSlaveModeEnable(), unpack.getExtraMessagesEnable());
        break;
      }
      break;
    }
    g_ctx.unpacker.resetBuffer();

  }
}
