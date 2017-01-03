#ifndef CONTEXT_H
#define CONTEXT_H

#define NUM_MENUS   2  /* Includes 1 for the null menu */
#define NUM_MODULES 4
#define NUM_CAMERAS 8

typedef struct
{
  uint8 modId = 0;
  uint8 latchedTriggers = 0;
  ModStore modStore;
} CtxModules;

typedef struct
{
  void (*funcInit[NUM_MENUS])(uint8)                   {NULL, MenuSoundInit};
  void (*funcSendPackets[NUM_MENUS])(uint8)            {NULL, MenuSoundSendPackets};
  void (*funcReceivePackets[NUM_MENUS])(uint8, uint8*) {NULL, MenuSoundReceivePackets};
  void (*funcActiveInit[NUM_MENUS])(uint8)             {NULL, MenuSoundActiveInit};
  uint8 (*funcTriggerCheck[NUM_MENUS])(uint8)          {NULL, MenuSoundTriggerCheck};
} CtxProcTable;

typedef struct {
  uint8 active = 0;
//  PacketInterModuleLogic interModuleLogic;
  CtxModules modules[NUM_MODULES];
//  PacketCamSettings camSettings[NUM_CAMERAS];

  CASerial bleSerial;  // Serial for talking to BLE module
  
  CtxProcTable procTable;

} Context;

#endif // CONTEXT_H
