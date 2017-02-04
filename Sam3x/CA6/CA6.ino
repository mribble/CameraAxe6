#include <CATypes.h>
#include <CAUtility.h>
#include <CAEeprom.h>
#include <CAPacket.h>
#include <CAPacketHelper.h>
#include <CATickTimer.h>
#include <CAEsp8266.h>
#include "MenuData.h"
#include "Context.h"

// Initialize the context
Context g_ctx;

void setup() {
  // This could be bigger, just want to know if the size starts getting really large
  CA_ASSERT(sizeof(ModStore) < 1024, "modStore is getting large.");

  CAU::logInit(9600);
  CAU::initializeAnalog();

  g_ctx.esp8266.init();
  g_ctx.packetHelper.init(74880);
}

void loop() {

  processTerminalCmds();
  caRunTests();
  processIncomingPacket();
//  sendMenuPackets();

  if (g_ctx.active) {
    if (interModuleLogicArbiter()) {  // True means we need to trigger cameras and flashes
      cameraFlashHandler();
    }
  }
  else {
    //checkModulePorts();
    delay(100);
  }
}

// This sends the packets from each menu
void sendMenuPackets() {
  uint8 i;
  
  for (i=0; i<NUM_MODULES; ++i) {
    uint8 modId = g_ctx.modules[i].modId;
    if (modId) {
      g_ctx.procTable.funcSendPackets[modId](i);
    }
  }
  if (g_ctx.fakeModule) {
    g_ctx.procTable.funcSendPackets[g_ctx.fakeModule](0);
  }
}

// This is the work that happens switching from menu mode to photo mode.
void photoModeSetup() {
  uint8 i;

  for(i=0; i<NUM_MODULES; ++i) {
    uint8 modId = g_ctx.modules[i].modId;
    if (modId) {
      g_ctx.procTable.funcActiveInit[modId](i);
    }
  }
}

void checkModulePorts() {
  uint8 i;
  for(i=0; i<NUM_MODULES; ++i) {
    uint8 val;
    CAEeprom moduleEeprom(unioDevice(CA_MODULE0+i));
    if (moduleEeprom.read(&val, 0x10, 1)) {
      CA_ASSERT(val, "Value should never be zero.");  // Zero is reserved for unconnected modules
      if (g_ctx.modules[i].modId != val) {
        g_ctx.modules[i].modId = val;
        // todo send update to host about changing modules
      }
    }
    else {
      if (g_ctx.modules[i].modId != 0) {
        // Module has been unplugged
        g_ctx.modules[i].modId = 0;
        // todo send update to host about changing modules
      }
    }
  }
}

// This handles collecting all the module results and seeing if a full trigger happened
uint8 interModuleLogicArbiter() {
  uint8 i, ret;
  uint8 trig[NUM_MODULES] = {0,0,0,0};

  if (g_ctx.interModuleLogic.getLatchEnable()) {
    for(i=0; i<NUM_MODULES; ++i) {
      int8 modId = g_ctx.modules[i].modId;
      if (modId) {
        g_ctx.modules[i].latchedTriggers |= g_ctx.procTable.funcTriggerCheck[modId](i);
        trig[i] = g_ctx.modules[i].latchedTriggers;
      }
      else {
        trig[i] = 1;
      }
    }
  }
  else {
    for (i=0; i<NUM_MODULES; ++i) {
      uint8 modId = g_ctx.modules[i].modId;
      if(modId) {
        trig[i] = g_ctx.procTable.funcTriggerCheck[modId](i);
      }
      else {
        trig[i] = 1;
      }
    }
  }

  CA_ASSERT(NUM_MODULES == 4, "Code below assumes 4 modules");
  switch (g_ctx.interModuleLogic.getLogic()) {
    case 0:  // or
      ret = (trig[0] || trig[1] || trig[2] || trig[3]);
      break;
    case 1:   // and
      ret = (trig[0] && trig[1] && trig[2] && trig[3]);
      break;
    case 2:   // (module0 || module1) && (module2 || module 3)
      ret = (trig[0] || trig[1]) && (trig[2] || trig[3]);
      break;
    case 3:   // (module0 && module1) || (module2 && module3)
      ret = (trig[0] || trig[1]) && (trig[2] || trig[3]);
      break;
    default:
      CA_ASSERT(0, "invalid interModuleLogic");
      break;
  }

  if (ret == CA_TRUE && g_ctx.interModuleLogic.getLatchEnable()) {
    for(i=0; i<NUM_MODULES; ++i) {
      g_ctx.modules[i].latchedTriggers = 0;
    }
  }

  return ret;
}


void cameraFlashHandler() {
  hwPortPin ppFocus, ppShutter;
  uint8 i;

  // todo -- For now just trigger all the cameras for 1 second

  for(i=0; i<8; ++i)
  {
    ppFocus = CAU::getCameraPin(i, FOCUS);
    CAU::pinMode(ppFocus, OUTPUT);
    CAU::digitalWrite(ppFocus, HIGH);
    ppShutter = CAU::getCameraPin(i, SHUTTER);
    CAU::pinMode(ppShutter, OUTPUT);
    CAU::digitalWrite(ppShutter, HIGH);
  }
  delay(1000);

  for(i=0; i<8; ++i)
  {
    ppFocus = CAU::getCameraPin(i, FOCUS);
    CAU::digitalWrite(ppFocus, LOW);
    ppShutter = CAU::getCameraPin(i, SHUTTER);
    CAU::digitalWrite(ppShutter, LOW);
  }

}



/*
//#define ENABLE_CUSTOM_UART
#ifdef ENABLE_CUSTOM_UART
// must update C:\Users\Moe\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.6\variants\arduino_due_x/variant.cpp
// to add -- void USART1_Handler(void) __attribute__((weak));
void USART1_Handler(void) {

  uint32_t status = _pUart->UART_SR;

  // Did we receive data?
  if ((status & UART_SR_RXRDY) == UART_SR_RXRDY)
    _rx_buffer->store_char(_pUart->UART_RHR);

  // Do we need to keep sending data?
  if ((status & UART_SR_TXRDY) == UART_SR_TXRDY) 
  {
    if (_tx_buffer->_iTail != _tx_buffer->_iHead) {
      _pUart->UART_THR = _tx_buffer->_aucBuffer[_tx_buffer->_iTail];
      _tx_buffer->_iTail = (unsigned int)(_tx_buffer->_iTail + 1) % SERIAL_BUFFER_SIZE;
    }
    else
    {
      // Mask off transmit interrupt so we don't get it anymore
      _pUart->UART_IDR = UART_IDR_TXRDY;
    }
  }

  // Acknowledge errors
  if ((status & UART_SR_OVRE) == UART_SR_OVRE || (status & UART_SR_FRAME) == UART_SR_FRAME)
  {
    // TODO: error reporting outside ISR
    _pUart->UART_CR |= UART_CR_RSTSTA;

}
#endif
*/

