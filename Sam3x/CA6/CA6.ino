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

void triggerCameras() {
  g_ctx.curCamElement = 0;

  // Trigger anything in this sorted list with 0 time
  while(g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0) {
    CamTimerElement x = g_ctx.camTimerElements[g_ctx.curCamElement];
    CAU::digitalWrite(g_ctx.camPins[x.camOffset].focusPin, x.focusSig);
    CAU::digitalWrite(g_ctx.camPins[x.camOffset].shutterPin, x.shutterSig);
    if (++g_ctx.curCamElement == NUM_CAM_TIMER_ELEMENTS) {
      break;
    }
  }
  
  g_ctx.camTimer.start(camTriggerISR, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
}

void camTriggerISR() {
  do {
    CamTimerElement x = g_ctx.camTimerElements[g_ctx.curCamElement];
    CAU::digitalWrite(g_ctx.camPins[x.camOffset].focusPin, x.focusSig);
    CAU::digitalWrite(g_ctx.camPins[x.camOffset].shutterPin, x.shutterSig);

    if (++g_ctx.curCamElement == NUM_CAM_TIMER_ELEMENTS) {
      g_ctx.camTimer.stop();
      return;
    }
  } while(g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset == 0);
  g_ctx.camTimer.start(camTriggerISR, g_ctx.camTimerElements[g_ctx.curCamElement].timeOffset, true);
}

void initCameraPins() {
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    g_ctx.camPins[i].focusPin = CAU::getCameraPin(i, FOCUS);
    g_ctx.camPins[i].shutterPin = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(g_ctx.camPins[i].focusPin, OUTPUT);
    CAU::pinMode(g_ctx.camPins[i].shutterPin, OUTPUT);
  }
}

void setupCamTiming() {
  uint8_t j = 0;
  uint64_t maxTotalTime = 0;
  
  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint64_t t0 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDelaySeconds(), g_ctx.camSettings[i].getDelayNanoseconds());
    uint64_t t1 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getDurationSeconds(), g_ctx.camSettings[i].getDurationNanoseconds());
    uint64_t t2 = CATickTimer::convertTimeToTicks(g_ctx.camSettings[i].getPostDelaySeconds(), g_ctx.camSettings[i].getPostDelayNanoseconds());
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusAfterDelay = (mode == CA_MODE_FLASH) ? LOW : HIGH;
    uint8_t focusAfterDuration = (mode == CA_MODE_PREFOCUS) ? HIGH : LOW;

    maxTotalTime = max(t0+t1+t2, maxTotalTime);

    // Set focus/shutter pins right after delay time
    g_ctx.camTimerElements[j].timeOffset = t0;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = focusAfterDelay;
    g_ctx.camTimerElements[j++].shutterSig = HIGH;

    // Set focus/shutter pins right after duration time
    g_ctx.camTimerElements[j].timeOffset = t0+t1;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = focusAfterDuration;
    g_ctx.camTimerElements[j++].shutterSig = LOW;
  }

  sortCamTiming(g_ctx.camTimerElements, j);
  // Don't need to sort the following because they are all inserted at the end with max time

  for(uint8_t i=0; i<NUM_CAMERAS; ++i) {
    uint8_t mode = g_ctx.camSettings[i].getMode();
    uint8_t focusAtEnd = (mode == CA_MODE_PREFOCUS || mode == CA_MODE_SMART_PREFOCUS) ? HIGH : LOW;
    
    // Set focus/shutter pins at very end of trigger
    g_ctx.camTimerElements[j].timeOffset = maxTotalTime;
    g_ctx.camTimerElements[j].camOffset = i;
    g_ctx.camTimerElements[j].focusSig = focusAtEnd;
    g_ctx.camTimerElements[j++].shutterSig = LOW;
  }

  biasCamTiming(g_ctx.camTimerElements, NUM_CAM_TIMER_ELEMENTS);

  //CA_LOG("Sorted Camera List\n");
  //for(uint8_t i=0; i<NUM_CAM_TIMER_ELEMENTS; ++i) {
  //  CA_LOG("%d  %"PRId64"\n", g_ctx.camTimerElements[i].camOffset, g_ctx.camTimerElements[i].timeOffset);
  //}
}

void sortCamTiming(CamTimerElement* arry, int8_t sz) {
  int8_t i = 1;

  // Insertion Sort
  while (i < sz) {
    CamTimerElement x = arry[i];
    int8_t j = i-1;
    while ((j >= 0) && (arry[j].timeOffset > x.timeOffset)) {
      arry[j+1] = arry[j];
      --j;
    }
    arry[j+1] = x;
    ++i;
  }
}

void biasCamTiming(CamTimerElement* arry, int8_t sz) {
  // Loop through all camera events and find difference in offsets and store those rather than the absolute offset stored before this was called
  // Note this must be called after sorting
  uint64_t x0 = arry[0].timeOffset;
  uint64_t x1;
  for (int8_t i=1; i<sz; ++i) {
    x1 = arry[i].timeOffset - arry[i-1].timeOffset;
    if (x1 == 1) {
      x1 = 0; // Because of the integer divide by clock we get some off by 1 results.  Any interrupt will take way more than 1 clock so skip this
    }
    arry[i-1].timeOffset = x0;
    x0 = x1;
  }
  arry[sz-1].timeOffset = x0;
}

