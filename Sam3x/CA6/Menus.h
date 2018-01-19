////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Logic for running dynamic menus.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MENUS_H
#define MENUS_H

#include <CASensorFilter.h>
#include "Context.h"
#include "PacketProcessor.h"

#define FAST_CHECK_FOR_PACKETS (Serial2.available())

//////////////////////////////
// executeLimitAt - Helper function to limit how often code executes
//////////////////////////////

bool executeLimitAt(uint32_t updateFrequency) {
  uint32_t curTime = millis();
  static uint32_t nextUpdate = millis();
  bool ret = false;
  
  if ((curTime >= nextUpdate) || (curTime-nextUpdate < updateFrequency*1024)) { // Handles wraparounds
    ret = true;
    nextUpdate = curTime + updateFrequency;
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dev Menu - A menu that demonstrates how to make a menu
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppPin;
} DevData;

DevData gDevData;

const char* dev_Name() {
  return "Dev Menu";
}

void dev_MenuInit() {
  // This menu has no IO to setup
}

void dev_PhotoInit() {
  gDevData.ppPin = CAU::getModulePin(0, 5);
  CAU::pinMode(gDevData.ppPin, INPUT_PULLUP);
}

void dev_MenuRun() {
  static uint32_t count = 0;

  if (executeLimitAt(1000)) {
    g_ctx.packetHelper.writePacketString(0, String(count++).c_str());
  }

  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  uint32_t val, sec, nano;
  packet = incomingPacketCheckUint32(packet, 1, val);
  packet = incomingPacketCheckUint32(packet, 2, val);
  packet = incomingPacketCheckUint32(packet, 3, val);
  packet = incomingPacketCheckUint32(packet, 4, val);
  packet = incomingPacketCheckUint32(packet, 5, val);
  packet = incomingPacketCheckTimeBox(packet, 6, sec, nano);
  packet = incomingPacketCheckUint32(packet, 7, val);
  incomingPacketFinish(packet);
}

void dev_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint8_t val = CAU::digitalRead(gDevData.ppPin);
    if (val == LOW) {
      triggerCameras();
    }

    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sound Menu - Detects sound
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppPin;      // This is the port and in where the analog sound value comes from
  CASensorFilter sf;    // This helps filter incoming values for a cleaner display
  uint32_t triggerVal;  // This stores the amount of sound change required to trigger the CA6
} SoundData;

SoundData gSoundData;

const char* sound_Name() {
  return "Sound Menu";
}

void sound_MenuInit() {
  gSoundData.ppPin = CAU::getModulePin(0, 0);    // Module 0 pin 0 is where the analog sound values are
  CAU::pinMode(gSoundData.ppPin, ANALOG_INPUT);
  gSoundData.sf.init(gSoundData.ppPin, CASensorFilter::ANALOG_THRESHOLD, 2000);  // Update display ever 2000 ms
  gSoundData.sf.setThreshold(2048);  // Analog sound can range from 0 to 4095.  No sound is at a 2048 value
}

void sound_PhotoInit() {
  gSoundData.ppPin = CAU::getModulePin(0, 0);  // Same settings as menu init since it's possible to skip right to photo mode and skip menu mode
  CAU::pinMode(gSoundData.ppPin, ANALOG_INPUT);
}

void sound_MenuRun() {
  uint16_t val = gSoundData.sf.getSensorData();

  // Handle outgoing packets
  if (executeLimitAt(500)) {
    // Every 500 ms send a packet to the webserver so it can display the filtered current value
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
  }

  // Handle incoming packets from webserver
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gSoundData.triggerVal); // Store the trigger value user set on webpage here
  incomingPacketFinish(packet);
}

void sound_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::analogRead(gSoundData.ppPin);
    val = (val >= 2048) ? (val-2048) : (2048-val);  // Bias value to have a center at 2048 (no sound for this sensor)

    bool trigger = (val >= gSoundData.triggerVal) ? true : false;
    if (trigger) {
      triggerCameras();
    }

    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets (needed so user can exit photo mode)
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Light Menu - Detects light
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppPin;            // This is the port for analog light values
  hwPortPin ppSensitivity0;   // This is a sensitivity control pin
  hwPortPin ppSensitivity1;   // This is a sensitivity control pin
  hwPortPin ppSensitivity2;   // This is a sensitivity control pin
  hwPortPin ppLaser;          // This port enables/disables the laser
  CASensorFilter sf;          // This helps filter incoming values for a cleaner display
  uint32_t triggerVal;        // This stores the amount of light change required to trigger the CA6
  uint32_t laserVal;          // Thi stores if the laser is off or on
  uint32_t triggerMode=0;     // This stores the mode (min/max/threshold) used to trigger the CA6
  uint32_t sensitivity=0;     // This stores the sensitivity level 0=low, 1=medium-low, 2=medium-high, 3=high
} LightData;

LightData gLightData;

const char* light_Name() {
  return "Light Menu";
}

