
/*
 * Create a uart service compatible with Nordic's *nRF Toolbox*.
 */

#include <BLEPeripheral.h>

// define pins (-1 means unused) for nRF52 boards
#define BLE_REQ_PIN   -1
#define BLE_RDY_PIN   -1
#define BLE_RST_PIN   -1

BLEPeripheral gBLE = BLEPeripheral (BLE_REQ_PIN, BLE_RDY_PIN, BLE_RST_PIN);
BLEService gUartService = BLEService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLEDescriptor gUartNameDescriptor = BLEDescriptor("2901", "UART");
BLECharacteristic gRxCharacteristic = BLECharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWriteWithoutResponse, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
BLEDescriptor gRxNameDescriptor = BLEDescriptor("2901", "RX - Receive Data (Write)");
BLECharacteristic gTxCharacteristic = BLECharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, BLE_ATTRIBUTE_MAX_VALUE_LENGTH);
BLEDescriptor gTxNameDescriptor = BLEDescriptor("2901", "TX - Transfer Data (Notify)");

// Just pass through data from BLE to serial
void bleReceived(BLECentral& /*central*/, BLECharacteristic& rxCharacteristic) {
  const uint8_t *data = rxCharacteristic.value();
  uint16_t sz = rxCharacteristic.valueLength();

  if (sz > 0) {
    Serial.write(data, sz);
  }
}

void setup() {
  Serial.begin(74880);
    
  gBLE.setLocalName("CameraAxe6");
  gBLE.addAttribute(gUartService);
  gBLE.addAttribute(gUartNameDescriptor);
  gBLE.setAdvertisedServiceUuid(gUartService.uuid());
  gBLE.addAttribute(gRxCharacteristic);
  gBLE.addAttribute(gRxNameDescriptor);
  gRxCharacteristic.setEventHandler(BLEWritten, bleReceived);
  gBLE.addAttribute(gTxCharacteristic);
  gBLE.addAttribute(gTxNameDescriptor);
  gBLE.begin();
}

void bleWrite(uint8_t *data, uint16_t sz) {
  // BLE can only send 20 bytes at a time so break it up.
  while (sz) {
    if (sz > 20) {
      gTxCharacteristic.setValue(data, 20);
      data+=20;
      sz-=20;
    } else {
      gTxCharacteristic.setValue(data, sz);
      sz = 0;
    }
  }
}

#define PACKET_SIZE_SIZE 2

uint16_t genPacketSize(uint8_t b0, uint8_t b1) {
  uint16_t ret = uint16_t(b0) + (uint16_t(b1)<<8);
  return ret;
}

uint8_t getPacketSize(uint16_t val, uint8_t byteNumber) {
  if (byteNumber == 0) {
    return uint8_t(val & 0xFF);
  } else {
    return val >> 8;
  }
}

void loop() {
  gBLE.poll();

  static uint16_t gPacketSize = 0;
  uint16_t serialSize = Serial.available();
  uint8_t buf[2048];

  if (serialSize > 0) {
    
    if (gPacketSize == 0 && serialSize >= PACKET_SIZE_SIZE) {
      uint8_t ibuf[PACKET_SIZE_SIZE];
      Serial.readBytes(ibuf, PACKET_SIZE_SIZE);
      gPacketSize = genPacketSize(ibuf[0], ibuf[1]);
    }
    if (gPacketSize != 0 && serialSize >= gPacketSize-PACKET_SIZE_SIZE) {
      buf[0] = getPacketSize(gPacketSize, 0);
      buf[1] = getPacketSize(gPacketSize, 1);
      Serial.readBytes(buf+PACKET_SIZE_SIZE, gPacketSize-PACKET_SIZE_SIZE);
      bleWrite(buf, gPacketSize);
      gPacketSize = 0;      
    }
  }
}


