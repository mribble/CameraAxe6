#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>

#define NUM_MENUS   3  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8
#define NUM_CAM_TIMER_ELEMENTS (NUM_CAMERAS*3)

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

struct CamPin {
  hwPortPin focusPin;
  hwPortPin shutterPin;
};

struct CamTimerElement {
  uint64_t timeOffset;
  uint8_t camOffset;
  uint8_t focusSig;
  uint8_t shutterSig;
  uint8_t sequencerVal;
};

struct Context {
  // Constructor with initialization list
  Context(){}

  CAState state = CA_STATE_MENU_MODE;
  uint8_t menuId = 0;
  CtxProcTable procTable;

  CAPacketHelper packetHelper;  // Handles serial communication and simplifies packet packing/unpacking
  CAEsp8266 esp8266;

  uint8_t camTriggerRunning;
  CAPacketCamSettings camSettings[NUM_CAMERAS];
  CamPin camPins[NUM_CAMERAS];
  CamTimerElement camTimerElements[NUM_CAM_TIMER_ELEMENTS];
  uint8_t curCamElement = 0;
  CATickTimer camTimer = CATickTimer(0);
  uint8_t sequencerMask = 0;
  uint8_t curSequencerBit = 0x01;
  uint8_t intervalometerEnable = 0;
  uint64_t intervalometerStartTime = 0;
  uint64_t intervalometerIntervalTime = 0;
  uint16_t intervalometerRepeats = 0;
  uint16_t intervalometerCurRepeats = 0;
  
};

#endif // CONTEXT_H
