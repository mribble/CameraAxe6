#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>

#define NUM_MENUS   3  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8

struct CtxModules
{
  uint8 modId = 0;
};

struct CtxProcTable
{
  void (*funcInfo[NUM_MENUS])(MenuData*) {NULL, MenuSound_Info, MenuTest_Info};
  void (*funcMenuInit[NUM_MENUS])()      {NULL, MenuSound_MenuInit, MenuTest_MenuInit};
  void (*funcPhotoInit[NUM_MENUS])()     {NULL, MenuSound_PhotoInit, MenuTest_PhotoInit};
  void (*funcMenuRun[NUM_MENUS])()       {NULL, MenuSound_MenuRun, MenuTest_MenuRun};
  void (*funcPhotoRun[NUM_MENUS])()      {NULL, MenuSound_PhotoRun, MenuTest_PhotoRun};
};

struct Context {
  // Constructor with initialization list
  Context(){}

  uint8 active = 0;
  uint8 menuId = 0;
  CAPacketCamSettingsBase camSettings[NUM_CAMERAS];

  CtxModules modules[NUM_MODULES];
  CAPacketHelper packetHelper;  // Handles serial communication and simplifies packet packing/unpacking
  CtxProcTable procTable;

  CAEsp8266 esp8266;
};

#endif // CONTEXT_H
