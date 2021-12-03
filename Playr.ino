#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>
#include <Preferences.h>
#include "keys.h"

#define KEYBOARD_ID 0x01
#define MEDIA_KEYS_ID 0x02
#define SYSTEM_CONTROL_ID 0x03
#define LED 2

const char ON_KEY_DOWN = 'D';
const char ON_KEY_UP = 'U';
const char ON_KEY_PRESSED = 'P';

const char CLIENT_CONNECTED = 'C';
const char CLIENT_DISCONNECTED = 'O';
const char SET_INSTANCE = 'S';
const char LOG = 'I';
const char DELIMITER = ';';

typedef uint8_t MediaKeyReport[2];
MediaKeyReport _mediaKeyReport;

typedef uint8_t SystemKeyReport[2];
SystemKeyReport _systemKeyReport;

String message = "";

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
NimBLECharacteristic* inputSystemKeys;
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
      //Serial.println(Command(LOG, pCharacteristic->getUUID().toString().c_str()));
    };

    void onWrite(NimBLECharacteristic* pCharacteristic) {};

    void onNotify(NimBLECharacteristic* pCharacteristic) {
      //Serial.println(Command(LOG, "Sending notification to clients"));
    };

    void onStatus(NimBLECharacteristic* pCharacteristic, Status status, int code) {};

    void onSubscribe(NimBLECharacteristic* pCharacteristic, ble_gap_conn_desc* desc, uint16_t subValue) {};
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
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- System Control
  USAGE_PAGE(1), 0x01,              // USAGE_PAGE (Generic Desktop)
  USAGE(1), 0x80,                   // USAGE (System Control)
  COLLECTION(1), 0x01,              // COLLECTION (Application)
  REPORT_ID(1), SYSTEM_CONTROL_ID,  //   REPORT_ID
  REPORT_COUNT(1), 0x01,            //   REPORT_COUNT (1)
  REPORT_SIZE(1), 0x02,             //   REPORT_SIZE (2)
  LOGICAL_MINIMUM(1), 0x01,         //   LOGICAL_MINIMUM (1)
  LOGICAL_MAXIMUM(1), 0x03,         //   LOGICAL_MAXIMUM (3)
  USAGE(1), 0x81,                   //   USAGE (System Power)
  USAGE(1), 0x82,                   //   USAGE (System Sleep)
  USAGE(1), 0x83,                   //   USAGE (System Wakeup)
  HIDINPUT(1), 0x60,                //   INPUT
  REPORT_SIZE(1), 0x06,             //   REPORT_SIZE (6)
  HIDINPUT(1), 0x03,                //   INPUT (Cnst,Var,Abs)
  END_COLLECTION(0),                // END_COLLECTION
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
      return 000000;
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
  inputSystemKeys = hid->inputReport(SYSTEM_CONTROL_ID);
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

// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
// http://www.freebsddiary.org/APC/usb_hid_usages.php
int getHidKeyCode(int inputEventCode) {
  switch (inputEventCode) {
    case 1: return KEY_ESC; // 0x29 DEC: 41
    case 2: return KEY_1; // 0x1E DEC: 30
    case 3: return KEY_2; // 0x1F DEC: 31
    case 4: return KEY_3; // 0x20 DEC: 32
    case 5: return KEY_4; // 0x21 DEC: 33
    case 6: return KEY_5; // 0x22 DEC: 34
    case 7: return KEY_6; // 0x23 DEC: 35
    case 8: return KEY_7; // 0x24 DEC: 36
    case 9: return KEY_8; // 0x25 DEC: 37
    case 10: return KEY_9; // 0x26 DEC: 38
    case 11: return KEY_0; // 0x27 DEC: 39
    case 12: return KEY_MINUS; // 0x2D DEC: 45
    case 13: return KEY_EQUAL; // 0x2E DEC: 46
    case 14: return KEY_BACKSPACE; // 0x2A DEC: 42
    case 15: return KEY_TAB; // 0x2B DEC: 43
    case 16: return KEY_Q; // 0x14 DEC: 20
    case 17: return KEY_W; // 0x1A DEC: 26
    case 18: return KEY_E; // 0x08 DEC: 8
    case 19: return KEY_R; // 0x15 DEC: 21
    case 20: return KEY_T; // 0x17 DEC: 23
    case 21: return KEY_Y; // 0x1C DEC: 28
    case 22: return KEY_U; // 0x18 DEC: 24
    case 23: return KEY_I; // 0x0C DEC: 12
    case 24: return KEY_O; // 0x12 DEC: 18
    case 25: return KEY_P; // 0x13 DEC: 19
    case 26: return KEY_LEFTBRACE; // 0x2F DEC: 47
    case 27: return KEY_RIGHTBRACE; // 0x30 DEC: 48
    case 28: return KEY_ENTER; // 0x28 DEC: 40
    case 29: return KEY_LEFTCTRL; // 0xE0 DEC: 224
    case 30: return KEY_A; // 0x04 DEC: 4
    case 31: return KEY_S; // 0x16 DEC: 22
    case 32: return KEY_D; // 0x07 DEC: 7
    case 33: return KEY_F; // 0x09 DEC: 9
    case 34: return KEY_G; // 0x0A DEC: 10
    case 35: return KEY_H; // 0x0B DEC: 11
    case 36: return KEY_J; // 0x0D DEC: 13
    case 37: return KEY_K; // 0x0E DEC: 14
    case 38: return KEY_L; // 0x0F DEC: 15
    case 39: return KEY_SEMICOLON; // 0x33 DEC: 51
    case 40: return KEY_APOSTROPHE; // 0x34 DEC: 52
    case 41: return KEY_GRAVE; // 0x35 DEC: 53
    case 42: return KEY_LEFTSHIFT; // 0xE1 DEC: 225
    case 43: return KEY_BACKSLASH; // 0x31 DEC: 49
    case 44: return KEY_Z; // 0x1D DEC: 29
    case 45: return KEY_X; // 0x1B DEC: 27
    case 46: return KEY_C; // 0x06 DEC: 6
    case 47: return KEY_V; // 0x19 DEC: 25
    case 48: return KEY_B; // 0x05 DEC: 5
    case 49: return KEY_N; // 0x11 DEC: 17
    case 50: return KEY_M; // 0x10 DEC: 16
    case 51: return KEY_COMMA; // 0x36 DEC: 54
    case 52: return KEY_DOT; // 0x37 DEC: 55
    case 53: return KEY_SLASH; // 0x38 DEC: 56
    case 54: return KEY_RIGHTSHIFT; // 0xE5 DEC: 229
    case 55: return KEY_KPASTERISK; // 0x55 DEC: 85
    case 56: return KEY_LEFTALT; // 0xE2 DEC: 226
    case 57: return KEY_SPACE; // 0x2C DEC: 44
    case 58: return KEY_CAPSLOCK; // 0x39 DEC: 57
    case 59: return KEY_F1; // 0x3A DEC: 58
    case 60: return KEY_F2; // 0x3B DEC: 59
    case 61: return KEY_F3; // 0x3C DEC: 60
    case 62: return KEY_F4; // 0x3D DEC: 61
    case 63: return KEY_F5; // 0x3E DEC: 62
    case 64: return KEY_F6; // 0x3F DEC: 63
    case 65: return KEY_F7; // 0x40 DEC: 64
    case 66: return KEY_F8; // 0x41 DEC: 65
    case 67: return KEY_F9; // 0x42 DEC: 66
    case 68: return KEY_F10; // 0x43 DEC: 67
    case 69: return KEY_NUMLOCK; // 0x53 DEC: 83
    case 70: return KEY_SCROLLLOCK; // 0x47 DEC: 71
    case 71: return KEY_KP7; // 0x5F DEC: 95
    case 72: return KEY_KP8; // 0x60 DEC: 96
    case 73: return KEY_KP9; // 0x61 DEC: 97
    case 74: return KEY_KPMINUS; // 0x56 DEC: 86
    case 75: return KEY_KP4; // 0x5C DEC: 92
    case 76: return KEY_KP5; // 0x5D DEC: 93
    case 77: return KEY_KP6; // 0x5E DEC: 94
    case 78: return KEY_KPPLUS; // 0x57 DEC: 87
    case 79: return KEY_KP1; // 0x59 DEC: 89
    case 80: return KEY_KP2; // 0x5A DEC: 90
    case 81: return KEY_KP3; // 0x5B DEC: 91
    case 82: return KEY_KP0; // 0x62 DEC: 98
    case 83: return KEY_KPDOT; // 0x63 DEC: 99
    case 85: return KEY_ZENKAKUHANKAKU; // 0x94 DEC: 148
    case 86: return KEY_102ND; // 0x64 DEC: 100
    case 87: return KEY_F11; // 0x44 DEC: 68
    case 88: return KEY_F12; // 0x45 DEC: 69
    case 89: return KEY_RO; // 0x87 DEC: 135
    case 90: return KEY_KATAKANA; // 0x92 DEC: 146
    case 91: return KEY_HIRAGANA; // 0x93 DEC: 147
    case 92: return KEY_HENKAN; // 0x8A DEC: 138
    case 93: return KEY_KATAKANAHIRAGANA; // 0x88 DEC: 136
    case 94: return KEY_MUHENKAN; // 0x8B DEC: 139
    case 95: return KEY_KPJPCOMMA; // 0x8C DEC: 140
    case 96: return KEY_KPENTER; // 0x58 DEC: 88
    case 97: return KEY_RIGHTCTRL; // 0xE4 DEC: 228
    case 98: return KEY_KPSLASH; // 0x54 DEC: 84
    case 99: return KEY_SYSRQ; // 0x46 DEC: 70
    case 100: return KEY_RIGHTALT; // 0xE6 DEC: 230
    case 102: return KEY_HOME; // 0x4A DEC: 74
    case 103: return KEY_UP; // 0x52 DEC: 82
    case 104: return KEY_PAGEUP; // 0x4B DEC: 75
    case 105: return KEY_LEFT; // 0x50 DEC: 80
    case 106: return KEY_RIGHT; // 0x4F DEC: 79
    case 107: return KEY_END; // 0x4D DEC: 77
    case 108: return KEY_DOWN; // 0x51 DEC: 81
    case 109: return KEY_PAGEDOWN; // 0x4E DEC: 78
    case 110: return KEY_INSERT; // 0x49 DEC: 73
    case 111: return KEY_DELETE; // 0x4C DEC: 76
    case 113: return KEY_MUTE; // 0x7F DEC: 127
    case 114: return KEY_VOLUMEDOWN; // 0x81 DEC: 129
    case 115: return KEY_VOLUMEUP; // 0x80 DEC: 128
    case 116: return KEY_POWER; // 0x66 DEC: 102
    case 117: return KEY_KPEQUAL; // 0x67 DEC: 103
    case 119: return KEY_PAUSE; // 0x48 DEC: 72
    case 121: return KEY_KPCOMMA; // 0x85 DEC: 133
    case 122: return KEY_HANGEUL; // 0x90 DEC: 144
    case 123: return KEY_HANJA; // 0x91 DEC: 145
    case 124: return KEY_YEN; // 0x89 DEC: 137
    case 125: return KEY_LEFTMETA; // 0xE3 DEC: 227
    case 126: return KEY_RIGHTMETA; // 0xE7 DEC: 231
    case 127: return KEY_COMPOSE; // 0x65 DEC: 101
    case 128: return KEY_STOP; // 0x78 DEC: 120
    case 129: return KEY_AGAIN; // 0x79 DEC: 121
    case 130: return KEY_PROPS; // 0x76 DEC: 118
    case 131: return KEY_UNDO; // 0x7A DEC: 122
    case 132: return KEY_FRONT; // 0x77 DEC: 119
    case 133: return KEY_COPY; // 0x7C DEC: 124
    case 134: return KEY_OPEN; // 0x74 DEC: 116
    case 135: return KEY_PASTE; // 0x7D DEC: 125
    case 136: return KEY_FIND; // 0x7E DEC: 126
    case 137: return KEY_CUT; // 0x7B DEC: 123
    case 138: return KEY_HELP; // 0x75 DEC: 117
    case 140: return KEY_CALC; // 0xFB DEC: 251
    case 142: return KEY_SLEEP; // 0xF8 DEC: 248
    case 152: return KEY_COFFEE; // 0xF9 DEC: 249
    case 158: return KEY_BACK; // 0xF1 DEC: 241
    case 159: return KEY_FORWARD; // 0xF2 DEC: 242
    case 161: return KEY_EJECTCD; // 0xEC DEC: 236
    case 163: return KEY_NEXTSONG; // 0xEB DEC: 235
    case 164: return KEY_PLAYPAUSE; // 0xE8 DEC: 232
    case 165: return KEY_PREVIOUSSONG; // 0xEA DEC: 234
    case 166: return KEY_STOPCD; // 0xE9 DEC: 233
    case 172: return KEY_HOMEPAGE; // 0xF0 DEC: 240
    case 173: return KEY_REFRESH; // 0xFA DEC: 250
    case 176: return KEY_EDIT; // 0xF7 DEC: 247
    case 177: return KEY_SCROLLUP; // 0xF5 DEC: 245
    case 178: return KEY_SCROLLDOWN; // 0xF6 DEC: 246
    case 179: return KEY_KPLEFTPAREN; // 0xB6 DEC: 182
    case 180: return KEY_KPRIGHTPAREN; // 0xB7 DEC: 183
    case 183: return KEY_F13; // 0x68 DEC: 104
    case 184: return KEY_F14; // 0x69 DEC: 105
    case 185: return KEY_F15; // 0x6A DEC: 106
    case 186: return KEY_F16; // 0x6B DEC: 107
    case 187: return KEY_F17; // 0x6C DEC: 108
    case 188: return KEY_F18; // 0x6D DEC: 109
    case 189: return KEY_F19; // 0x6E DEC: 110
    case 190: return KEY_F20; // 0x6F DEC: 111
    case 191: return KEY_F21; // 0x70 DEC: 112
    case 192: return KEY_F22; // 0x71 DEC: 113
    case 193: return KEY_F23; // 0x72 DEC: 114
    case 194: return KEY_F24; // 0x73 DEC: 115
    default: return 0;
  }
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
  Serial.print(Command(LOG, "Default Mac "));
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
  Serial.print(Command(LOG, "New Mac "));
  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], newSuffix);
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  Serial.println(Command(LOG, "Starting Playr Server..."));
  int instance = getInstance();
  setupMacAddr(instance);
  startServer(instance == 0 ? "Playr Remote" : "Playr Remote #" + String(instance));
}

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
  if (command == ON_KEY_DOWN || command == ON_KEY_UP || command == ON_KEY_PRESSED) {
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
    ESP.restart();
  }
}

