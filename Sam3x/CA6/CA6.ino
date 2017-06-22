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
  SerialIO.begin(9600);
  CAU::initializeAnalog();

  g_ctx.esp8266.init(4800);
  hwPortPin rts = CAU::getModulePin(0,0);
  hwPortPin cts = CAU::getModulePin(0,1);
  g_ctx.packetHelper.init(g_ctx.esp8266.getSerial());
}

void loop() {
  caRunTests();
  processTerminalCmds();
  checkModulePorts();

  if (g_ctx.state == CA_STATE_MENU_MODE) {
    g_ctx.procTable.funcMenuRun[g_ctx.menuId]();
    delay(100);
  } else if (g_ctx.state == CA_STATE_PHOTO_MODE) {
    g_ctx.procTable.funcPhotoRun[g_ctx.menuId]();
  } else {
    // Menus hanlde processing packets so only do this here if no menus are running
    CAPacketElement *packet = processIncomingPacket();
    if (packet) {
      delete packet;
    }
  }
}

void checkModulePorts() {
  for(uint8_t i=0; i<NUM_MODULES; ++i) {
    uint8_t val = 0;
    CAEeprom moduleEeprom(unioDevice(CA_MODULE0+i));
    if (moduleEeprom.readModuleId(&val)) {
      if (g_ctx.modules[i].modId != val) {
        g_ctx.modules[i].modId = val;
        // todo send update to host about changing modules
        CA_INFO("Module detected", val);
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

void triggerCameras() {
  hwPortPin ppFocus, ppShutter;
  uint8_t i;

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

