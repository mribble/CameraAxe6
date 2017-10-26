#ifndef MENUS_H
#define MENUS_H

#include <CASensorFilter.h>
#include "Context.h"
#include "PacketProcessor.h"

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
  packet = incomingPacketCheckTimeBox(packet, 5, sec, nano);
  packet = incomingPacketCheckUint32(packet, 6, val);
  incomingPacketFinish(packet);
}

void dev_PhotoRun() {
  while (g_ctx.state == CA_STATE_PHOTO_MODE) {
    // Handle triggering
    uint8_t val = CAU::digitalRead(gDevData.ppPin);
    if (val == LOW) {
      triggerCameras();
    }

    // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
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

    // Handle incoming packets (needed so user can exit photo mode)
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

    // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
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
      packet = incomingPacketCheckUint32(packet, 2+i*3*2+j*2+0, gValveData.dropDelay[i][j]);
      packet = incomingPacketCheckUint32(packet, 2+i*3*2+j*2+1, gValveData.dropSize[i][j]);
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
      bool done[4] = {false,false,false,false};
      triggerCameras();
      uint32_t startTime = millis();
      uint32_t t[6];  // t[0] is end of delay drop 0 ;; t[1] is end of size drop 0 ;; continued for each drop
      while ((done[0] != true) && (done[1] != true) && (done[2] != true) && (done[3] != true))
      {
        uint32_t prevTime = startTime;
        for(uint8_t i=0; i<4; ++i) {  // Valves
          for(uint8_t j=0; j<3; ++j) { //Drops
            prevTime = t[j*2+0] = prevTime+gValveData.dropDelay[i][j];
            prevTime = t[j*2+1] = prevTime+gValveData.dropSize[i][j];
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
  uint16_t sensorVal = 0;
  uint32_t updateRefPeriodMS = 200;
  uint32_t referenceUpdateTimeMS;
  uint16_t referenceSensorVal = 0;
  uint16_t triggerCount = 0;
  boolean inStrikeCycle = false;        // logical indicating that we are in a strike cycle
  boolean TriggerTooHigh = false; // or Sensitivity too high
  char strikeDetailsBuf[5][27]; // circular buffer of the details for the last 5 strikes
  int16_t peakOfStrike = 0;
  uint16_t refAtStrike = 0;
  uint32_t strikeStartTimeUS;  // used for strike duration display
  uint32_t strikeStartTimeMS; // used for 2-second max strike duration to prevent lock-up with ambient or sensitivity changes

  const int16_t workingMaxSensorVal = 4015; // light sensors saturate before reaching max of 4096 - measured for Vishay TEPT4400
} LightningData;

LightningData gLightningData;

const char* lightning_Name() {
  return "Lightning Menu";
}

void lightning_MenuInit() {
  gLightningData.ppLight = CAU::getModulePin(0, 0);
  CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void lightning_PhotoInit() {
  gLightningData.ppLight = CAU::getModulePin(0, 0);
  CAU::pinMode(gLightningData.ppLight, ANALOG_INPUT);
  gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
  gLightningData.referenceSensorVal = gLightningData.sensorVal;
}

void lightning_MenuRun() {
  // Handle incoming packets
  CAPacketElement *packet = processIncomingPacket();
  packet = incomingPacketCheckUint32(packet, 0, gLightningData.triggerDiffThreshold);
  packet = incomingPacketCheckUint32(packet, 1, gLightningData.updateRefPeriodMS);
  incomingPacketFinish(packet);

  // Handle outgoing packets once per second
  if (executeLimitAt(1000)) {
    gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
    g_ctx.packetHelper.writePacketString(2, String(gLightningData.sensorVal).c_str());
  }

  // Still may need to deal with Camera settings, e.g. want to default to Focus active, no delay, etc.
}

  //LTGDisplayPhotoMode Send photo-mode data back to mobile display all values obtained from gLightningData
void LTGDisplayPhotoMode() {
  // Handle outgoing packets
  g_ctx.packetHelper.writePacketString(3, String(gLightningData.referenceSensorVal).c_str());
  g_ctx.packetHelper.writePacketString(4, String(gLightningData.sensorVal).c_str());
  g_ctx.packetHelper.writePacketString(5, String(gLightningData.referenceSensorVal - gLightningData.sensorVal).c_str());
  g_ctx.packetHelper.writePacketString(6, String(gLightningData.triggerDiffThreshold).c_str());
  g_ctx.packetHelper.writePacketString(7, String(gLightningData.updateRefPeriodMS).c_str());
  g_ctx.packetHelper.writePacketString(8, String(gLightningData.triggerCount).c_str());
  g_ctx.packetHelper.writePacketString(9, gLightningData.strikeDetailsBuf[gLightningData.triggerCount % 5]);
  g_ctx.packetHelper.writePacketString(10, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 1) % 5]);
  g_ctx.packetHelper.writePacketString(11, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 2) % 5]);
  g_ctx.packetHelper.writePacketString(12, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 3) % 5]);
  g_ctx.packetHelper.writePacketString(13, gLightningData.strikeDetailsBuf[(gLightningData.triggerCount - 4) % 5]);

  // Handle incoming packets
    CAPacketElement *packet = processIncomingPacket();
    incomingPacketFinish(packet);
}

