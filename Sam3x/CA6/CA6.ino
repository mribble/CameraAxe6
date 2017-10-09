#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CAEsp8266.h>
#include "Context.h"
#include "Menus.h"
#include "PacketProcessor.h"
#include "Terminal.h"
#include "Tests.h"
#include "TriggerCam.h"

// Initialize the context
Context g_ctx;

void setup() {
  SerialIO.begin(9600);
  CAU::initializeAnalog();
  g_ctx.esp8266.init(74880);
  g_ctx.packetHelper.init(g_ctx.esp8266.getSerial(), (HardwareSerial*)(&SerialIO));
  initCameraPins();
}

void loop() {
  while(1) { // Existing loop causes 10 ms delay so we will stay in it forever to avoid that delay
    caRunTests();
    processTerminalCmds();
  
    if (g_ctx.state == CA_STATE_MENU_MODE) {
      if (g_ctx.menuId == 0) {
        // Menus normally process packets, but menuId is a null menu which means we need to handle the processing here
        CAPacketElement *packet = processIncomingPacket();
        if (packet) {
          delete packet;
        }
      } else {
        g_ctx.procTable.funcMenuRun[g_ctx.menuId]();
      }
      if (!camTriggerRunning()) {
        // This needs to get turned off for test trigger and there wasn't a better spot
        endTriggerCameraState();
      }
    } else if (g_ctx.state == CA_STATE_PHOTO_MODE) {
      g_ctx.procTable.funcPhotoRun[g_ctx.menuId]();
    } else {
      CA_ASSERT(0, "Unsupported CA_STATE mode");
    }
  }
}


