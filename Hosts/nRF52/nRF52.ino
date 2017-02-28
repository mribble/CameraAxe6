
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

uint8_t gRxBuf[2048];
uint16_t gRxSize = 0;
uint8_t gTxBuf[2048];

void bleReceived(BLECentral& /*central*/, BLECharacteristic& rxCharacteristic) {
  const uint8_t *data = rxCharacteristic.value();
  uint16_t sz = rxCharacteristic.valueLength();
  for (uint16_t i = 0; i < sz; i++) {
    gRxBuf[gRxSize++] = data[i];
  }

  if (gRxSize > 0) {
    Serial.write(gRxBuf, gRxSize);
    gRxSize = 0;
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

void loop() {
  gBLE.poll();

  uint16_t serialSize = Serial.available();

  if (serialSize > 0) {
    Serial.readBytes(gTxBuf, serialSize);
    bleWrite(gTxBuf, serialSize);
  }
}


