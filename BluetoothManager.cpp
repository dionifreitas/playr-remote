
#include "BluetoothManager.h"

static NimBLEServer *pServer;
bool isConnected = false;

BluetoothManager::BluetoothManager() {}

class ServerCallbacks : public NimBLEServerCallbacks
{
  void onConnect(NimBLEServer *pServer)
  {
    Serial.println("BluetoothManager: Client connected");
    isConnected = true;
  };

  void onDisconnect(NimBLEServer *pServer)
  {
    Serial.println("BluetoothManager: Client disconnected - start advertising");
    NimBLEDevice::startAdvertising();
    isConnected = false;
  };

  void onConnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
  {
    Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
  };

  void onDisconnect(NimBLEServer *pServer, ble_gap_conn_desc *desc)
  {
    Serial.println(NimBLEAddress(desc->peer_ota_addr).toString().c_str());
  }

  void onMTUChange(uint16_t MTU, ble_gap_conn_desc *desc)
  {
    Serial.println("BluetoothManager: MTU updated: " + String(MTU) + " for connection ID: " + String(desc->conn_handle));
  };

  uint32_t onPassKeyRequest()
  {
    Serial.println("BluetoothManager: Server Passkey Request");
    return 000000;
  };

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.println("BluetoothManager: The passkey YES/NO number: " + String(pass_key));
    return true;
  };

  void onAuthenticationComplete(ble_gap_conn_desc *desc)
  {
    if (!desc->sec_state.encrypted)
    {
      NimBLEDevice::getServer()->disconnect(desc->conn_handle);
      Serial.println("BluetoothManager: Encrypt connection failed - disconnecting client");
      return;
    }
    Serial.println("BluetoothManager: Starting BLE work!");
  };
};

NimBLEServer *BluetoothManager::getServer()
{
  return pServer;
}

void BluetoothManager::setupServer(String deviceName)
{
  Serial.println("BluetoothManager: Starting Bluetooth Manager...");
  NimBLEDevice::init(deviceName.c_str());
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(true, true, true);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());
}

// void BluetoothManager::setupAdversiting() {
//   NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
//   pAdvertising->setAppearance(HID_KEYBOARD);
//   NimBLEService* pAlexaGadgetService = pServer->createService("FE03"); // Alexa Gadged Service // FEE7
//   pAdvertising->addServiceUUID(pAlexaGadgetService->getUUID());
//   pAlexaGadgetService->start();
//   pAdvertising->addServiceUUID(hid->hidService()->getUUID());
//   pAdvertising->setScanResponse(true);
//   pAdvertising->start();
// }