void setLightSensitivity(uint32_t sensitivity, hwPortPin ppSensitivity0, hwPortPin ppSensitivity1, hwPortPin ppSensitivity2) {
  CAU::pinMode(ppSensitivity0, OUTPUT);
  CAU::pinMode(ppSensitivity1, OUTPUT);
  CAU::pinMode(ppSensitivity2, OUTPUT);
  
  // Low resistance is low sensitivity on the sensor.  See schematic for resistor ladder setup
  // ppSensitivity0 is the buffer -- LOW to sink to ground for lowest sensitivity
  // ppSensitivity1 is connected directly to sam3x io pin -- pull LOW to set medium sensitivity otherwise high impedance
  
  if (sensitivity == 0) { // Low Sensitivity
    CAU::digitalWrite(ppSensitivity0, LOW);
    CAU::digitalWrite(ppSensitivity1, HIGH);
    CAU::digitalWrite(ppSensitivity2, HIGH);
  }
  else if (sensitivity == 1) { // Medium-Low Sensitivity
    CAU::digitalWrite(ppSensitivity0, HIGH);
    CAU::digitalWrite(ppSensitivity1, LOW);
    CAU::digitalWrite(ppSensitivity2, HIGH);
  }
  else if (sensitivity == 2) { // Medium-High Sensitivity
    CAU::digitalWrite(ppSensitivity0, HIGH);
    CAU::digitalWrite(ppSensitivity1, HIGH);
    CAU::digitalWrite(ppSensitivity2, LOW);
  }
  else { // High Sensitivity
    CAU::digitalWrite(ppSensitivity0, HIGH);
    CAU::digitalWrite(ppSensitivity1, HIGH);
    CAU::digitalWrite(ppSensitivity2, HIGH);
  }
}

void light_MenuInit() {
  gLightData.ppPin = CAU::getModulePin(0, 0);           // Module 0 pin 0 is where the analog light values
  gLightData.ppSensitivity0 = CAU::getModulePin(0, 2);  // Module 0 pin 2 is where sensitivity control 0 is
  gLightData.ppSensitivity1 = CAU::getModulePin(0, 3);  // Module 0 pin 3 is where sensitivity control 1 is
  gLightData.ppLaser = CAU::getModulePin(0, 1);         // Module 0 pin 1 is where the laser is controlled
  CAU::pinMode(gLightData.ppPin, ANALOG_INPUT);
  setLightSensitivity(gLightData.sensitivity, gLightData.ppSensitivity0, gLightData.ppSensitivity1, gLightData.ppSensitivity2);
  gLightData.sf.init(gLightData.ppPin, CASensorFilter::ANALOG_MIN, 2000);  // Update display ever 2000 ms
  CAU::pinMode(gLightData.ppLaser, OUTPUT);
  CAU::digitalWrite(gLightData.ppLaser, LOW);
}

void light_PhotoInit() {
  gLightData.ppPin = CAU::getModulePin(0, 0);           // Module 0 pin 0 is where the analog light values are
  gLightData.ppSensitivity0 = CAU::getModulePin(0, 2);  // Module 0 pin 2 is where sensitivity control 0 is
  gLightData.ppSensitivity1 = CAU::getModulePin(0, 3);  // Module 0 pin 3 is where sensitivity control 1 is
  gLightData.ppLaser = CAU::getModulePin(0, 1);         // Module 0 pin 1 is where the laser is controlled
  CAU::pinMode(gLightData.ppPin, ANALOG_INPUT);
  setLightSensitivity(gLightData.sensitivity, gLightData.ppSensitivity0, gLightData.ppSensitivity1, gLightData.ppSensitivity2);
  CAU::pinMode(gLightData.ppLaser, OUTPUT);
   CAU::digitalWrite(gLightData.ppLaser, LOW);
}

void light_MenuRun() {
  uint16_t val = gLightData.sf.getSensorData();
  uint32_t prevTriggerMode = gLightData.triggerMode;
  uint32_t prevSensitivity = gLightData.sensitivity;

  // Handle outgoing packets
  if (executeLimitAt(500)) {
    // Every 500 ms send a packet to the webserver so it can display the filtered current value
    g_ctx.packetHelper.writePacketString(3, String(val).c_str());
  }

  // Handle incoming packets from webserver
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gLightData.triggerMode); // Store the trigger mode user set on webpage here
  packet = incomingPacketCheckUint32(packet, 1, gLightData.sensitivity); // Store the sensitivity level  user set on webpage here
  packet = incomingPacketCheckUint32(packet, 2, gLightData.triggerVal); // Store the trigger value user set on webpage here
  packet = incomingPacketCheckUint32(packet, 4, gLightData.laserVal); // Store the laser value user set on webpage here
  
  incomingPacketFinish(packet);

  // Update the sensitivity if it has changed
  if (prevSensitivity != gLightData.sensitivity) {
    setLightSensitivity(gLightData.sensitivity, gLightData.ppSensitivity0, gLightData.ppSensitivity1, gLightData.ppSensitivity2);
  }

  if (gLightData.laserVal == 0) {
    CAU::digitalWrite(gLightData.ppLaser, LOW);
  }
  else {
    CAU::digitalWrite(gLightData.ppLaser, HIGH);
  }

  // Update the display based on current mode
  if (prevTriggerMode != gLightData.triggerMode) {
    if (gLightData.triggerMode == 0) {  // Min Mode
      gLightData.sf.init(gLightData.ppPin, CASensorFilter::ANALOG_MIN, 2000);  // Update display ever 2000 ms
    }
    else { // Max Mode
      gLightData.sf.init(gLightData.ppPin, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
    }
  }
}

