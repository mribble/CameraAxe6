#ifndef CONTEXT_H
#define CONTEXT_H

#include <CAPacket.h>

#define NUM_MENUS   3  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8

struct CtxModules
{
  uint8 modId = 0;
  uint8 latchedTriggers = 0;
  ModStore modStore;
};

struct CtxProcTable
{
  void (*funcInit[NUM_MENUS])(uint8)                   {NULL, MenuSoundInit, MenuTestInit};
  void (*funcSendPackets[NUM_MENUS])(uint8)            {NULL, MenuSoundSendPackets, MenuTestSendPackets};
  void (*funcReceivePackets[NUM_MENUS])(uint8, uint8*) {NULL, MenuSoundReceivePackets, MenuTestReceivePackets};
  void (*funcActiveInit[NUM_MENUS])(uint8)             {NULL, MenuSoundActiveInit, MenuTestActiveInit};
  uint8 (*funcTriggerCheck[NUM_MENUS])(uint8)          {NULL, MenuSoundTriggerCheck, MenuTestTriggerCheck};
};

struct Context {
  // Constructor with initialization list
  Context(){}

  uint8 active = 0;
  CAPacketInterModuleLogicBase interModuleLogic;
  CAPacketCamSettingsBase camSettings[NUM_CAMERAS];

  CtxModules modules[NUM_MODULES];
  CAPacketHelper packetHelper;  // Handles serial communication and simplifies packet packing/unpacking
  CtxProcTable procTable;

// todo remove
//  PacketInterModuleLogic interModuleLogic;
//  PacketCamSettings camSettings[NUM_CAMERAS];
};

#endif // CONTEXT_H
