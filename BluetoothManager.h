#include <NimBLEDevice.h>

#ifndef BluetoothManager_h
#define BluetoothManager_h

class BluetoothManager
{

public:
    BluetoothManager();

public:
    bool isConnected;

public:
    void setupServer(String deviceName);

public:
    NimBLEServer *getServer();
};

#endif
