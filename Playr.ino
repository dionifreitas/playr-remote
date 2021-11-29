#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Preferences.h>

#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02
#define LED 2

const char KEY_DOWN = 'D';
const char KEY_UP = 'U';
const char KEY_PRESSED = 'P';
const char CLIENT_CONNECTED = 'C';
const char CLIENT_DISCONNECTED = 'O';
const char SET_INSTANCE = 'S';
const char LOG = 'I';
const char DELIMITER = ';';

typedef uint8_t MediaKeyReport[2];
MediaKeyReport     _mediaKeyReport;

typedef struct
{
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

KeyReport _keyReport;

uint16_t vid       = 0x05ac;
uint16_t pid       = 0x820a;
uint16_t version   = 0x0210;

static NimBLEServer* pServer;
NimBLEHIDDevice* hid;
NimBLECharacteristic* inputKeyboard;
NimBLECharacteristic* outputKeyboard;
NimBLECharacteristic* inputMediaKeys;
Preferences preferences;

bool isConnected = false;

uint8_t macAddr[6];

String Command(char identifier, String strData) {
  return String(identifier) + DELIMITER + strData;
}

class DescriptorCallbacks : public NimBLEDescriptorCallbacks {
    void onWrite(NimBLEDescriptor* pDescriptor) {
      std::string dscVal((char*)pDescriptor->getValue(), pDescriptor->getLength());
      Serial.print(LOG + ";");
      Serial.print("Descriptor witten value:");
      Serial.println(dscVal.c_str());
    };

    void onRead(NimBLEDescriptor* pDescriptor) {
      Serial.print(LOG + ";");
      Serial.print(pDescriptor->getUUID().toString().c_str());
      Serial.println(" Descriptor read");
    };
};

class CharacteristicCallbacks: public NimBLECharacteristicCallbacks {
    void onRead(NimBLECharacteristic* pCharacteristic) {
      Serial.println(Command(LOG, pCharacteristic->getUUID().toString().c_str()));
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {
      uint8_t* value = (uint8_t*)(pCharacteristic->getValue().c_str());
      (void)value;
      //Serial.println(Command(LOG, pCharacteristic->getUUID().toString().c_str() + ": onWrite(), value: " + pCharacteristic->getValue().c_str()));

    };
    void onNotify(NimBLECharacteristic* pCharacteristic) {
      Serial.println(Command(LOG, "Sending notification to clients"));
    };
    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {
      String str = ("Notification/Indication status code: ");
      str += status;
      str += ", return code: ";
      str += code;
      str += ", ";
      str += NimBLEUtils::returnCodeToString(code);
      Serial.println(Command(LOG, str));
    };
    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {
      String str = "Client ID: ";
      str += desc->conn_handle;
      str += " Address: ";
      str += std::string(NimBLEAddress(desc->peer_ota_addr)).c_str();
      if (subValue == 0) {
        str += " Unsubscribed to ";
      } else if (subValue == 1) {
        str += " Subscribed to notfications for ";
      } else if (subValue == 2) {
        str += " Subscribed to indications for ";
      } else if (subValue == 3) {
        str += " Subscribed to notifications and indications for ";
      }
      str += std::string(pCharacteristic->getUUID()).c_str();
      Serial.println(Command(LOG, str));
    };
};

static DescriptorCallbacks dscCallbacks;
static CharacteristicCallbacks chrCallbacks;

static const uint8_t _hidReportDescriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop Ctrls)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  // ------------------------------------------------- Keyboard
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1)
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   Logical Maximum (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8)
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8)
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x01,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys
  USAGE_PAGE(1),      0x0C,          // USAGE_PAGE (Consumer)
  USAGE(1),           0x01,          // USAGE (Consumer Control)
  COLLECTION(1),      0x01,          // COLLECTION (Application)
  REPORT_ID(1),       MEDIA_KEYS_ID, //   REPORT_ID (3)
  USAGE_PAGE(1),      0x0C,          //   USAGE_PAGE (Consumer)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  REPORT_COUNT(1),    0x10,          //   REPORT_COUNT (16)
  USAGE(1),           0xB5,          //   USAGE (Scan Next Track)     ; bit 0: 1
  USAGE(1),           0xB6,          //   USAGE (Scan Previous Track) ; bit 1: 2
  USAGE(1),           0xB7,          //   USAGE (Stop)                ; bit 2: 4
  USAGE(1),           0xCD,          //   USAGE (Play/Pause)          ; bit 3: 8
  USAGE(1),           0xE2,          //   USAGE (Mute)                ; bit 4: 16
  USAGE(1),           0xE9,          //   USAGE (Volume Increment)    ; bit 5: 32
  USAGE(1),           0xEA,          //   USAGE (Volume Decrement)    ; bit 6: 64
  USAGE(2),           0x23, 0x02,    //   Usage (WWW Home)            ; bit 7: 128
  USAGE(2),           0x94, 0x01,    //   Usage (My Computer) ; bit 0: 1
  USAGE(2),           0x92, 0x01,    //   Usage (Calculator)  ; bit 1: 2
  USAGE(2),           0x2A, 0x02,    //   Usage (WWW fav)     ; bit 2: 4
  USAGE(2),           0x21, 0x02,    //   Usage (WWW search)  ; bit 3: 8
  USAGE(2),           0x26, 0x02,    //   Usage (WWW stop)    ; bit 4: 16
  USAGE(2),           0x24, 0x02,    //   Usage (WWW back)    ; bit 5: 32
  USAGE(2),           0x83, 0x01,    //   Usage (Media sel)   ; bit 6: 64
  USAGE(2),           0x8A, 0x01,    //   Usage (Mail)        ; bit 7: 128
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  END_COLLECTION(0)                  // END_COLLECTION
};


