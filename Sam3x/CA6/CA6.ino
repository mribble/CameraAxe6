#include <CAUtility.h>
#include <CAEeprom.h>
#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CATickTimer.h>
#include <CAEsp8266.h>
#include "MenuData.h"
#include "Context.h"

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
  } else if (g_ctx.state == CA_STATE_PHOTO_MODE) {
    g_ctx.procTable.funcPhotoRun[g_ctx.menuId]();
  } else {
    CA_ASSERT(0, "Unsupported CA_STATE mode");
  }
}