void light_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    bool trigger;
    uint16_t val = CAU::analogRead(gLightData.ppPin);

    if (!camTriggerRunning() && gLightData.laserVal) {
       CAU::digitalWrite(gLightData.ppLaser, HIGH);
    } 
    else {
       CAU::digitalWrite(gLightData.ppLaser, LOW);
    }

    if (gLightData.triggerMode == 0) {  // Min Mode
      trigger = (val < gLightData.triggerVal) ? true : false;
    }
    else { // Max Mode
      trigger = (val > gLightData.triggerVal) ? true : false;
    }

    if (trigger) {
      triggerCameras();
    }

    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets (needed so user can exit photo mode)
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
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
  uint32_t val = gVibrationData.sf.getSensorData();

  // Handle outgoing packets
  if (executeLimitAt(1000)) {
    val = map(val, 1, 4095, 1, 1000);  // Convert the values used on the micro to values on webpage
    g_ctx.packetHelper.writePacketString(1, String(val).c_str());
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

    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Valve Menu - Triggers Valve
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppValve[4];
  uint32_t shutterLag;
  uint32_t flashDelay;
  uint32_t dropDelay[4][3];
  uint32_t dropSize[4][3];
  uint8_t autoState = 0;
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
  g_ctx.packetHelper.writePacketString(27, String(gValveData.flashDelay).c_str());
  g_ctx.packetHelper.writePacketString(30, String(gValveData.dropDelay[0][1]).c_str());
  g_ctx.packetHelper.writePacketString(34, "Not Active");
}

void valve_MenuRun() {
  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gValveData.shutterLag);
  packet = incomingPacketCheckUint32(packet, 1, gValveData.flashDelay);
  for (uint8_t i=0; i<4; ++i) { //Valves (max is 4)
    for (uint8_t j=0; j<3; ++j) { //Drops (max is 3)
      packet = incomingPacketCheckUint32(packet, 2+i*3*2+j*2+0, gValveData.dropDelay[i][j]);
      packet = incomingPacketCheckUint32(packet, 2+i*3*2+j*2+1, gValveData.dropSize[i][j]);
    }
  }
  incomingPacketFinish(packet);
}

