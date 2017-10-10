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
} TestData;

TestData gTestData;

const char* test_Name() {
  return "Test Menu";
}

void test_MenuInit() {
  // This menu has no IO to setup
}

void test_PhotoInit() {
  gTestData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gTestData.ppPin, INPUT_PULLUP);
}

void test_MenuRun() {
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

void test_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint8_t val = CAU::digitalRead(gTestData.ppPin);
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
} SoundData;

SoundData gSoundData;

const char* sound_Name() {
  return "Sound Menu";
}

void sound_MenuInit() {
  gSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gSoundData.ppPin, ANALOG_INPUT);
  gSoundData.sf.init(gSoundData.ppPin, CASensorFilter::ANALOG_THRESHOLD, 2000);  // Update display ever 2000 ms
  gSoundData.sf.setThreshold(2048);
}

void sound_PhotoInit() {
  gSoundData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gSoundData.ppPin, ANALOG_INPUT);
}

void sound_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  uint16_t val = gSoundData.sf.getSensorData();

  // Handle outgoing packets
  if ((curTime >= nextUpdate) && (curTime-nextUpdate < updateFrequency*1000)) { // Handles wraparounds
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
    nextUpdate = curTime + updateFrequency;
  }

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gSoundData.triggerVal);
  incomingPacketFinish(packet);
}

void sound_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::analogRead(gSoundData.ppPin);
    val = (val >= 2048) ? (val-2048) : (2048-val);
    uint8_t trigger = (val >= gSoundData.triggerVal) ? true : false;

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
} VibrationData;

VibrationData gVibrationData;

const char* vibration_Name() {
  return "Vibration Menu";
}

void vibration_MenuInit() {
  gVibrationData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gVibrationData.ppPin, ANALOG_INPUT);
  gVibrationData.sf.init(gVibrationData.ppPin, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
}

void vibration_PhotoInit() {
  gVibrationData.ppPin = CAU::getModulePin(0, 0);
  CAU::pinMode(gVibrationData.ppPin, ANALOG_INPUT);
}

void vibration_MenuRun() {
  uint32_t updateFrequency = 500;  // 500 ms
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  uint32_t val = gVibrationData.sf.getSensorData();

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
     gVibrationData.triggerVal = map(val, 1, 1000, 1, 4095); // Convert the values used on the webpage to values used on mirco
  }
}

void vibration_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::analogRead(gVibrationData.ppPin);

    if (val >=  gVibrationData.triggerVal) {
      triggerCameras();
    }

    // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Valve Menu - Triggers Valve
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppValve[4];
  uint32_t shutterLag;
  uint32_t flashDelay;
  uint32_t dropDelay[4][3];
  uint32_t dropSize[4][3];
} ValveData;

ValveData gValveData;

const char* valve_Name() {
  return "Valve Menu";
}

void valve_MenuInit() {
}

void valve_PhotoInit() {
  gValveData.ppValve[0] = CAU::getModulePin(0, 0);
  gValveData.ppValve[1] = CAU::getModulePin(0, 1);
  gValveData.ppValve[2] = CAU::getModulePin(0, 2);
  gValveData.ppValve[3] = CAU::getModulePin(0, 3);

  CAU::pinMode(gValveData.ppValve[0], OUTPUT);
  CAU::digitalWrite(gValveData.ppValve[0], LOW);
  CAU::pinMode(gValveData.ppValve[1], OUTPUT);
  CAU::digitalWrite(gValveData.ppValve[1], LOW);
  CAU::pinMode(gValveData.ppValve[2], OUTPUT);
  CAU::digitalWrite(gValveData.ppValve[2], LOW);
  CAU::pinMode(gValveData.ppValve[3], OUTPUT);
  CAU::digitalWrite(gValveData.ppValve[3], LOW);
}

void valve_MenuRun() {
  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gValveData.shutterLag);
  packet = incomingPacketCheckUint32(packet, 1, gValveData.flashDelay);
  for (uint8_t i=0; i<4; ++i) { //Valves (max is 4)
    for (uint8_t j=0; j<3; ++j) { //Drops (max is 3)
      packet = incomingPacketCheckUint32(packet, 2+i*2+0, gValveData.dropDelay[i][j]);
      packet = incomingPacketCheckUint32(packet, 2+i*2+1, gValveData.dropSize[i][j]);
    }
  }
  incomingPacketFinish(packet);
}

void valve_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {

    // Handle incoming packets
    uint32_t val = 0;
    CAPacketElement *packet = processIncomingPacket();
    packet = incomingPacketCheckUint32(packet, 26, val);
    incomingPacketFinish(packet);

    if (val) {
      triggerCameras();
      delay(10);
      uint32_t startTime = millis();
      uint32_t t[6];  // t[0] is end of delay drop 0 ;; t[1] is end of size drop 0 ;; continued for each drop
      for(uint8_t i=0; i<4; ++i) {  // Valves
        for(uint8_t j=0; j<3; ++j) { //Drops
          t[j*2+0] = startTime+gValveData.dropDelay[i][j];
          t[j*2+1] = t[j*2+0]+gValveData.dropSize[i][j];
        }
        uint32_t curTime = millis();
        if (curTime >= t[5]) { CAU::digitalWrite(gValveData.ppValve[i], LOW); }
        else if (curTime >= t[4]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
        else if (curTime >= t[3]) { CAU::digitalWrite(gValveData.ppValve[i], LOW); }
        else if (curTime >= t[2]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
        else if (curTime >= t[1]) { CAU::digitalWrite(gValveData.ppValve[i], LOW); }
        else if (curTime >= t[0]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
      }
    }
  }
}

#endif //MENUS_H

