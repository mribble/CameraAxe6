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

void triggerCameras() {
  hwPortPin ppFocus, ppShutter;
  uint8_t i;

  // todo -- For now just trigger all the cameras for 1 second

  for(i=0; i<8; ++i)
  {
    ppFocus = CAU::getCameraPin(i, FOCUS);
    CAU::pinMode(ppFocus, OUTPUT);
    CAU::digitalWrite(ppFocus, HIGH);
    ppShutter = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(ppShutter, OUTPUT);
    CAU::digitalWrite(ppShutter, HIGH);
  }
  delay(1000);

  for(i=0; i<8; ++i)
  {
    ppFocus = CAU::getCameraPin(i, FOCUS);
    CAU::digitalWrite(ppFocus, LOW);
    ppShutter = CAU::getCameraPin(i, SHUTTER);
    CAU::digitalWrite(ppShutter, LOW);
  }
}


void initCameraPins() {
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    g_ctx.camPins[i].focusPin = CAU::getCameraPin(i, FOCUS);
    g_ctx.camPins[i].shutterPin = CAU::getCameraPin(i, SHUTTER);
  }
}

uint64_t convertTimeToTicks(uint32_t seconds, uint32_t nanoseconds) {
  const uint8_t ticksPerMicroSec = 84; //Sam8 running at 84 mhz
  uint64_t ret = ((uint64_t)seconds*1000*1000*1000 + nanoseconds)/ticksPerMicroSec;
  return ret;
}

void setupCamTiming() {
  uint8_t j = 0;
  uint64_t maxTotalTime = 0;
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint64_t t0 = convertTimeToTicks(g_ctx.camSettings[i].getDelaySeconds(), g_ctx.camSettings[i].getDelayNanoseconds());
    uint64_t t1 = convertTimeToTicks(g_ctx.camSettings[i].getDurationSeconds(), g_ctx.camSettings[i].getDurationNanoseconds());
    uint64_t t2 = convertTimeToTicks(g_ctx.camSettings[i].getPostDelaySeconds(), g_ctx.camSettings[i].getPostDelayNanoseconds());
    
    uint8_t mode = g_ctx.camSettings[i].getMode();

    uint8_t focusAfterDelay = (mode == CA_MODE_FLASH) ? LOW : HIGH;
    //uint8_t focusAfterDuration = (;
    //uint8_t focusAtEnd = ;

    maxTotalTime = max(t0+t1+t2, maxTotalTime);

    // Set focus/shutter pins right after delay time
    g_ctx.camTimerElements[j].timeOffset = t0;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = HIGH;
    g_ctx.camTimerElements[j++].shutterSig = HIGH;
    // Set focus/shutter pins right after duration time
    g_ctx.camTimerElements[j].timeOffset = t0;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = HIGH;
    g_ctx.camTimerElements[j++].shutterSig = HIGH;
  }

  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    // Set focus/shutter pins at very end of trigger  
  }
}

void sortCamTiming() {
  //todo
}

