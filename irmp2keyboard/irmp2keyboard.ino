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

//
// IRMP2Keyboard "Main Firmware"
//
// This "sketch" receives infrared remote control codes using the "IRMP" library:
// https://www.mikrocontroller.net/articles/IRMP
// "Known" codes are mapped to keyboard keypresses using "config_keybindings.h"
// The resulting keypresses are either sent to the PC using USB (HID-Project needed!)
// or are communicated via PS2-port over Pin 2 (data) and Pin 3 (clock).
//
// Dependencies: TimerOne
//               HID-Project (for USB support)
//
// Have a look at "config.h" and "config_keybindings.h" for more info!
//


#include <TimerOne.h>
#include "Arduino.h"
#include "src/irmp/irmp.h"

// Include config first (defines which kind of keyboard we will emulate)
#include "config.h"

// The PS2 keyboard code is part of this project
#ifdef PS2_KEYBOARD
  #include "PS2Keyboard.h"
  PS2Keyboard Keyboard(3,2); //(2:data, 3:clock)
// For the USB keyboard, the "HID-Project" library will provide everything we need.
#else
  #include <HID-Project.h>
#endif

// The struct used in "config_keybindings.h"
struct KeyTable {
  IRMP_DATA irkey;
  uint8_t modifiers;
  KeyboardKeycode key;
};

// Modifier bits used in "config_keybindings.h"
#define MOD_NONE 0x00
#define MOD_CTRL 0x01
#define MOD_SHIFT 0x02
#define MOD_ALT 0x04
#define MOD_META 0x08

#include "config_keybindings.h"

void setup() {
  Serial.begin(9600);
  irmp_init();
  Timer1.initialize(1000000 / F_INTERRUPTS);
  Timer1.attachInterrupt(timerinterrupt);
  Keyboard.begin();
}

void timerinterrupt() {
  irmp_ISR();
}


IRMP_DATA lastbutton = {0x00, 0x0000, 0x0000, 0x00};
unsigned long pressed_time;
bool fdc_mode = false;

void loop() {
  IRMP_DATA data;

  // The PS2 keyboard needs regular polling for host communication attempts
#ifdef PS2_KEYBOARD
  if (Keyboard.poll())
    return;
#endif

  if (irmp_get_data(&data)) {
    // Once the first packet with FDC protocol comes in, switch mode
    if (data.protocol == IRMP_FDC_PROTOCOL)
      fdc_mode = true;

    // All "non-keyboard" remote controls.
    // Release is done with a timeout.
    if (!fdc_mode) {
      if (memcmp(&data, &lastbutton, sizeof(IRMP_DATA) - 1) == 0)
        pressed_time = millis();
      else {
        for (unsigned int index = 0; index < (sizeof(REMOTE_KEYS) / sizeof(KeyTable)); index++) {
          if (memcmp(&data, &REMOTE_KEYS[index], sizeof(IRMP_DATA) - 1) == 0) {
            //Serial.println("Pressing");
            Keyboard.releaseAll();
            Keyboard.press(REMOTE_KEYS[index].key);
            pressed_time = millis();
            lastbutton = data;
            break;
          }
        }
      }
    }

    // Special handling for the FDC-3402 keyboard.
    // Actually sends "release codes" to tell us to release a key.
    else if (data.protocol == IRMP_FDC_PROTOCOL) {
      pressed_time = millis();
      lastbutton = data;

      // Read and clear the "button released" bit
      bool pressed = (data.command & 0x0080) ? false : true;
      data.command &= ~0x0080;

      for (unsigned int index = 0; index < (sizeof(REMOTE_KEYS) / sizeof(KeyTable)); index++) {
        if (memcmp(&data, &REMOTE_KEYS[index], sizeof(IRMP_DATA) - 1) == 0) {
          if (pressed) {
            Keyboard.press(REMOTE_KEYS[index].key);
            if (data.command == 0x007E) { // Release Ein/Aus key
              delay(10);
              Keyboard.release(REMOTE_KEYS[index].key);
            }
          }
          else
            Keyboard.release(REMOTE_KEYS[index].key);
          break;
        }
      }
    }
  }

  // Releasetime for FDC is always 1s. Use configured release time for all the others
  unsigned long releasetime = (fdc_mode) ? 1000 : RELEASE_TIME;
  if (lastbutton.protocol && (millis() - pressed_time) > releasetime) {
    //Serial.println("Releasing");
    Keyboard.releaseAll();
    lastbutton.protocol = 0x00;
    fdc_mode = false;
  }
}