void valve_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {

    // Handle incoming packets
    uint32_t dropButton = 0;

    if (FAST_CHECK_FOR_PACKETS) {
      uint32_t incFlashButton = 0;
      uint32_t decFlashButton = 0;
      uint32_t incValveButton = 0;
      uint32_t decValveButton = 0;
      uint32_t autoButton = 0;
      
      CAPacketElement *packet = processIncomingPacket();
      packet = incomingPacketCheckUint32(packet, 26, dropButton);
      packet = incomingPacketCheckUint32(packet, 28, incFlashButton);
      packet = incomingPacketCheckUint32(packet, 29, decFlashButton);
      packet = incomingPacketCheckUint32(packet, 31, incValveButton);
      packet = incomingPacketCheckUint32(packet, 32, decValveButton);
      packet = incomingPacketCheckUint32(packet, 33, autoButton);
      incomingPacketFinish(packet);

      if (incFlashButton) {
        gValveData.flashDelay = min((gValveData.flashDelay+5)/5*5, 9999);
        g_ctx.packetHelper.writePacketUint32(1, gValveData.flashDelay);
        g_ctx.packetHelper.writePacketString(27, String(gValveData.flashDelay).c_str());
      }
      if (decFlashButton) {
        gValveData.flashDelay = (gValveData.flashDelay <= 5) ? 0 : ((gValveData.flashDelay-1)/5*5);
        g_ctx.packetHelper.writePacketUint32(1, gValveData.flashDelay);
        g_ctx.packetHelper.writePacketString(27, String(gValveData.flashDelay).c_str());
      }
      if (incValveButton) {
        gValveData.dropDelay[0][1] = min((gValveData.dropDelay[0][1]+5)/5*5, 999);
        g_ctx.packetHelper.writePacketUint32(4, gValveData.dropDelay[0][1]);
        g_ctx.packetHelper.writePacketString(30, String(gValveData.dropDelay[0][1]).c_str());
      }
      if (decValveButton) {
        gValveData.dropDelay[0][1] = (gValveData.dropDelay[0][1] == 0) ? 0 : ((gValveData.dropDelay[0][1]-1)/5*5);
        g_ctx.packetHelper.writePacketUint32(4, gValveData.dropDelay[0][1]);
        g_ctx.packetHelper.writePacketString(30, String(gValveData.dropDelay[0][1]).c_str());
      }
      if (autoButton) {
        switch(gValveData.autoState) {
          case 0:
            gValveData.autoState = 1;
            g_ctx.packetHelper.writePacketString(34, "Press Auto at max splash height");
            gValveData.flashDelay = 250;
            g_ctx.packetHelper.writePacketUint32(1, 250);
            g_ctx.packetHelper.writePacketString(27, "250");
            gValveData.dropDelay[0][1] = 150;
            g_ctx.packetHelper.writePacketUint32(4, 150);
            g_ctx.packetHelper.writePacketString(30, "150");
            break;
          case 1:
            gValveData.autoState = 2;
            g_ctx.packetHelper.writePacketString(34, "Press Auto at nice collision");
            break;
         default:
            gValveData.autoState = 0;
            g_ctx.packetHelper.writePacketString(34, "Not Active");
            break;
        }
        
      }
    }

    if(gValveData.autoState && !camTriggerRunning()) {
      switch (gValveData.autoState) {
        case 1:
          gValveData.flashDelay = min((gValveData.flashDelay+10), 450);
          g_ctx.packetHelper.writePacketUint32(1, gValveData.flashDelay);
          g_ctx.packetHelper.writePacketString(27, String(gValveData.flashDelay).c_str());
          break;
        case 2:
          gValveData.dropDelay[0][1] = (gValveData.dropDelay[0][1] == 0) ? 0 : ((gValveData.dropDelay[0][1]-10));
          g_ctx.packetHelper.writePacketUint32(4, gValveData.dropDelay[0][1]);
          g_ctx.packetHelper.writePacketString(30, String(gValveData.dropDelay[0][1]).c_str());
          break;
      }
      dropButton = 1; // Forces the valve to run
    }

    if (dropButton && !camTriggerRunning()) {
      if (gValveData.shutterLag <= gValveData.flashDelay) {
        // This is the normal path
        uint64_t camDelayMs = gValveData.flashDelay - gValveData.shutterLag;
        uint64_t flashDelayMs = gValveData.flashDelay;
        setupCamTiming(camDelayMs*1000000, flashDelayMs*1000000);
        triggerCameras();
      }
      else {
        // In this case shuter lag is so long we need to introduce an
        // extra delay to give the shutter time to open
        uint64_t valveDelayMs = gValveData.shutterLag - gValveData.flashDelay;
        uint64_t camDelayMs = 0;
        uint64_t flashDelayMs = gValveData.flashDelay + valveDelayMs;
        setupCamTiming(camDelayMs*1000000, flashDelayMs*1000000);
        triggerCameras();
        delay(valveDelayMs);
      }
  
      bool done[4] = {false,false,false,false};
      uint32_t startTime = millis();
      uint32_t t[6];  // t[0] is end of delay drop 0 ;; t[1] is end of size drop 0 ;; continued for each drop
      while ((done[0] == false) || (done[1] == false) || (done[2] == false) || (done[3] == false))
      {
        for(uint8_t i=0; i<4; ++i) {  // Valves
          uint32_t prevTime = startTime;
          for(uint8_t j=0; j<3; ++j) { //Drops
            prevTime = (t[j*2+0] = prevTime+gValveData.dropDelay[i][j]);
            prevTime = (t[j*2+1] = prevTime+gValveData.dropSize[i][j]);
          }
          uint32_t curTime;
          curTime = millis();
          if (curTime >= t[5]) {
            CAU::digitalWrite(gValveData.ppValve[i], LOW);
            done[i] = true;
          }
          else if (curTime >= t[4]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
          else if (curTime >= t[3]) { CAU::digitalWrite(gValveData.ppValve[i], LOW); }
          else if (curTime >= t[2]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
          else if (curTime >= t[1]) { CAU::digitalWrite(gValveData.ppValve[i], LOW); }
          else if (curTime >= t[0]) { CAU::digitalWrite(gValveData.ppValve[i], HIGH); }
        }
      }
    }
  }
  setupCamTiming(); // Restore the camera/flash timings back to the correct values
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Lightning Menu (Contributed by Dan Lenardon)
//  Release Notes
//  * First version for CA6
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////
// A specific threshold algorithm for Lightning.
// Uses a light module in Module 0 and triggers all enabled camera ports
////////////////////////////////////////

typedef struct {
  uint32_t triggerDiffThreshold;
  hwPortPin ppLight;
  hwPortPin ppSensitivity0;   // This is a sensitivity control pin
  hwPortPin ppSensitivity1;   // This is a sensitivity control pin
  hwPortPin ppSensitivity2;   // This is a sensitivity control pin
  uint16_t sensorVal = 0;
  uint32_t referenceUpdateTimeMS;
  uint16_t referenceSensorVal = 0;
  uint16_t triggerCount = 0;
  boolean inStrikeCycle = false;        // logical indicating that we are in a strike cycle
  char strikeDetailsBuf[5][32]; // circular buffer of the details for the last 5 strikes
  int16_t peakOfStrike = 0;
  uint16_t refAtStrike = 0;
  uint32_t strikeStartTimeMS; // used for strike duration display and 2-second max strike duration to prevent lock-up with ambient or sensitivity changes
  uint32_t sensitivity=0;     // This stores the sensitivity level 0=low, 1=medium-low, 2=medium-high, 3=high
  char sensitivityStr [4][9] = {"LOW", "MED-LOW", "MED-HIGH", "HIGH"};
  const int16_t workingMaxSensorVal = 4015; // light sensors saturate before reaching max of 4096 - measured for Vishay TEPT4400
} LightningData;

LightningData gLightningData;

void autoLightSensitivity(uint16_t val, uint32_t& sensitivity) {
  // Adjust sensitivity to try to get the light value in the middle range of 1000-3000 (out of 4096)
  if(val < 1000 && sensitivity < 3) sensitivity++;
  if(val > 3000 && sensitivity > 0) sensitivity--;
}

const char* lightning_Name() {
  return "Lightning Menu";
}

void lightning_MenuInit() {
  gLightningData.ppLight = CAU::getModulePin(0, 0);
  CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
  setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
  // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void lightning_PhotoInit() {
  gLightningData.ppLight = CAU::getModulePin(0, 0);
  CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
  setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
  // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void lightning_MenuRun() {
  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gLightningData.triggerDiffThreshold);
  incomingPacketFinish(packet);

  // Handle outgoing packets once per second
  if (executeLimitAt(1000)) {
    gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
    autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
    setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
    // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
    gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
    g_ctx.packetHelper.writePacketString(1, String(gLightningData.sensorVal).c_str());
    g_ctx.packetHelper.writePacketString(2, gLightningData.sensitivityStr[gLightningData.sensitivity]);
  }

  // Still may need to deal with Camera settings, e.g. want to default to Focus active, no delay, etc.
}

  //LTGDisplayPhotoMode Send photo-mode data back to mobile display all values obtained from gLightningData
void LTGDisplayPhotoMode() {
  // Handle outgoing packets
  g_ctx.packetHelper.writePacketString(3, String(gLightningData.sensorVal).c_str());
  g_ctx.packetHelper.writePacketString(4, gLightningData.sensitivityStr[gLightningData.sensitivity]);
  
  // Check if we need to display warnings or suggestions for snoot or trigger level
  if (gLightningData.sensitivity == 0) {  // sensitivity=0 (LOW) means bright light; suggest extending the snoot
    g_ctx.packetHelper.writePacketString(5, "** Consider extending snoot and pointing at storm clouds **");
  }
  else {
    g_ctx.packetHelper.writePacketString(5, " "); // blank out the message
  }
  if ((gLightningData.referenceSensorVal + gLightningData.triggerDiffThreshold) >= gLightningData.workingMaxSensorVal) {
    // At top of Sensor range - trigVal too high - can't trigger
    g_ctx.packetHelper.writePacketString(6, "** Trigger Threshold may be too high -- consider lowering it **");
  }
  else {
    g_ctx.packetHelper.writePacketString(6, " ");
  }
  
  g_ctx.packetHelper.writePacketString(7, String(gLightningData.triggerCount).c_str());
  g_ctx.packetHelper.writePacketString(8, gLightningData.strikeDetailsBuf[gLightningData.triggerCount % 5]);
  g_ctx.packetHelper.writePacketString(9, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 1) % 5]);
  g_ctx.packetHelper.writePacketString(10, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 2) % 5]);
  g_ctx.packetHelper.writePacketString(11, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 3) % 5]);
  g_ctx.packetHelper.writePacketString(12, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 4) % 5]);
}

