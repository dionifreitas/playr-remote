
#include "HidManager.h"

#include "HidDescriptor.h"

NimBLEHIDDevice *hid;
NimBLECharacteristic *inputKeyboard;
NimBLECharacteristic *outputKeyboard;
NimBLECharacteristic *inputMediaKeys;
NimBLECharacteristic *inputSystemKeys;

uint16_t vid = 0x05ac;
uint16_t pid = 0x820a;
uint16_t version = 0x0210;

KeyReport _keyReport;

HidManager::HidManager() {}

void HidManager::createDescriptor(NimBLECharacteristic *characteristic) {
    NimBLE2904 *p2904characteristic = (NimBLE2904 *)characteristic->createDescriptor("2904");
    p2904characteristic->setFormat(NimBLE2904::FORMAT_UTF8);
}

void HidManager::setupAdversiting() {
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setAppearance(HID_KEYBOARD);

    // NimBLEService* pAlexaGadgetService = pServer->createService("FE03"); // Alexa Gadged Service // FEE7
    // pAdvertising->addServiceUUID(pAlexaGadgetService->getUUID());
    // pAlexaGadgetService->start();

    pAdvertising->addServiceUUID(hid->hidService()->getUUID());
    pAdvertising->setScanResponse(true);
    pAdvertising->start();
}

void HidManager::setupHid(NimBLEServer *pServer) {
    hid = new NimBLEHIDDevice(pServer);
    inputKeyboard = hid->inputReport(KEYBOARD_ID);  // <-- input REPORTID from report map
    outputKeyboard = hid->outputReport(KEYBOARD_ID);
    inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);
    inputSystemKeys = hid->inputReport(SYSTEM_CONTROL_ID);
    hid->manufacturer()->setValue("Playr");
    hid->pnp(0x02, vid, pid, version);
    hid->hidInfo(0x00, 0x01);
    hid->reportMap((uint8_t *)_hidReportDescriptor, sizeof(_hidReportDescriptor));
    hid->startServices();
    hid->setBatteryLevel(100);

    this->createDescriptor(inputKeyboard);
    this->createDescriptor(outputKeyboard);
    this->createDescriptor(inputMediaKeys);
    this->createDescriptor(inputSystemKeys);
    this->setupAdversiting();
}

void HidManager::sendReport(KeyReport *keys) {
    inputKeyboard->setValue((uint8_t *)keys, sizeof(KeyReport));
    inputKeyboard->notify();
}

size_t press(uint8_t hidKey) {
    int modifier = getModifier(hidKey);
    if (modifier != -1) {
        _keyReport.modifiers |= modifier;
    }

    for (uint8_t i = 0; i < 6; i++) {
        if (_keyReport.keys[i] == 0x00) {
            _keyReport.keys[i] = hidKey;
            this->sendReport(&_keyReport);
            return 1;
        } else if (_keyReport.keys[i] == hidKey) {
            return 0;
        }
    }

    return 0;
}

size_t release(uint8_t hidKey) {
    int modifier = getModifier(hidKey);
    if (modifier != -1) {
        _keyReport.modifiers &= ~modifier;
    }

    for (uint8_t i = 0; i < 6; i++) {
        if (_keyReport.keys[i] == hidKey) {
            _keyReport.keys[i] = 0x00;
        }
    }

    this->sendReport(&_keyReport);
    return 1;
}