class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) {
      Serial.println(Command(LOG, "Client connected"));
      isConnected = true;
    };
    void onDisconnect(NimBLEServer* pServer) {
      Serial.println("Client disconnected - start advertising");
      NimBLEDevice::startAdvertising();
      isConnected = false;
    };
    void onConnect(NimBLEServer* pServer, ble_gap_conn_desc* desc) {
      Serial.println(Command(CLIENT_CONNECTED, NimBLEAddress(desc->peer_ota_addr).toString().c_str()));
    };
    void onDisconnect (NimBLEServer *pServer, ble_gap_conn_desc *desc) {
      Serial.println(Command(CLIENT_DISCONNECTED, NimBLEAddress(desc->peer_ota_addr).toString().c_str()));
    }
    void onMTUChange(uint16_t MTU, ble_gap_conn_desc* desc) {
      Serial.println(Command(LOG, "MTU updated: " + String(MTU) + " for connection ID: " + String(desc->conn_handle)));
    };
    uint32_t onPassKeyRequest() {
      Serial.println(Command(LOG, "Server Passkey Request"));
      return 112233;
    };

    bool onConfirmPIN(uint32_t pass_key) {
      Serial.println(Command(LOG, "The passkey YES/NO number: " + String(pass_key)));
      return true;
    };

    void onAuthenticationComplete(ble_gap_conn_desc* desc) {
      if (!desc->sec_state.encrypted) {
        NimBLEDevice::getServer()->disconnect(desc->conn_handle);
        Serial.println(Command(LOG, "Encrypt connection failed - disconnecting client"));
        return;
      }
      Serial.println(Command(LOG, "Starting BLE work!"));
    };
};

void createDescriptor(NimBLECharacteristic* characteristic) {
  NimBLE2904* p2904characteristic = (NimBLE2904*)characteristic->createDescriptor("2904");
  p2904characteristic->setFormat(NimBLE2904::FORMAT_UTF8);
  p2904characteristic->setCallbacks(&dscCallbacks);
}

void setupHid() {
  hid = new NimBLEHIDDevice(pServer);
  inputKeyboard = hid->inputReport(KEYBOARD_ID);  // <-- input REPORTID from report map
  outputKeyboard = hid->outputReport(KEYBOARD_ID);
  inputMediaKeys = hid->inputReport(MEDIA_KEYS_ID);
  outputKeyboard->setCallbacks(&chrCallbacks);
  hid->manufacturer()->setValue("Playr");
  hid->pnp(0x02, vid, pid, version);
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  createDescriptor(inputKeyboard);
  createDescriptor(outputKeyboard);
  createDescriptor(inputMediaKeys);
}