void handleKeyEvent(char command, String value) {
  int key = getHidKeyCode(value.toInt());
  if (key == KEY_NEXTSONG || key == KEY_PREVIOUSSONG || key == KEY_STOP || key == KEY_PLAYPAUSE ||
      key == KEY_MUTE || key == KEY_VOLUMEUP || key == KEY_VOLUMEDOWN || key == KEY_BACK || key == KEY_HOMEPAGE) {
    switch (command) {
      case ON_KEY_DOWN: pressMediaKey(key); break;
      case ON_KEY_UP: releaseMediaKey(key); break;
      case ON_KEY_PRESSED: writeMediaKey(key); break;
      default: return;
    }
    return;
  }

  if (key == KEY_POWER || key == KEY_SLEEP) {
    switch (command) {
      case ON_KEY_DOWN: pressSystemKey(key); break;
      case ON_KEY_UP: releaseSystemKey(key); break;
      case ON_KEY_PRESSED: writeSystemKey(key); break;
      default: return;
    }
    return;
  }

  switch (command) {
    case ON_KEY_DOWN: press(key); break;
    case ON_KEY_UP: release(key); break;
    case ON_KEY_PRESSED: write(key); break;
    default: return;
  }
}

int ledState = LOW;
unsigned long previousMillis = 0;

