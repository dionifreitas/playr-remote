#include "BluetoothManager.h"
#include "HidKeyCodes.h"
#include "HidManager.h"
#include "Util.h"

#define LED 2

#define PRESS 'D'    // DOWN KEY
#define RELEASE 'U'  // UP KEY
#define WRITE 'P'    // DOWN and UP key
#define SEPARATOR ';'
#define END '\n'

BluetoothManager bluetoothManager;
HidManager hidManager;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  Serial.println("Starting System...");
  bluetoothManager.setupServer("Remote Control");
  hidManager.setupHid(bluetoothManager.getServer());
  Serial.println("Started Playr Server!");
}

String readSerialIfAvailable() {
  if (Serial.available()) {
    return Serial.readStringUntil(END);
  }
  return "";
}

void handleKeyEvent(char command, String value) {
  int key = getHidKeyCode(value.toInt());
  if (key == 0) {
    Serial.println("E:Invalid input event code: '" + value + "'. See all available codes at: https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h");
  }
  switch (command) {
    case PRESS: hidManager.press(key); break;
    case RELEASE: hidManager.release(key); break;
    case WRITE: hidManager.write(key); break;
    default:
      Serial.print("E:Invalid command: '");
      Serial.print(command);
      Serial.println("'");
      return;
  }
}

void handleMessages() {
  String message = trim(readSerialIfAvailable());
  if (message.length() > 0) {
    digitalWrite(LED, HIGH);
    char command = splitString(message, SEPARATOR, 0).charAt(0);
    String value = splitString(message, SEPARATOR, 1);

    handleKeyEvent(command, value);
    digitalWrite(LED, LOW);
  } else {
    Serial.println("E:Invalid input command");
  }
}

void loop() {

  handleMessages();
}

// KeyReport getKeyReportFromKey(char key) {
//   KeyReport keyReport;
//   memset(&keyReport, 0, sizeof(KeyReport));

//   getHid

//   if (key >= 'a' && key <= 'z') {
//     keyReport.keys[0] = key - 'a' + 4; // Letras minúsculas começam em 4
//   } else if (key >= 'A' && key <= 'Z') {
//     keyReport.modifiers |= 0x02; // Definir o bit 1 do modificador como 1
//     (SHIFT pressionado) keyReport.keys[0] = key - 'A' + 4; // Letras
//     maiúsculas também começam em 4
//   } else if (key >= '0' && key <= '9') {
//     keyReport.keys[0] = key - '0' + 30; // Números começam em 30
//   }

//   return keyReport;
// }

// #include <NimBLEDevice.h>
// #include <NimBLEHIDDevice.h>
// #include <NimBLECharacteristic.h>
// #include <Preferences.h>
// #include "Keys.h"
// #include "HidDescriptor.h"
// #include "HidKeyCodes.h"

// #define LED 2

// const char ON_KEY_DOWN = 'D';
// const char ON_KEY_UP = 'U';
// const char ON_KEY_PRESSED = 'P';

// const char CLIENT_CONNECTED = 'C';
// const char CLIENT_DISCONNECTED = 'O';
// const char SET_INSTANCE = 'S';
// const char LOG = 'I';
// const char DELIMITER = ';';
// const char DELETE_ALL_PAIRED = 'R';
// const char RAW_PRESS = 'A';
// const char RAW_RELEASE = 'B';

// typedef uint8_t MediaKeyReport[2];
// MediaKeyReport _mediaKeyReport;
// typedef uint8_t SystemKeyReport[2];
// SystemKeyReport _systemKeyReport;

// String message = "";

// KeyReport _keyReport;

// static NimBLEServer* pServer;
//
// NimBLECharacteristic* inputKeyboard;
// NimBLECharacteristic* outputKeyboard;
// NimBLECharacteristic* inputMediaKeys;
// NimBLECharacteristic* inputSystemKeys;
// Preferences preferences;

// bool isConnected = false;
// uint8_t macAddr[6];

// String Command(char identifier, String strData) {
//   return String(identifier) + DELIMITER + strData;
// }

// void startServer(String deviceName) {
//   Serial.println(Command(LOG, "Starting Playr Server..."));
//   NimBLEDevice::init(deviceName.c_str());
//   NimBLEDevice::setPower(ESP_PWR_LVL_P9);
//   NimBLEDevice::setSecurityAuth(true, true, true);
//   pServer = NimBLEDevice::createServer();
//   pServer->setCallbacks(new ServerCallbacks());

//   setupHid();
//   setupAdversiting();

//   hid->setBatteryLevel(100);

//   Serial.println(Command(LOG, "Started Playr Server!"));
// }

// int getInstance() {
//   preferences.begin("PLAYR", true);
//   unsigned int instanceNumber = preferences.getUInt("INST", 0);
//   preferences.end();
//   return instanceNumber;
// }

// void setupInstance(int instance) {
//   if (instance >= 0 && instance < CONFIG_BT_NIMBLE_MAX_CONNECTIONS) {
//     preferences.begin("PLAYR", false);
//     preferences.putUInt("INST", instance);
//     preferences.end();
//   }
// }

