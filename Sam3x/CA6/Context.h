////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dreaming Robots - Copyright 2017, 2018
//
// Context for all global data
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>
#include <CATickTimer.h>
#include "MenuData.h"

// Registers
#define REG_DWT_CTRL ((volatile uint32_t*)0xE0001000)     // Data Watchpoint and Trace Control Register
#define REG_DWT_CYCCNT ((volatile uint32_t*)0xE0001004)   // Data Watchpoint and Trace Cycle Count Register
#define REG_SCB_DEMCR ((volatile uint32_t*)0xE000EDFC)    // debug exception and monitor control
#define CLOCK_TICKS (*REG_DWT_CYCCNT)
#define TICKS_PER_MICROSECOND 84

#define NUM_MENUS   9  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8
#define NUM_CAM_TIMER_ELEMENTS (NUM_CAMERAS*3)
#define NUM_SEQUENCER_BITS 8

struct CtxProcTable {
  const char* (*funcName[NUM_MENUS])()   {NULL, dev_Name, sound_Name, light_Name, vibration_Name, valve_Name,
                                                lightning_Name, projectile_Name, beam_Name};
  void (*funcMenuInit[NUM_MENUS])()      {NULL, dev_MenuInit, sound_MenuInit, light_MenuInit, vibration_MenuInit, valve_MenuInit,
                                                lightning_MenuInit, projectile_MenuInit, beam_MenuInit};
  void (*funcPhotoInit[NUM_MENUS])()     {NULL, dev_PhotoInit, sound_PhotoInit, light_PhotoInit, vibration_PhotoInit, valve_PhotoInit,
                                                lightning_PhotoInit, projectile_PhotoInit, beam_PhotoInit};
  void (*funcMenuRun[NUM_MENUS])()       {NULL, dev_MenuRun, sound_MenuRun, light_MenuRun, vibration_MenuRun, valve_MenuRun,
                                                lightning_MenuRun, projectile_MenuRun, beam_MenuRun};
  void (*funcPhotoRun[NUM_MENUS])()      {NULL, dev_PhotoRun, sound_PhotoRun, light_PhotoRun, vibration_PhotoRun, valve_PhotoRun,
                                                lightning_PhotoRun, projectile_PhotoRun, beam_PhotoRun};
};

enum CAState {
  CA_STATE_MENU_MODE,
  CA_STATE_PHOTO_MODE,
};

struct CamElement0 {
  uint64_t timeOffset;
  uint8_t camOffset;
  uint8_t focusSig;
  uint8_t shutterSig;
  uint8_t sequencerVal;
};

struct CamElement1 {
  uint64_t timeOffset;
  uint32_t setMask;
  uint32_t clearMask;
};

struct Context {
  // Constructor with initialization list
  Context(){}

  CAState state = CA_STATE_MENU_MODE;
  uint8_t menuId = 0;
  CtxProcTable procTable;

  CAPacketHelper packetHelper;  // Handles serial communication and simplifies packet packing/unpacking
  CAEsp8266 esp8266;

  CAPacketCamSettings camSettings[NUM_CAMERAS];

  CamElement1 camElements[NUM_CAM_TIMER_ELEMENTS];
  uint8_t numCamElements;
  uint8_t curCamElement;
  uint32_t seqMask[NUM_SEQUENCER_BITS];
  uint8_t sequencerValue;
  
  uint8_t camTriggerRunning;
  CATickTimer camTimer = CATickTimer(0);
  uint8_t curSeqMask = 0;
  uint8_t curSeqBit = 0x01;
  uint8_t intervalometerEnable = 0;
  uint64_t intervalometerStartTime = 0;
  uint64_t intervalometerIntervalTime = 0;
  uint16_t intervalometerRepeats = 0;
  uint16_t intervalometerCurRepeats = 0;

 CATickTimer menuTimer = CATickTimer(1);
 uint8_t prevToggleCamVal = LOW;
  
};

extern Context g_ctx;
#endif // CONTEXT_H
