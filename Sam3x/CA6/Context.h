#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>

#define NUM_MENUS   3  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8
#define NUM_CAM_TIMER_ELEMENTS (NUM_CAMERAS*5)

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
  CamPin camPins[NUM_CAMERAS];
  CamTimerElement camTimerElements[NUM_CAM_TIMER_ELEMENTS];
  
};

#endif // CONTEXT_H