void setupAdversiting() {
  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->setAppearance(HID_KEYBOARD);

  NimBLEService* pAlexaGadgetService = pServer->createService("FE03"); // Alexa Gadged Service
  pAdvertising->addServiceUUID(pAlexaGadgetService->getUUID());
  pAlexaGadgetService->start();

  pAdvertising->addServiceUUID(hid->hidService()->getUUID());

  pAdvertising->setScanResponse(true);
  pAdvertising->start();
}

void startServer(String deviceName) {
  Serial.println(Command(LOG, "Starting Playr Server..."));
  NimBLEDevice::init(deviceName.c_str());
  NimBLEDevice::setPower(ESP_PWR_LVL_P9);
  NimBLEDevice::setSecurityAuth(true, true, true); 
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  setupHid();
  setupAdversiting();

  hid->setBatteryLevel(100);

  Serial.println(Command(LOG, "Started Playr Server!"));
}

int getInstance() {
  preferences.begin("PLAYR", true);
  unsigned int instanceNumber = preferences.getUInt("INST", 0);
  preferences.end();
  return instanceNumber;
}

void setupInstance(int instance) {
  if (instance >= 0 && instance < 255) {
    preferences.begin("PLAYR", false);
    preferences.putUInt("INST", instance);
    preferences.end();
  }
}

void setupMacAddr(int instance) {
  esp_read_mac(macAddr, ESP_MAC_BT);
  int newSuffix = (macAddr[5] + instance > 255 ?  macAddr[5] + instance - 255 : macAddr[5] + instance);
  uint8_t newMac[6] = {macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], newSuffix - 2};
  esp_base_mac_addr_set(newMac);
  Serial.print(Command(LOG, "STANDARD MAC "));
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.print(Command(LOG, "NEW MAC "));
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], newSuffix);
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  //esp_log_level_set("*", ESP_LOG_VERBOSE);
  Serial.println(Command(LOG, "Starting Playr Server..."));
  int instance = getInstance();
  setupMacAddr(instance);
  startServer(instance == 0 ? "Playr Remote" : "Playr Remote #" + String(instance));
  Serial.println(Command(LOG, "Started!"));
}
String message = "";

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void handleMessage() {
  char command = getValue(message, DELIMITER, 0).charAt(0);
  String value = getValue(message, DELIMITER, 1);
  if (command == KEY_DOWN || command == KEY_UP || command == KEY_PRESSED) {
    handleKeyEvent(command, value);
  } else if (command == SET_INSTANCE) {
    handleServerInstance(value);
  }
}

void handleServerInstance(String value) {
  int instance = value.toInt();
  if (instance >= 0 && instance < 255) {
    setupInstance(instance);
    NimBLEDevice::deinit(true);
    //delay(500);
    ESP.restart();
  }
}

void handleKeyEvent(char command, String value) {
  Serial.println(value);
  if (value.indexOf(",") > 0) { // Check if is MediaKey
    int first = getValue(value, ',', 0).toInt();
    int last = getValue(value, ',', 1).toInt();
    MediaKeyReport report = { first, last };
    switch (command) {
      case KEY_DOWN : press(report); break;
      case KEY_UP: release(report); break;
      case KEY_PRESSED: write(report); break;
      default: return;
    }
  } else { // Not a MediaKey
    int key = value.toInt();
    switch (command) {
      case KEY_DOWN : press(key); break;
      case KEY_UP: release(key); break;
      case KEY_PRESSED: write(key); break;
      default: return;
    }
  }
}

