#ifndef MENUS_H
#define MENUS_H

#include "Context.h"
#include "PacketProcessor.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test Menu - A menu that tests all the different UI features
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  uint32_t nextUpdate;
  hwPortPin ppPin;
  uint32_t count;
} MenuTestData;

MenuTestData gMenuTestData;

const char* menuTest_Name() {
  return "Test Menu";
}

void menuTest_MenuInit() {
  gMenuTestData.nextUpdate = millis();
  gMenuTestData.count = 0;
  // This menu has no IO to setup
}

void menuTest_PhotoInit() {
  gMenuTestData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuTestData.ppPin, INPUT_PULLUP);
}

void menuTest_MenuRun() {
  uint32_t updateFrequency = 1000;  // 1000 ms
  uint32_t curTime = millis();
  uint32_t nextUpdate = gMenuTestData.nextUpdate;

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*256)) { // Handles wraparounds
    ++gMenuTestData.count;
    g_ctx.packetHelper.writePacketString(0, String(gMenuTestData.count).c_str());
    gMenuTestData.nextUpdate = curTime + updateFrequency;
  }

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  uint32_t val, sec, nano;
  packet = incomingPacketCheckUint32(packet, 1, val);
  packet = incomingPacketCheckUint32(packet, 2, val);
  packet = incomingPacketCheckUint32(packet, 3, val);
  packet = incomingPacketCheckUint32(packet, 4, val);
  packet = incomingPacketCheckTimeBox(packet, 5, sec, nano);
  packet = incomingPacketCheckUint32(packet, 6, val);
  incomingPacketFinish(packet);
}

void menuTest_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint8_t val = CAU::digitalRead(gMenuTestData.ppPin);
    if (val == LOW) {
      triggerCameras();
    }

    // Handle incoming packets
    if (SerialIO.available()) {
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound Menu - Detects sound
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  uint32_t nextUpdate;
  hwPortPin ppPin;
  uint32_t triggerVal;
} MenuSoundData;

MenuSoundData gMenuSoundData;

const char* menuSound_Name() {
  return "Sound Menu";
}

void menuSound_MenuInit() {
  gMenuSoundData.nextUpdate = millis();
  gMenuSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppPin, ANALOG_INPUT);
}

void menuSound_PhotoInit() {
  gMenuSoundData.nextUpdate = millis();
  gMenuSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppPin, ANALOG_INPUT);
}

void menuSound_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  uint32_t nextUpdate = gMenuSoundData.nextUpdate;

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    uint16_t val = CAU::analogRead(gMenuSoundData.ppPin);
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
    gMenuSoundData.nextUpdate = curTime + updateFrequency;
  }

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gMenuSoundData.triggerVal);
  incomingPacketFinish(packet);
}

void menuSound_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::analogRead(gMenuSoundData.ppPin);
    uint8_t trigger = (val >= gMenuSoundData.triggerVal) ? true : false;
    if (trigger) {
      triggerCameras();
    }

    // Handle incoming packets
    if (SerialIO.available()) {
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

enum FilterMode {
  DIGITAL,
  ANALOG,
};

struct FilterData {
  hwPortPin pin;            // Pin that is being filtered
  FilterMode mode;          // Know if the reading of the pin should be analog or digital
  uint8_t curValIndex;      // Current index for the rolling window
  uint16_t vals[4];         // Rolling window of values
  uint16_t visibleTime;     // Min time in milliseconds the value will be visible
  uint32_t nextUpdateTime;  // The time in milliseconds when we update the curValIndex
};

uint16_t sensorFilter(FilterData *d) {
  uint16_t val;
  if (d->mode == DIGITAL) {
    val = CAU::digitalRead(d->pin);
  }
  else {
    val = CAU::analogRead(d->pin);
  }
  //todo
}
#endif //MENUS_H