void lightning_PhotoRun() {
#define DURATIONOFFSET 3 // Microseconds to add as 1/2 the estimated time between AnalogReads in strike detection loop

  uint32_t curTimeMS = millis();
  int16_t currentDif = 0;
  uint32_t strikeDurUS = 0;
  uint32_t strikeDurMS = 0;
  uint32_t decimalUS = 0;

  gLightningData.triggerCount = 0;
  // Clear the strike details buffer from prior runs
  for(int8_t i = 0; i < 5; i++) {
    gLightningData.strikeDetailsBuf[i] [0] = '\0';
  }
  gLightningData.referenceSensorVal = CAU::analogRead(gLightningData.ppLight);  // initialize reference base
  gLightningData.referenceUpdateTimeMS = curTimeMS + gLightningData.updateRefPeriodMS;  // initialize the update timer

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
        gLightningData.strikeStartTimeUS = micros();  // initialize the strike start for duration measure
        gLightningData.strikeStartTimeMS = curTimeMS;  // start clock for 2-second max check        
        gLightningData.refAtStrike = gLightningData.referenceSensorVal;  // save reference value at beginning of strike
        break;
      }

      // Not yet in a strike event
      // Is it time to update the Reference Value?
      if (curTimeMS >= gLightningData.referenceUpdateTimeMS) {
        gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
        gLightningData.referenceUpdateTimeMS = curTimeMS + (uint32_t)gLightningData.updateRefPeriodMS;  // Update Timer 
        if ((gLightningData.referenceSensorVal + gLightningData.triggerDiffThreshold) >= gLightningData.workingMaxSensorVal)
          // At top of Sensor range - trigVal too high - can't trigger
          gLightningData.TriggerTooHigh = true;  // Display a warning warning, but update ReferenceValue anyway, else continual trigger
        else
          gLightningData.TriggerTooHigh = false;
      }

      //Only display details once per second
      if (executeLimitAt(1000)) {
        LTGDisplayPhotoMode();
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
        gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
        gLightningData.referenceUpdateTimeMS = curTimeMS + gLightningData.updateRefPeriodMS; // reset the Ref Update clock
        break;
      }
    }

    // Strike cycle just finished, store values in character buffer for display
    strikeDurUS = micros() - gLightningData.strikeStartTimeUS + DURATIONOFFSET;
    strikeDurMS = strikeDurUS / 1000;
    decimalUS = strikeDurUS % 1000;
    sprintf(gLightningData.strikeDetailsBuf[gLightningData.triggerCount%5], "%4u%5u%5u%5u.%03u\0", gLightningData.triggerCount, gLightningData.refAtStrike, gLightningData.peakOfStrike, strikeDurMS, decimalUS);

    // Loop until DeviceCycles (Bulb TImer) is completed for both devices
    while ( camTriggerRunning() ) {
      curTimeMS = millis();  // capture the current time
      gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);  // Just to keep the current value up to date 
      currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
      // Check if light value has gone back up over threshold -- secondary pulses/flashes
      if (currentDif > (int16_t) gLightningData.triggerDiffThreshold) {
        strikeDurUS = micros() - gLightningData.strikeStartTimeUS + DURATIONOFFSET;
        strikeDurMS = strikeDurUS / 1000;
        decimalUS = strikeDurUS % 1000;
        gLightningData.peakOfStrike = max(gLightningData.peakOfStrike, gLightningData.sensorVal);
        sprintf(gLightningData.strikeDetailsBuf[gLightningData.triggerCount%5], "%3u %5u %5u %5u.%03u\0", gLightningData.triggerCount, gLightningData.refAtStrike, gLightningData.peakOfStrike, strikeDurMS, decimalUS);
      }
      delay(1); // Since we are waiting for the BulbSec timer, no need to go any faster than 1 ms
    }
    // Recheck if current value is below threshold and if so reset Reference
    gLightningData.sensorVal = CAU::analogRead(gLightningData.ppLight);
    currentDif = (int16_t) gLightningData.sensorVal - (int16_t) gLightningData.referenceSensorVal;
    if (currentDif < (int16_t) gLightningData.triggerDiffThreshold)
    {
      gLightningData.referenceSensorVal = gLightningData.sensorVal;          // Update the threshold reference base value to current value 
      gLightningData.referenceUpdateTimeMS = curTimeMS + gLightningData.updateRefPeriodMS; // reset the Ref Update clock
    }
    // If still above threshold, keep old Reference to allow a retrigger to capture more secondary flashes / pulses

  }
  // Out of PHOTO mode
  // clear camera/flash back to default (especially reset camera prefocus)
  initCameraPins();
  gLightningData.peakOfStrike = 0;  // zero out the peak before going back to MENU
  // Now back to MENU mode
}

#endif //MENUS_H

