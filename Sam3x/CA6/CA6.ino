#include <CATypes.h>
#include <CAUtility.h>
#include <CAEeprom.h>
#include <CASerial.h>
#include <CAPacket.h>
#include <CATickTimer.h>
#include "MenuData.h"
#include "Context.h"

// Initialize the context
Context g_ctx;

//#define ENABLE_CUSTOM_UART
#ifdef ENABLE_CUSTOM_UART
// must update C:\Users\Moe\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.6\variants\arduino_due_x/variant.cpp
// to add -- void USART1_Handler(void) __attribute__((weak));
void USART1_Handler(void)
{
/*
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
 */
}
#endif

/*void isolateRFD()
{
  hwPortPin pp;
  pp = CAU::getOnboardDevicePin(BLE_TX_PIN);
  CAU::pinMode(pp, INPUT);
  pp = CAU::getOnboardDevicePin(BLE_RX_PIN);
  CAU::pinMode(pp, INPUT);
}*/

void setup()
{
  // This could be bigger, just want to know if the size starts getting really large
  CA_ASSERT(sizeof(ModStore) < 1024, "modStore is getting large.");

  CAU::logInit(9600);
  CAU::initializeAnalog();

  g_ctx.bleSerial.init(9600);

  //delay(5000); //Give time to connect USB serial
}

void loop()
{
  caRunTests();
/*
  if (g_ctx.active)
  {
    if (interModuleLogicArbiter())  // True means we need to trigger cameras and flashes
    {
      cameraFlashHandler();
    }
  }
  else
  {
    checkModulePorts();
    delay(100);
  }
*/
}

// This is the work that happens switching from menu mode to photo mode.
void photoModeSetup()
{
  uint8 i;

  for(i=0; i<NUM_MODULES; ++i)
  {
    uint8 modId = g_ctx.modules[i].modId;
    if (modId)
    {
      g_ctx.procTable.funcActiveInit[modId](i);
    }
  }
}

void checkModulePorts()
{
  uint8 i;
  for(i=0; i<NUM_MODULES; ++i)
  {
    uint8 val;
    CAEeprom moduleEeprom(unioDevice(CA_MODULE0+i));
    if (moduleEeprom.read(&val, 0x10, 1))
    {
      CA_ASSERT(val, "Value should never be zero.");  // Zero is reserved for unconnected modules
      if (g_ctx.modules[i].modId != val)
      {
        g_ctx.modules[i].modId = val;
        // todo send update to host about changing modules
      }
    }
    else
    {
      if (g_ctx.modules[i].modId != 0)
      {
        // Module has been unplugged
        g_ctx.modules[i].modId = 0;
        // todo send update to host about changing modules
      }
    }
  }
}

// This handles collecting all the module results and seeing if a full trigger happened
uint8 interModuleLogicArbiter()
{
  uint8 i, ret;
  uint8 trig[NUM_MODULES] = {0,0,0,0};

/*  if (g_ctx.interModuleLogic.enable_latch)
  {
    for(i=0; i<NUM_MODULES; ++i)
    {
      int8 modId = g_ctx.modules[i].modId;
      if (modId)
      {
        g_ctx.modules[i].latchedTriggers |= g_ctx.procTable.funcTriggerCheck[modId](i);
        trig[i] = g_ctx.modules[i].latchedTriggers;
      }
      else
      {
        trig[i] = 1;
      }
    }
  }
  else
  {
    for (i=0; i<NUM_MODULES; ++i)
    {
      uint8 modId = g_ctx.modules[i].modId;
      if(modId)
      {
        trig[i] = g_ctx.procTable.funcTriggerCheck[modId](i);
      }
      else
      {
        trig[i] = 1;
      }
    }
  }
  */
/*
  CA_ASSERT(NUM_MODULES == 4, "Code below assumes 4 modules");
  switch (g_ctx.interModuleLogic.logic)
  {
    case IML_OR:
      ret = (trig[0] || trig[1] || trig[2] || trig[3]);
      break;
    case IML_AND:
      ret = (trig[0] && trig[1] && trig[2] && trig[3]);
      break;
    case IML_OR_AND_OR:
      ret = (trig[0] || trig[1]) && (trig[2] || trig[3]);
      break;
    case IML_AND_OR_AND:
      ret = (trig[0] || trig[1]) && (trig[2] || trig[3]);
      break;
    default:
      CA_ASSERT(0, "invalid interModuleLogic");
      break;
  }
  */
/*  if (ret == CA_TRUE && g_ctx.interModuleLogic.enable_latch)
  {
    for(i=0; i<NUM_MODULES; ++i)
    {
      g_ctx.modules[i].latchedTriggers = 0;
    }
  }
*/  
  return ret;
}


void cameraFlashHandler()
{
  
}