void loop() {
  if (Serial.available()) {
    digitalWrite(LED, HIGH);
    message = Serial.readStringUntil('\n');
    digitalWrite(LED, LOW);
    handleMessage();
  }

  if (!isConnected) {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

// HID
#define SHIFT 0x80
const uint8_t _asciimap[128] =
{
  0x00,             // NUL
  0x00,             // SOH
  0x00,             // STX
  0x00,             // ETX
  0x00,             // EOT
  0x00,             // ENQ
  0x00,             // ACK
  0x00,             // BEL
  0x2a,     // BS Backspace
  0x2b,     // TAB  Tab
  0x28,     // LF Enter
  0x00,             // VT
  0x00,             // FF
  0x00,             // CR
  0x00,             // SO
  0x00,             // SI
  0x00,             // DEL
  0x00,             // DC1
  0x00,             // DC2
  0x00,             // DC3
  0x00,             // DC4
  0x00,             // NAK
  0x00,             // SYN
  0x00,             // ETB
  0x00,             // CAN
  0x00,             // EM
  0x00,             // SUB
  0x00,             // ESC
  0x00,             // FS
  0x00,             // GS
  0x00,             // RS
  0x00,             // US

  0x2c,      //  ' '
  0x1e | SHIFT,  // !
  0x34 | SHIFT,  // "
  0x20 | SHIFT,  // #
  0x21 | SHIFT,  // $
  0x22 | SHIFT,  // %
  0x24 | SHIFT,  // &
  0x34,          // '
  0x26 | SHIFT,  // (
  0x27 | SHIFT,  // )
  0x25 | SHIFT,  // *
  0x2e | SHIFT,  // +
  0x36,          // ,
  0x2d,          // -
  0x37,          // .
  0x38,          // /
  0x27,          // 0
  0x1e,          // 1
  0x1f,          // 2
  0x20,          // 3
  0x21,          // 4
  0x22,          // 5
  0x23,          // 6
  0x24,          // 7
  0x25,          // 8
  0x26,          // 9
  0x33 | SHIFT,    // :
  0x33,          // ;
  0x36 | SHIFT,    // <
  0x2e,          // =
  0x37 | SHIFT,    // >
  0x38 | SHIFT,    // ?
  0x1f | SHIFT,    // @
  0x04 | SHIFT,    // A
  0x05 | SHIFT,    // B
  0x06 | SHIFT,    // C
  0x07 | SHIFT,    // D
  0x08 | SHIFT,    // E
  0x09 | SHIFT,    // F
  0x0a | SHIFT,    // G
  0x0b | SHIFT,    // H
  0x0c | SHIFT,    // I
  0x0d | SHIFT,    // J
  0x0e | SHIFT,    // K
  0x0f | SHIFT,    // L
  0x10 | SHIFT,    // M
  0x11 | SHIFT,    // N
  0x12 | SHIFT,    // O
  0x13 | SHIFT,    // P
  0x14 | SHIFT,    // Q
  0x15 | SHIFT,    // R
  0x16 | SHIFT,    // S
  0x17 | SHIFT,    // T
  0x18 | SHIFT,    // U
  0x19 | SHIFT,    // V
  0x1a | SHIFT,    // W
  0x1b | SHIFT,    // X
  0x1c | SHIFT,    // Y
  0x1d | SHIFT,    // Z
  0x2f,          // [
  0x31,          // bslash
  0x30,          // ]
  0x23 | SHIFT,  // ^
  0x2d | SHIFT,  // _
  0x35,          // `
  0x04,          // a
  0x05,          // b
  0x06,          // c
  0x07,          // d
  0x08,          // e
  0x09,          // f
  0x0a,          // g
  0x0b,          // h
  0x0c,          // i
  0x0d,          // j
  0x0e,          // k
  0x0f,          // l
  0x10,          // m
  0x11,          // n
  0x12,          // o
  0x13,          // p
  0x14,          // q
  0x15,          // r
  0x16,          // s
  0x17,          // t
  0x18,          // u
  0x19,          // v
  0x1a,          // w
  0x1b,          // x
  0x1c,          // y
  0x1d,          // z
  0x2f | SHIFT,  //
  0x31 | SHIFT,  // |
  0x30 | SHIFT,  // }
  0x35 | SHIFT,  // ~
  0       // DEL
};

size_t write(uint8_t c)
{
  uint8_t p = press(c);  // Keydown
  release(c);            // Keyup
  return p;              // just return the result of press() since release() almost always returns 1
}

size_t write(const MediaKeyReport c)
{
  uint16_t p = press(c);  // Keydown
  release(c);            // Keyup
  return p;              // just return the result of press() since release() almost always returns 1
}

//size_t write(const uint8_t *buffer, size_t size) {
//  size_t n = 0;
//  while (size--) {
//    if (*buffer != '\r') {
//      if (write(*buffer)) {
//        n++;
//      } else {
//        break;
//      }
//    }
//    buffer++;
//  }
//  return n;
//}

size_t press(uint8_t k)
{
  uint8_t i;
  if (k >= 136) {     // it's a non-printing key (not a modifier)
    k = k - 136;
  } else if (k >= 128) {  // it's a modifier key
    _keyReport.modifiers |= (1 << (k - 128));
    k = 0;
  } else {        // it's a printing key
    k = pgm_read_byte(_asciimap + k);
    if (!k) {
      //setWriteError();
      return 0;
    }
    if (k & 0x80) {           // it's a capital letter or other character reached with shift
      _keyReport.modifiers |= 0x02; // the left shift modifier
      k &= 0x7F;
    }
  }

  // Add k to the key report only if it's not already present
  // and if there is an empty slot.
  if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
      _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
      _keyReport.keys[4] != k && _keyReport.keys[5] != k) {

    for (i = 0; i < 6; i++) {
      if (_keyReport.keys[i] == 0x00) {
        _keyReport.keys[i] = k;
        break;
      }
    }
    if (i == 6) {
      //setWriteError();
      return 0;
    }
  }
  sendReport(&_keyReport);
  return 1;
}

size_t press(const MediaKeyReport k)
{
  uint16_t k_16 = k[1] | (k[0] << 8);
  uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);

  mediaKeyReport_16 |= k_16;
  _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
  _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

  sendReport(&_mediaKeyReport);
  return 1;
}