void lightning_PhotoRun() {
  #define UPDATEREFPERIODMS 200 // Update the Reference Base value every 200 MS to adjust for ambient changes including moving clouds
  #define DURATIONOFFSET 3 // Microseconds to add as 1/2 the estimated time between AnalogReads in strike detection loop

  uint32_t curTimeMS = millis();
  int16_t currentDif = 0;
  uint32_t strikeDurMS = 0;
  uint32_t decimalUS = 0;

  gLightningData.triggerCount = 0;
  // Clear the strike details buffer from prior runs
  for(int8_t i = 0; i < 5; i++) {
    gLightningData.strikeDetailsBuf[i] [0] = '\0';
  }
  gLightningData.referenceSensorVal = CAU::analogRead(gLightningData.ppLight);  // initialize reference base
  gLightningData.referenceUpdateTimeMS = curTimeMS + UPDATEREFPERIODMS;  // initialize the update timer

  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Loop checking for a strike (curval - ref > trigger)
    while (!gLightningData.inStrikeCycle && g_ctx.state == CA_STATE_PHOTO_MODE) {
      curTimeMS = millis();  // capture the current time
      gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
      currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
      if (currentDif > (int16_t) gLightningData.triggerDiffThreshold) {
        // Begin a new strike -- strike lasts until sensor1 goes back below threshold or 2 seconds (bail out)
        gLightningData.inStrikeCycle = true;
        //Trigger Cameras
        triggerCameras();
        gLightningData.triggerCount = gLightningData.triggerCount + 1;
        gLightningData.peakOfStrike = gLightningData.sensorVal; // initialize the peak reading for this strike
        gLightningData.strikeStartTimeMS = curTimeMS;  // start clock for strike duration and 2-second max check        
        gLightningData.refAtStrike = gLightningData.referenceSensorVal;  // save reference value at beginning of strike
        break;
      }

      // Not yet in a strike event
      // Is it time to update the Reference Value?
      if (curTimeMS >= gLightningData.referenceUpdateTimeMS) {
        autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
        setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
        // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
        gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
        gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
        gLightningData.referenceUpdateTimeMS = curTimeMS + UPDATEREFPERIODMS;  // Update Timer 
      }

      //Only display details once per second
      if (executeLimitAt(1000)) {
        LTGDisplayPhotoMode();
      }
      if (FAST_CHECK_FOR_PACKETS) {
        // Handle incoming packets
        CAPacketElement *packet = processIncomingPacket();
        incomingPacketFinish(packet);
      }
    }  // End of loop looking for start of a strike

    // Begin loop looking for end of strike and handling DeviceCycles
    while (gLightningData.inStrikeCycle && g_ctx.state == CA_STATE_PHOTO_MODE) {
      gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
      gLightningData.peakOfStrike = max(gLightningData.peakOfStrike, gLightningData.sensorVal);
      currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
      // Strike is done if current value is back below threshold or 2 seconds max to prevent lockups
      if (currentDif <= (int16_t) gLightningData.triggerDiffThreshold) {
        // Lightning may often have multiple short flashes/pulses, so this might just be the end of the first pulse
        gLightningData.inStrikeCycle = false;
        break;
      }
      curTimeMS = millis();
      if (curTimeMS > (gLightningData.strikeStartTimeMS + 2000)) {
        // Sensor value has been above threshold for >2 seconds - must be an ambient or sensor sensitivity change
        // Just end the strike and reset the Reference level
        gLightningData.inStrikeCycle = false;
        autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
        setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
        // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
        gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
        gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
        gLightningData.referenceUpdateTimeMS = curTimeMS + UPDATEREFPERIODMS; // reset the Ref Update clock
        break;
      }

      if (FAST_CHECK_FOR_PACKETS) {
        // Handle incoming packets
        CAPacketElement *packet = processIncomingPacket();
        incomingPacketFinish(packet);
      }
    }

    // Strike cycle just finished, store values in character buffer for display
    strikeDurMS = millis() - gLightningData.strikeStartTimeMS;
    sprintf(gLightningData.strikeDetailsBuf[gLightningData.triggerCount%5], "<pre>%4u%5u%5u%5u</pre>\0", gLightningData.triggerCount, gLightningData.refAtStrike, gLightningData.peakOfStrike, strikeDurMS);

    // Loop until DeviceCycles (Bulb TImer) is completed for both devices
    while ( camTriggerRunning() ) {
      curTimeMS = millis();  // capture the current time
      gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Just to keep the current value up to date 
      currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
      // Check if light value has gone back up over threshold -- secondary pulses/flashes
      if (currentDif > (int16_t) gLightningData.triggerDiffThreshold) {
        strikeDurMS = millis() - gLightningData.strikeStartTimeMS;
        gLightningData.peakOfStrike = max(gLightningData.peakOfStrike, gLightningData.sensorVal);
        sprintf(gLightningData.strikeDetailsBuf[gLightningData.triggerCount%5], "<pre>%4u%5u%5u%5u</pre>\0", gLightningData.triggerCount, gLightningData.refAtStrike, gLightningData.peakOfStrike, strikeDurMS);
      }
      delay(1); // Since we are waiting for the BulbSec timer, no need to go any faster than 1 ms
      if (FAST_CHECK_FOR_PACKETS) {
        // Handle incoming packets
        CAPacketElement *packet = processIncomingPacket();
        incomingPacketFinish(packet);
      }

    }
    // Recheck if current value is below threshold and if so reset Reference
    gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
    currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
    if (currentDif < (int16_t) gLightningData.triggerDiffThreshold)
    {
      autoLightSensitivity(gLightningData.sensorVal, gLightningData.sensitivity);
      setLightSensitivity(gLightningData.sensitivity, gLightningData.ppSensitivity0, gLightningData.ppSensitivity1, gLightningData.ppSensitivity2);
      // May have to test whether a small delay might be needed to allow the sensitivity change to take effect (?3 microseconds?)
      gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Re-read light value in case sensitivity has changed
      gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
      gLightningData.referenceUpdateTimeMS = curTimeMS + UPDATEREFPERIODMS; // reset the Ref Update clock
    }
    // If still above threshold, keep old Reference to allow a retrigger to capture more secondary flashes / pulses

  }
  // Out of PHOTO mode
  // clear camera/flash back to default (especially reset camera prefocus)
  initCameraPins();
  gLightningData.peakOfStrike = 0;  // zero out the peak before going back to MENU
  // Now back to MENU mode
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Projectile Menu - Detects projectiles
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppDetect0;        // This is the port where the analog value comes from
  hwPortPin ppDetect1;        // This is the port where the analog value comes from
  hwPortPin ppEmit;           // This is the port to enable an IR LED
  CASensorFilter sf0;         // This helps filter incoming values for a cleaner display
  CASensorFilter sf1;         // This helps filter incoming values for a cleaner display
  uint32_t triggerVal0;       // This stores the amount of change required to trigger the CA6
  uint32_t triggerVal1;       // This stores the amount of change required to trigger the CA6
  uint32_t distToTarget;      // The distance to the target in hundredths of inches
  uint32_t extraTime;         // Extra time offset in microseconds
} ProjectileData;

