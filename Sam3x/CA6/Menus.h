#ifndef MENUS_H
#define MENUS_H

#include <CASensorFilter.h>
#include "Context.h"
#include "PacketProcessor.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test Menu - A menu that tests all the different UI features
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppPin;
} MenuTestData;

MenuTestData gMenuTestData;

const char* menuTest_Name() {
  return "Test Menu";
}

void menuTest_MenuInit() {
  // This menu has no IO to setup
}

void menuTest_PhotoInit() {
  gMenuTestData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuTestData.ppPin, INPUT_PULLUP);
}

void menuTest_MenuRun() {
  uint32_t updateFrequency = 1000;  // 1000 ms
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  static uint32_t count = 0;

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*256)) { // Handles wraparounds
    g_ctx.packetHelper.writePacketString(0, String(count++).c_str());
    nextUpdate = curTime + updateFrequency;
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
  hwPortPin ppPin;
  CASensorFilter sf;
  uint32_t triggerVal;
} MenuSoundData;

MenuSoundData gMenuSoundData;

const char* menuSound_Name() {
  return "Sound Menu";
}

void menuSound_MenuInit() {
  gMenuSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppPin, ANALOG_INPUT);
  gMenuSoundData.sf.init(gMenuSoundData.ppPin, CASensorFilter::ANALOG_THRESHOLD, 2000);  // Update display ever 2000 ms
  gMenuSoundData.sf.setThreshold(2048);
}

void menuSound_PhotoInit() {
  gMenuSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuSoundData.ppPin, ANALOG_INPUT);
}

void menuSound_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  uint16_t val = gMenuSoundData.sf.getSensorData();

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
    nextUpdate = curTime + updateFrequency;
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
    val = (val >= 2048) ? (val-2048) : (2048-val);
    uint8_t trigger = (val >= gMenuSoundData.triggerVal) ? true : false;

    if (trigger) {
      triggerCameras();
    }

    // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Vibration Menu - Detects vibration
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppPin;
  CASensorFilter sf;
  uint32_t triggerVal;
} MenuVibrationData;

MenuVibrationData gMenuVibrationData;

const char* menuVibration_Name() {
  return "Vibration Menu";
}

void menuVibration_MenuInit() {
  gMenuVibrationData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuVibrationData.ppPin, ANALOG_INPUT);
  gMenuVibrationData.sf.init(gMenuVibrationData.ppPin, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
}

void menuVibration_PhotoInit() {
  gMenuVibrationData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gMenuVibrationData.ppPin, ANALOG_INPUT);
}

void menuVibration_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  uint32_t val = gMenuVibrationData.sf.getSensorData();

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    val = map(val, 1, 4095, 1, 1000);  // Convert the values used on the micro to values on webpage
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
    nextUpdate = curTime + updateFrequency;
  }

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  val=0;
  packet = incomingPacketCheckUint32(packet, 0, val);
  incomingPacketFinish(packet);
  if (val != 0) {
     gMenuVibrationData.triggerVal = map(val, 1, 1000, 1, 4095); // Convert the values used on the webpage to values used on mirco
  }
}

void menuVibration_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::analogRead(gMenuVibrationData.ppPin);

    if (val >=  gMenuVibrationData.triggerVal) {
      triggerCameras();
    }

    // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
  }
}

#endif //MENUS_H