// void setupMacAddr(int instance) {
//   uint8_t macAddr[6];
//   esp_read_mac(macAddr, ESP_MAC_BT);

//   uint8_t newSuffix = (macAddr[5] + instance) % 256;
//   uint8_t newMac[6] = {macAddr[0], macAddr[1], macAddr[2], macAddr[3],
//   macAddr[4], newSuffix}; esp_base_mac_addr_set(newMac);

//   Serial.printf("Default Mac %02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0],
//   macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
//   Serial.printf("New Mac %02X:%02X:%02X:%02X:%02X:%02X\n", newMac[0],
//   newMac[1], newMac[2], newMac[3], newMac[4], newMac[5]);
// }

// String getValue(String data, char separator, int index)
// {
//   int found = 0;
//   int strIndex[] = { 0, -1 };
//   int maxIndex = data.length() - 1;

//   for (int i = 0; i <= maxIndex && found <= index; i++) {
//     if (data.charAt(i) == separator || i == maxIndex) {
//       found++;
//       strIndex[0] = strIndex[1] + 1;
//       strIndex[1] = (i == maxIndex) ? i + 1 : i;
//     }
//   }
//   return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

// void handleMessage() {

//   // char command = getValue(message, DELIMITER, 0).charAt(0);
//   // String value = getValue(message, DELIMITER, 1);
//   // if (command == ON_KEY_DOWN || command == ON_KEY_UP || command ==
//   ON_KEY_PRESSED) {
//   //   handleKeyEvent(command, value);
//   // } else if (command == SET_INSTANCE) {
//   //   handleServerInstance(value);
//   // } else if (command == RAW_PRESS || command == RAW_RELEASE) {
//   //   handleRawEvent(command, value);
//   // }
// }

// void handleServerInstance(String value) {
//   int instance = value.toInt();
//   if (instance >= 0 && instance < CONFIG_BT_NIMBLE_MAX_CONNECTIONS) {
//     setupInstance(instance);
//     NimBLEDevice::deinit(true);
//     ESP.restart();
//   }
// }

// void handleRawEvent(char command, String value) {
//    const keyReport = ""

//    switch (command) {
//      case RAW_PRESS: press(keyReport); break;
//      case RAW_RELEASE: release(keyReport); break;
//    }
// }



//   if (key == KEY_POWER || key == KEY_SLEEP) {
//     switch (command) {
//       case ON_KEY_DOWN: pressSystemKey(key); break;
//       case ON_KEY_UP: releaseSystemKey(key); break;
//       case ON_KEY_PRESSED: writeSystemKey(key); break;
//       default: return;
//     }
//     return;
//   }

//   switch (command) {
//     case ON_KEY_DOWN: press(key); break;
//     case ON_KEY_UP: release(key); break;
//     case ON_KEY_PRESSED: write(key); break;
//     default: return;
//   }
// }

// int ledState = LOW;
// unsigned long previousMillis = 0;

// void setup() {
//   pinMode(LED, OUTPUT);
//   Serial.begin(115200);
//   Serial.println(Command(LOG, "Starting Playr Server..."));
//   int instance = getInstance();
//   setupMacAddr(instance);
//   startServer(instance == 0 ? "Playr Remote" : "Playr Remote #" +
//   String(instance));
// }

// void loop() {
//   if (Serial.available()) {
//     digitalWrite(LED, HIGH);
//     message = Serial.readStringUntil('\n');
//     message = Serial.read
//     handleMessage();
//     digitalWrite(LED, LOW);
//   }

//   if (!isConnected) {
//     unsigned long currentMillis = millis();
//     if (currentMillis - previousMillis >= 500) {
//       ledState = (ledState == LOW) ? HIGH : LOW;
//       digitalWrite(LED, ledState);
//       previousMillis = currentMillis;
//     }
//   } else if (ledState == HIGH) {
//     digitalWrite(LED, LOW);
//   }
// }





//   sendReport(&_keyReport);
//   return 1;
// }







// size_t writeSystemKey(uint8_t systemKey)
// {
//   uint8_t p = pressSystemKey(systemKey);
//   releaseSystemKey(systemKey);
//   return p;
// }

// size_t pressSystemKey(uint8_t systemKey)
// {
//   switch (systemKey) {
//     case KEY_POWER:
//       _systemKeyReport[0] = 0x01;
//       _systemKeyReport[1] = 0x00;
//       break;
//     case KEY_SLEEP:
//       _systemKeyReport[0] = 0x02;
//       _systemKeyReport[1] = 0x00;
//       break;
//   }
//   sendSystemReport(&_systemKeyReport);
//   return 1;
// }

// size_t releaseSystemKey(uint8_t systemKey)
// {
//   _systemKeyReport[0] = 0x00;
//   _systemKeyReport[1] = 0x00;
//   sendSystemReport(&_systemKeyReport);
//   return 1;
// }

// void sendReport(KeyReport* keys)
// {
//   inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
//   inputKeyboard->notify();
// }



// void sendSystemReport(SystemKeyReport* keys)
// {
//   inputSystemKeys->setValue((uint8_t*)keys, sizeof(SystemKeyReport));
//   inputSystemKeys->notify();
// }
