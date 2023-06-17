// https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h
// http://www.freebsddiary.org/APC/usb_hid_usages.php

#include "Keys.h"

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

int getModifier(int hidKey) {
  switch (hidKey) {
    case KEY_LEFTCTRL: return MOD_KEY_LEFTCTRL;
    case KEY_LEFTSHIFT: return MOD_KEY_LEFTSHIFT;
    case KEY_LEFTALT: return MOD_KEY_LEFTALT;
    case KEY_LEFTMETA: return MOD_KEY_LEFT_GUI; 
    case KEY_RIGHTCTRL: return MOD_KEY_RIGHTCTRL;
    case KEY_RIGHTSHIFT: return MOD_KEY_RIGHTSHIFT;
    case KEY_RIGHTALT: return MOD_KEY_RIGHTALT;
    case KEY_RIGHTMETA: return MOD_KEY_RIGHT_GUI;
  }
  return -1;
}
