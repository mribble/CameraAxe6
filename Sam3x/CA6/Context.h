#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>

#define NUM_MENUS   3  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8
#define NUM_CAM_TIMER_ELEMENTS (NUM_CAMERAS*3)
#define NUM_SEQUENCER_BITS 8

struct CtxProcTable
{
  const char* (*funcName[NUM_MENUS])()   {NULL, MenuSound_Name, MenuTest_Name};
  void (*funcMenuInit[NUM_MENUS])()      {NULL, MenuSound_MenuInit, MenuTest_MenuInit};
  void (*funcPhotoInit[NUM_MENUS])()     {NULL, MenuSound_PhotoInit, MenuTest_PhotoInit};
  void (*funcMenuRun[NUM_MENUS])()       {NULL, MenuSound_MenuRun, MenuTest_MenuRun};
  void (*funcPhotoRun[NUM_MENUS])()      {NULL, MenuSound_PhotoRun, MenuTest_PhotoRun};
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
  uint32_t setMasks[4];
  uint32_t clearMasks[4];
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
  uint32_t seqMask[NUM_SEQUENCER_BITS][4];
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
  
};

#endif // CONTEXT_H