size_t release(uint8_t k)
{
  uint8_t i;
  if (k >= 136) {     // it's a non-printing key (not a modifier)
    k = k - 136;
  } else if (k >= 128) {  // it's a modifier key
    _keyReport.modifiers &= ~(1 << (k - 128));
    k = 0;
  } else {        // it's a printing key
    k = pgm_read_byte(_asciimap + k);
    if (!k) {
      return 0;
    }
    if (k & 0x80) {             // it's a capital letter or other character reached with shift
      _keyReport.modifiers &= ~(0x02);  // the left shift modifier
      k &= 0x7F;
    }
  }

  // Test the key report to see if k is present.  Clear it if it exists.
  // Check all positions in case the key is present more than once (which it shouldn't be)
  for (i = 0; i < 6; i++) {
    if (0 != k && _keyReport.keys[i] == k) {
      _keyReport.keys[i] = 0x00;
    }
  }

  sendReport(&_keyReport);
  return 1;
}

size_t release(const MediaKeyReport k)
{
  uint16_t k_16 = k[1] | (k[0] << 8);
  uint16_t mediaKeyReport_16 = _mediaKeyReport[1] | (_mediaKeyReport[0] << 8);
  mediaKeyReport_16 &= ~k_16;
  _mediaKeyReport[0] = (uint8_t)((mediaKeyReport_16 & 0xFF00) >> 8);
  _mediaKeyReport[1] = (uint8_t)(mediaKeyReport_16 & 0x00FF);

  sendReport(&_mediaKeyReport);
  return 1;
}

void sendReport(KeyReport* keys)
{
  //  Serial.println("l;Clients: " +  String(inputKeyboard->getSubscribedCount ()));
  //  Serial.println("l;Clients1: " +  String(inputMediaKeys->getSubscribedCount ()));
  inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
  inputKeyboard->notify();

}

void sendReport(MediaKeyReport* keys)
{
  //  Serial.println("l;Clients: " +  String(inputKeyboard->getSubscribedCount ()));
  //  Serial.println("l;Clients1: " +  String(inputMediaKeys->getSubscribedCount ()));
  inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
  inputMediaKeys->notify();
}

void delay_ms(uint64_t ms) {
  uint64_t m = esp_timer_get_time();
  if (ms) {
    uint64_t e = (m + (ms * 1000));
    if (m > e) { //overflow
      while (esp_timer_get_time() > e) { }
    }
    while (esp_timer_get_time() < e) {}
  }
}
