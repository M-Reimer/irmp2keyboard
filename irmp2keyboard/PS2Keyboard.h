/*
    IRMP2Keyboard infrared remote to PS2/USB keyboard converter
    Copyright (C) 2019 Manuel Reimer <manuel.reimer@gmx.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#define MAX_KEYS 6

// We only support keys with up to two bytes to save space
enum KeyboardKeycode : uint16_t {
  KEY_NONE = 0x0000,
  KEY_A = 0x001C,
  KEY_B = 0x0032,
  KEY_C = 0x0021,
  KEY_D = 0x0023,
  KEY_E = 0x0024,
  KEY_F = 0x002B,
  KEY_G = 0x0034,
  KEY_H = 0x0033,
  KEY_I = 0x0043,
  KEY_J = 0x003B,
  KEY_K = 0x0042,
  KEY_L = 0x004B,
  KEY_M = 0x003A,
  KEY_N = 0x0031,
  KEY_O = 0x0044,
  KEY_P = 0x004D,
  KEY_Q = 0x0015,
  KEY_R = 0x002D,
  KEY_S = 0x001B,
  KEY_T = 0x002C,
  KEY_U = 0x003C,
  KEY_V = 0x002A,
  KEY_W = 0x001D,
  KEY_X = 0x0022,
  KEY_Y = 0x0035,
  KEY_Z = 0x001A,
  KEY_1 = 0x0016,
  KEY_2 = 0x001E,
  KEY_3 = 0x0026,
  KEY_4 = 0x0025,
  KEY_5 = 0x002E,
  KEY_6 = 0x0036,
  KEY_7 = 0x003D,
  KEY_8 = 0x003E,
  KEY_9 = 0x0046,
  KEY_0 = 0x0045,
  KEY_ENTER = 0x005A,
  KEY_RETURN = 0x005A, // Alias
  KEY_ESC = 0x0076,
  KEY_BACKSPACE = 0x0066,
  KEY_TAB = 0x000D,
  KEY_SPACE = 0x0029,
  KEY_MINUS = 0x004E,
  KEY_EQUAL = 0x0055,
  KEY_LEFT_BRACE = 0x0054,
  KEY_RIGHT_BRACE = 0x005B,
  KEY_BACKSLASH = 0x005D,
  KEY_SEMICOLON = 0x004C,
  KEY_QUOTE = 0x0052,
  KEY_TILDE = 0x000E,
  KEY_COMMA = 0x0041,
  KEY_PERIOD = 0x0049,
  KEY_SLASH = 0x004A,
  KEY_CAPS_LOCK  = 0x0058,
  KEY_F1 = 0x0005,
  KEY_F2 = 0x0006,
  KEY_F3 = 0x0004,
  KEY_F4 = 0x000C,
  KEY_F5 = 0x0003,
  KEY_F6 = 0x000B,
  KEY_F7 = 0x0083,
  KEY_F8 = 0x000A,
  KEY_F9 = 0x0001,
  KEY_F10 = 0x0009,
  KEY_F11 = 0x0078,
  KEY_F12 = 0x0007,

  KEY_INSERT = 0xE070,
  KEY_HOME = 0xE06C,
  KEY_PAGE_UP = 0xE07D,
  KEY_DELETE = 0xE071,
  KEY_END = 0xE069,
  KEY_PAGE_DOWN = 0xE07A,
  KEY_RIGHT_ARROW = 0xE074,
  KEY_LEFT_ARROW = 0xE06B,
  KEY_DOWN_ARROW = 0xE072,
  KEY_UP_ARROW = 0xE075,
  KEY_RIGHT = 0xE074, // Alias
  KEY_LEFT = 0xE06B, // Alias
  KEY_DOWN = 0xE072, // Alias
  KEY_UP = 0xE075, // Alias
  KEY_NUM_LOCK = 0x0077,
  KEYPAD_DIVIDE = 0xE04A,
  KEYPAD_MULTIPLY = 0x007C,
  KEYPAD_SUBTRACT = 0x007B,
  KEYPAD_ADD = 0x0079,
  KEYPAD_ENTER = 0xE05A,
  KEYPAD_1 = 0x0069,
  KEYPAD_2 = 0x0072,
  KEYPAD_3 = 0x007A,
  KEYPAD_4 = 0x006B,
  KEYPAD_5 = 0x0073,
  KEYPAD_6 = 0x0074,
  KEYPAD_7 = 0x006C,
  KEYPAD_8 = 0x0075,
  KEYPAD_9 = 0x007D,
  KEYPAD_0 = 0x0070,
  KEYPAD_DOT = 0x0071,

  KEY_APPLICATION = 0xE02F, // Context menu/right click
  KEY_MENU = 0xE02F, // Alias

  KEY_POWER = 0xE037,

  KEY_LEFT_CTRL = 0x0014,
  KEY_LEFT_SHIFT = 0x0012,
  KEY_LEFT_ALT = 0x0011,
  KEY_LEFT_GUI = 0xE01F,
  KEY_LEFT_WINDOWS = 0xE01F, // Alias
  KEY_RIGHT_CTRL = 0xE014,
  KEY_RIGHT_SHIFT = 0x0059,
  KEY_RIGHT_ALT = 0xE011,
  KEY_RIGHT_GUI = 0xE027,
  KEY_RIGHT_WINDOWS = 0xE027, // Alias

  KEY_NON_US = 0x0061, // Not available on US keyboards. Combined "<", ">" and "|" on DE keyboard.
};

union KeycodeUnion {
  KeyboardKeycode key;
  uint8_t bytes[2];
};

class PS2Keyboard {
public:
  PS2Keyboard(int clk, int data);
  size_t press(KeyboardKeycode k);
  size_t release(KeyboardKeycode k);
  size_t releaseAll(void);
  bool poll();
  void begin();

private:
  KeyboardKeycode _keylist[MAX_KEYS];
  int _ps2clk;
  int _ps2data;
  bool _enabled;
  void golo(int pin);
  void gohi(int pin);
  int write(unsigned char data);
  int read(unsigned char * data);
  void ack();
  void HandleCommand(unsigned char command);
};
