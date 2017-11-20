#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CAEsp8266.h>
#include "Context.h"
#include "Menus.h"
#include "NewMenu.h"
#include "PacketProcessor.h"
#include "Terminal.h"
#include "Tests.h"
#include "TriggerCam.h"

// Initialize the context
Context g_ctx;

void setup() {
  // Set up clock ticks
  *REG_SCB_DEMCR |= 0x01000000;
  *REG_DWT_CTRL |= (uint32_t)1;
  *REG_DWT_CYCCNT = (uint32_t)0;

  initCameraPins();
  SerialIO.begin(12345); // Speed doesn't matter here
  CAU::initializeAnalog();
  g_ctx.esp8266.init(74880);
  g_ctx.packetHelper.init(g_ctx.esp8266.getSerial(), (HardwareSerial*)(&SerialIO));
  esp8266ProgramMode();
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

// Put esp8266 into programming mode if dev module on port 4 button pressed during boot
void esp8266ProgramMode() {
  hwPortPin ppPin;
  ppPin = CAU::getModulePin(3, 5);
  CAU::pinMode(ppPin, INPUT_PULLUP);
  if (CAU::digitalRead(ppPin) == LOW) {
    blinkCameraPins();
    g_ctx.esp8266.reprogramESP();
  }
  CAU::pinMode(ppPin, INPUT);
}