void loop() {
  if (Serial.available()) {
    digitalWrite(LED, HIGH);
    message = Serial.readStringUntil('\n');
    handleMessage();
    digitalWrite(LED, LOW);
  }

  if (!isConnected) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 500) {
      ledState = (ledState == LOW) ? HIGH : LOW;
      digitalWrite(LED, ledState);
      previousMillis = currentMillis;
    }
  } else if (ledState == HIGH) {
    digitalWrite(LED, LOW);
  }
}

size_t write(uint8_t c)
{
  uint8_t p = press(c);
  release(c);
  return p;
}

size_t press(uint8_t hidKey)
{
  uint8_t i;
  if (hidKey == 0) {
    return 0;
  }
  switch (hidKey) {
    case KEY_LEFTCTRL: _keyReport.modifiers |= MOD_KEY_LEFTCTRL; hidKey = 0; break;
    case KEY_LEFTSHIFT: _keyReport.modifiers |= MOD_KEY_LEFTSHIFT; hidKey = 0; break;
    case KEY_LEFTALT: _keyReport.modifiers |= MOD_KEY_LEFTALT; hidKey = 0; break;
    case KEY_LEFTMETA: _keyReport.modifiers |= MOD_KEY_LEFT_GUI; hidKey = 0; break; // aka Windows Key
    case KEY_RIGHTCTRL: _keyReport.modifiers |= MOD_KEY_RIGHTCTRL; hidKey = 0; break;
    case KEY_RIGHTSHIFT: _keyReport.modifiers |= MOD_KEY_RIGHTSHIFT; hidKey = 0; break;
    case KEY_RIGHTALT: _keyReport.modifiers |= MOD_KEY_RIGHTALT; hidKey = 0; break;
    case KEY_RIGHTMETA: _keyReport.modifiers |= MOD_KEY_RIGHT_GUI; hidKey = 0; break; // aka Windows Key
  }

  if (_keyReport.keys[0] != hidKey && _keyReport.keys[1] != hidKey &&
      _keyReport.keys[2] != hidKey && _keyReport.keys[3] != hidKey &&
      _keyReport.keys[4] != hidKey && _keyReport.keys[5] != hidKey) {

    for (i = 0; i < 6; i++) {
      if (_keyReport.keys[i] == 0x00) {
        _keyReport.keys[i] = hidKey;
        break;
      }
    }
    if (i == 6) {
      return 0;
    }
  }
  sendReport(&_keyReport);
  return 1;
}

