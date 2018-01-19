////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Handles incoming serial packets from esp8266
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef PACKET_PROCESSOR_H
#define PACKET_PROCESSOR_H

#include "TriggerCam.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processIncomingPacket() handles packets coming from the client (Android) to the host (sam3x).
// Some packets always do the same thing so we handle those in this function and return null.
// Other packets are menu specific and have to be handled outside this function.  In those cases we return
// the packet from this function.  In these cases the caller must delete tthe packet when it's done.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAPacketElement* processIncomingPacket() {
  CAPacketHelper &ph = g_ctx.packetHelper;
  CAPacket &mUnpacker = ph.getUnpacker();
  CAPacketElement *ret = NULL;

  if (ph.readOnePacket()) {
    bool packetGuard = mUnpacker.unpackGuard();
    uint8_t packetSize = mUnpacker.unpackSize();
    uint8_t packetType = mUnpacker.unpackType();

    CA_ASSERT(packetGuard==true, "Failed guard check");

    switch (packetType) {
      case PID_STRING: {
        CAPacketString unpack(mUnpacker);
        unpack.unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_STRING(sam) - %d %s\n", unpack.getClientHostId(), unpack.getString());
        break;
      }
      case PID_UINT32: {
        CAPacketUint32 *unpack = new CAPacketUint32(mUnpacker);
        unpack->unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_UINT32(sam) - %d %d\n", unpack->getClientHostId(), unpack->getValue());
        ret = unpack;
        break;
      }
      case PID_TIME_BOX: {
        CAPacketTimeBox *unpack = new CAPacketTimeBox(mUnpacker);
        unpack->unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_TIME_BOX(sam) - %d %d %d\n", unpack->getClientHostId(), 
                  unpack->getNanoseconds(), unpack->getSeconds() );
        ret = unpack;
        break;
      }
      case PID_MENU_SELECT: {
        CAPacketMenuSelect unpack(mUnpacker);
        unpack.unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_MENU_SELECT(sam) - %d %s\n", unpack.getMenuMode(), unpack.getMenuName());

        // This is where we do all the code to set pins back to defaults when changing menus or exiting/entering photo mode
        resetCameraPorts();
        resetModulePorts();
        
        uint8_t index = 0;
        if (strcmp(unpack.getMenuName(), "null") == 0) {
          index = 0;
        } else {
          for (uint8_t i=1; i<NUM_MENUS; ++i) {
            if (strcmp(unpack.getMenuName(), g_ctx.procTable.funcName[i]()) == 0) {
              index = i;
            }
          }
          CA_ASSERT(index, "No menu name match");
        }
        g_ctx.menuId = index;

        if (g_ctx.menuId == 0) {
          g_ctx.state = CA_STATE_MENU_MODE;
        } 
        else if (unpack.getMenuMode() == 0) { 
          g_ctx.procTable.funcMenuInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_MENU_MODE;
          
        }
        else {
          g_ctx.procTable.funcPhotoInit[g_ctx.menuId]();
          g_ctx.state = CA_STATE_PHOTO_MODE;
          startTriggerCameraState();
          handleMirrorLockup();
        }
        break;
      }
      case PID_CAM_SETTINGS: {
        CAPacketCamSettings unpack(mUnpacker);
        unpack.unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_CAM_SETTINGS(sam) - %d %d %d %d %d %d %d %d %d %d\n", unpack.getCamPortNumber(),
                  unpack.getMode(), unpack.getDelaySeconds(), unpack.getDelayNanoseconds(), unpack.getDurationSeconds(),
                  unpack.getDurationNanoseconds(), unpack.getPostDelaySeconds(), unpack.getPostDelayNanoseconds(), unpack.getSequencer(),
                  unpack.getMirrorLockup());
        g_ctx.camSettings[unpack.getCamPortNumber()] = unpack;
        if (unpack.getCamPortNumber() == NUM_CAMERAS-1) { // Since all cameras are always sent we only need to setup camera timing on the last one (this is an expensive operation)
          setupCamTiming();
        }
        break;
      }
      case PID_INTERVALOMETER: {
        CAPacketIntervalometer unpack(mUnpacker);
        unpack.unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_INTERVALOMETER(sam) - %d %d %d %d %d %d\n", unpack.getEnable(), unpack.getStartSeconds(),
                unpack.getStartNanoseconds(), unpack.getIntervalSeconds(), unpack.getIntervalNanoseconds(), unpack.getRepeats());
        g_ctx.intervalometerEnable = unpack.getEnable();
        g_ctx.intervalometerStartTime = CATickTimer::convertTimeToTicks(unpack.getStartSeconds(), unpack.getStartNanoseconds());
        g_ctx.intervalometerIntervalTime = CATickTimer::convertTimeToTicks(unpack.getIntervalSeconds(), unpack.getIntervalNanoseconds());
        g_ctx.intervalometerRepeats = unpack.getRepeats();
        break;
      }
      case PID_CAM_TRIGGER: {
        CAPacketCamTrigger unpack(mUnpacker);
        unpack.unpack();
        CA_LOG(CA_SAM_IN_PACKETS, "PID_CAM_TRIGGER(sam) - %d %d %d\n", unpack.getMode(), unpack.getFocus(), unpack.getShutter());
        if (unpack.getMode() == CA_MODE_STANDARD) {
          startTriggerCameraState();
          triggerCameras(); }
        else if (unpack.getMode() == CA_MODE_TOGGLE) {
          toggleCamerasInPhotoMode();
        }
        break;
      }
      default: {
        CA_LOG(CA_ERROR, "Error, unknown packet (sam) %d %d\n", packetType, packetSize);
        break;
      }
    }
    mUnpacker.resetBuffer();
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function for menus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function for menus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAPacketElement* incomingPacketCheckTimeBox(CAPacketElement* base, uint8_t clientHostId, uint32_t &seconds, uint32_t &nanoseconds) {
  if (base != NULL) {
    if (base->getPacketType() == PID_TIME_BOX) {
      if (base->getClientHostId() == clientHostId) {
        CAPacketTimeBox *p = (CAPacketTimeBox*) base;
        seconds = p->getSeconds();
        nanoseconds = p->getNanoseconds();
        delete base;
        return NULL;
      }
    }
  }
  return base;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper function for menus
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void incomingPacketFinish(CAPacketElement* base) {
  if (base != NULL) {
    //CA_LOG("Unprocessed packet at incomingPacketFinish() type: %d, id:%d\n", base->getPacketType(), base->getClientHostId());
    // All packets sent for menu and photo mode so there the above message prints when switching modes
    delete base;
  }
}
#endif //PACKET_PROCESSOR_H