ProjectileData gProjectileData;

const char* projectile_Name() {
  return "Projectile Menu";
}

void projectile_MenuInit() {
  gProjectileData.ppDetect0 = CAU::getModulePin(2, 0);    // Module 0 pin 0 is where the analog values are
  CAU::pinMode(gProjectileData.ppDetect0, ANALOG_INPUT);
  gProjectileData.ppDetect1 = CAU::getModulePin(2, 1);    // Module 0 pin 1 is where the analog values are
  CAU::pinMode(gProjectileData.ppDetect1, ANALOG_INPUT);

  gProjectileData.ppEmit = CAU::getModulePin(2, 2);
  CAU::pinMode(gProjectileData.ppEmit, OUTPUT);
  CAU::digitalWrite(gProjectileData.ppEmit, HIGH);
  
  gProjectileData.sf0.init(gProjectileData.ppDetect0, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
  gProjectileData.sf1.init(gProjectileData.ppDetect1, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
}

void projectile_PhotoInit() {
  gProjectileData.ppDetect0 = CAU::getModulePin(2, 0);    // Module 0 pin 0 is where the analog values are
  CAU::pinMode(gProjectileData.ppDetect0, ANALOG_INPUT);
  gProjectileData.ppDetect1 = CAU::getModulePin(2, 1);    // Module 0 pin 1 is where the analog values are
  CAU::pinMode(gProjectileData.ppDetect1, ANALOG_INPUT);

  gProjectileData.ppEmit = CAU::getModulePin(2, 2);
  CAU::pinMode(gProjectileData.ppEmit, OUTPUT);
  CAU::digitalWrite(gProjectileData.ppEmit, HIGH);
  
  gProjectileData.sf0.init(gProjectileData.ppDetect0, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
  gProjectileData.sf1.init(gProjectileData.ppDetect1, CASensorFilter::ANALOG_MAX, 2000);  // Update display ever 2000 ms
}

void projectile_MenuRun() {
  uint16_t val0 = gProjectileData.sf0.getSensorData();
  uint16_t val1 = gProjectileData.sf1.getSensorData();

  // Handle outgoing packets
  if (executeLimitAt(500)) {
    // Every 500 ms send a packet to the webserver so it can display the filtered current value
    g_ctx.packetHelper.writePacketString(1, String(val0).c_str());
    g_ctx.packetHelper.writePacketString(3, String(val1).c_str());
  }

  // Handle incoming packets from webserver
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gProjectileData.triggerVal0);   // Store the trigger value user set on webpage here
  packet = incomingPacketCheckUint32(packet, 2, gProjectileData.triggerVal1);   // Store the trigger value user set on webpage here
  packet = incomingPacketCheckUint32(packet, 4, gProjectileData.distToTarget);  // Store the trigger value user set on webpage here
  packet = incomingPacketCheckUint32(packet, 5, gProjectileData.extraTime);     // Store the trigger value user set on webpage here
  incomingPacketFinish(packet);
}

void projectile_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    uint32_t startTime=0;
    uint32_t endTime=0;
    bool firstSensor = false;
    bool secondSensor = false;

    // Loop to check for sensor values
    while (1) {
      if (!firstSensor) {
        if (CAU::analogRead(gProjectileData.ppDetect0) >= gProjectileData.triggerVal0) {
          startTime = CLOCK_TICKS;
          firstSensor = true;
        }
      }
      else {
        if (CAU::analogRead(gProjectileData.ppDetect1) >= gProjectileData.triggerVal1) {
          endTime = CLOCK_TICKS;
          secondSensor = true;
          break;
        }
        if ((CLOCK_TICKS-startTime) > (500*1000*TICKS_PER_MICROSECOND)) {
          // Took 1 second which is too long so we need to timeout
          break;
        }
      }
  
      if (FAST_CHECK_FOR_PACKETS) {
        break;
      }
    }

    if (firstSensor && secondSensor) {
      const uint32_t distanceBetweenSensors = 4;
      uint32_t elapsedTicks = endTime-startTime;
      uint32_t inchPerSec = distanceBetweenSensors*TICKS_PER_MICROSECOND*1000000/elapsedTicks;
      uint32_t ticksDelay = gProjectileData.distToTarget*10000*TICKS_PER_MICROSECOND/inchPerSec; // distToTarget/inchesPerSec (then adjust for 100ths of inches in distance and convert sec to ticks)
      ticksDelay += gProjectileData.extraTime*TICKS_PER_MICROSECOND;
      
      while(CLOCK_TICKS-endTime < ticksDelay) { //Delay in loop until it's time to trigger camera
        if (FAST_CHECK_FOR_PACKETS) { break; }
      }
      triggerCameras();

      if (executeLimitAt(500)) {
        g_ctx.packetHelper.writePacketString(6, String(inchPerSec/12).c_str());  // Feet/sec
        g_ctx.packetHelper.writePacketString(7, String(inchPerSec*254/100/100).c_str());  // Meters/sec
      }
    }
    else if (firstSensor && !secondSensor) {
      if (executeLimitAt(500)) {
        g_ctx.packetHelper.writePacketString(6, "Second sensor");  // Feet/sec
        g_ctx.packetHelper.writePacketString(7, "not triggered");  // Meters/sec
      }
    }
  
    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets (needed so user can exit photo mode)
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Beam Menu - Detects IR Beam light
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct {
  hwPortPin ppDetect;   // This is the pin used to detect light
  hwPortPin ppEmit;     // This is the pin used to enable the IR LED
  CASensorFilter sf;    // This helps filter incoming values for a cleaner display
  uint32_t triggerVal;  // This stores value to detect light 0=low, 1=high
} BeamData;

BeamData gBeamData;

const char* beam_Name() {
  return "Beam Menu";
}

void isrGenerateSquareWave() {
  static uint8_t curVal = HIGH;
  curVal = (curVal==HIGH) ? LOW : HIGH; // Toggle value every time this is called
  CAU::digitalWrite(gBeamData.ppEmit, curVal);
}

void beam_MenuInit() {
  gBeamData.ppDetect = CAU::getModulePin(0, 0);    // Module 0 pin 0 is where the analog values are
  CAU::pinMode(gBeamData.ppDetect, INPUT);
  gBeamData.ppEmit = CAU::getModulePin(0, 1);      // Module 0 pin 1 turns on the IR LED
  CAU::pinMode(gBeamData.ppEmit, OUTPUT);
  CAU::digitalWrite(gBeamData.ppEmit, LOW);
  g_ctx.menuTimer.stop();
  uint64_t ticks = CATickTimer::convertTimeToTicks(0, 13000); // 38Khz with 50% duty cycle
  g_ctx.menuTimer.start(isrGenerateSquareWave, ticks, true);
}

void beam_PhotoInit() {
  gBeamData.ppDetect = CAU::getModulePin(0, 0);    // Module 0 pin 0 is where the analog values are
  CAU::pinMode(gBeamData.ppDetect, INPUT);
  gBeamData.ppEmit = CAU::getModulePin(0, 1);      // Module 0 pin 1 turns on the IR LED
  CAU::pinMode(gBeamData.ppEmit, OUTPUT);
  CAU::digitalWrite(gBeamData.ppEmit, LOW);
  g_ctx.menuTimer.stop();
  uint64_t ticks = CATickTimer::convertTimeToTicks(0, 13000); // 38Khz with 50% duty cycle
  g_ctx.menuTimer.start(isrGenerateSquareWave, ticks, true);
}

void beam_MenuRun() {
  // Handle outgoing packets
  if (executeLimitAt(250)) {
    // Every 250 ms send a packet to the webserver so it can display the current value
    uint8_t val = CAU::digitalRead(gBeamData.ppDetect);
    if (val == LOW) {
      g_ctx.packetHelper.writePacketString(1, "LOW");      
     }
    else {
      g_ctx.packetHelper.writePacketString(1, "HIGH");
    }
  }

  // Handle incoming packets from webserver
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gBeamData.triggerVal); // Store the trigger value user set on webpage here
  incomingPacketFinish(packet);
}

void beam_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint16_t val = CAU::digitalRead(gBeamData.ppDetect);

    if (val == gBeamData.triggerVal) {
      triggerCameras();
    }
    
    if (FAST_CHECK_FOR_PACKETS) {
      // Handle incoming packets (needed so user can exit photo mode)
      CAPacketElement *packet = processIncomingPacket();
      incomingPacketFinish(packet);
    }
  }
}


#endif //MENUS_H

