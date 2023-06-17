#ifndef HidManager_h
#define HidManager_h

#include <NimBLECharacteristic.h>
#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include "HidKeyCodes.h"

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
} KeyReport;

class HidManager {
   public:
    HidManager();
    void setupHid(NimBLEServer *pServer);
    void sendReport(KeyReport *keys);
    size_t press(uint8_t hidKey);
    size_t release(uint8_t hidKey);

   private:
    void createDescriptor(NimBLECharacteristic *characteristic);
    void setupAdversiting();

   private:
    NimBLEHIDDevice *hid;
};

#endif