size_t release(uint8_t hidKey)
{
  uint8_t i;

  if (hidKey == 0) {
    return 0;
  }

  switch (hidKey) {
    case KEY_LEFTCTRL: _keyReport.modifiers &= ~MOD_KEY_LEFTCTRL; hidKey = 0; break;
    case KEY_LEFTSHIFT: _keyReport.modifiers &= ~MOD_KEY_LEFTSHIFT; hidKey = 0; break;
    case KEY_LEFTALT: _keyReport.modifiers &= ~MOD_KEY_LEFTALT; hidKey = 0; break;
    case KEY_LEFTMETA: _keyReport.modifiers &= ~MOD_KEY_LEFT_GUI; hidKey = 0; break; // aka Windows Key
    case KEY_RIGHTCTRL: _keyReport.modifiers &= ~MOD_KEY_RIGHTCTRL; hidKey = 0; break;
    case KEY_RIGHTSHIFT: _keyReport.modifiers &= ~MOD_KEY_RIGHTSHIFT; hidKey = 0; break;
    case KEY_RIGHTALT: _keyReport.modifiers &= ~MOD_KEY_RIGHTALT; hidKey = 0; break;
    case KEY_RIGHTMETA: _keyReport.modifiers &= ~MOD_KEY_RIGHT_GUI; hidKey = 0; break; // aka Windows Key
  }

  for (i = 0; i < 6; i++) {
    if (0 != hidKey && _keyReport.keys[i] == hidKey) {
      _keyReport.keys[i] = 0x00;
    }
  }

  sendReport(&_keyReport);
  return 1;
}

size_t writeMediaKey(uint8_t mediaKey)
{
  uint8_t p = pressMediaKey(mediaKey);
  releaseMediaKey(mediaKey);
  return p;
}

size_t pressMediaKey(uint8_t mediaKey)
{
  switch (mediaKey) {
    case KEY_NEXTSONG:
      _mediaKeyReport[0] = 0x01;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_PREVIOUSSONG:
      _mediaKeyReport[0] = 0x02;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_STOP:
      _mediaKeyReport[0] = 0x04;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_PLAYPAUSE:
      _mediaKeyReport[0] = 0x08;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_MUTE:
      _mediaKeyReport[0] = 0x10;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_VOLUMEUP:
      _mediaKeyReport[0] = 0x20;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_VOLUMEDOWN:
      _mediaKeyReport[0] = 0x40;
      _mediaKeyReport[1] = 0x00;
      break;
    case KEY_BACK:
      _mediaKeyReport[0] = 0x00;
      _mediaKeyReport[1] = 0x20;
      break;
    case KEY_HOMEPAGE:
      _mediaKeyReport[0] = 0x80;
      _mediaKeyReport[1] = 0x00;
      break;
  }
  sendReport(&_mediaKeyReport);
  return 1;
}

size_t releaseMediaKey(uint8_t mediaKey)
{
  _mediaKeyReport[0] = 0x00;
  _mediaKeyReport[1] = 0x00;
  sendReport(&_mediaKeyReport);
  return 1;
}

size_t writeSystemKey(uint8_t systemKey)
{
  uint8_t p = pressSystemKey(systemKey);
  releaseSystemKey(systemKey);
  return p;
}

size_t pressSystemKey(uint8_t systemKey)
{
  switch (systemKey) {
    case KEY_POWER:
      _systemKeyReport[0] = 0x01;
      _systemKeyReport[1] = 0x00;
      break;
    case KEY_SLEEP:
      _systemKeyReport[0] = 0x02;
      _systemKeyReport[1] = 0x00;
      break;
  }
  sendSystemReport(&_systemKeyReport);
  return 1;
}

size_t releaseSystemKey(uint8_t systemKey)
{
  _systemKeyReport[0] = 0x00;
  _systemKeyReport[1] = 0x00;
  sendSystemReport(&_systemKeyReport);
  return 1;
}

void sendReport(KeyReport* keys)
{
  inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
  inputKeyboard->notify();
}

void sendReport(MediaKeyReport* keys)
{
  inputMediaKeys->setValue((uint8_t*)keys, sizeof(MediaKeyReport));
  inputMediaKeys->notify();
}

void sendSystemReport(SystemKeyReport* keys)
{
  inputSystemKeys->setValue((uint8_t*)keys, sizeof(SystemKeyReport));
  inputSystemKeys->notify();
}
