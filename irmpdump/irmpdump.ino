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
// IRMP2Keyboard IRMPDump
//
// This "sketch" is used to "learn" your remote control keys.
// Upload into any supported Arduino (equipped with an infrared receiver).
// Then open the "Serial Monitor" and press the button, you want to decode, on your remote control.
// Keep the button held down to measure your remote control release time.
//
// Dependencies: TimerOne
//
// Note: This sketch can also be used to "scan" unknown codes.
//       To do this, go to src/irmp and enable "IRMP_LOGGING" in irmpconfig.h
//       Scans can be collected on the "Serial Monitor".
//
// Wiring: Infrared input (TSOP)
//           Pin 8 (D8) on Arduino Pro Micro (or Leonardo)
//           Pin 12 (D12) on Arduino Nano (or Uno)
//


#include <TimerOne.h>
#include "Arduino.h"
#include "src/irmp/irmp.h"
#include "src/irmp/irmpextlog.h"

void setup() {
  Serial.begin(9600);
  irmp_init();
  Timer1.initialize(1000000 / F_INTERRUPTS);
  Timer1.attachInterrupt(timerinterrupt);
}

void timerinterrupt() {
  irmp_ISR();
}

void initextlog(void) {} // Not needed at all. We have "setup" for this
void sendextlog(unsigned char dump) {
  Serial.write(dump);
}

unsigned long last_time;
void loop() {
  IRMP_DATA data;
  if (irmp_get_data(&data)) {
    // Button got pressed: Preformat and output code
    if (! (data.flags & IRMP_FLAG_REPETITION)) {
      last_time = millis();
      if (data.protocol == IRMP_FDC_PROTOCOL)
        data.command &= ~0x0080;
      char result[100];
      snprintf(result, 100, "0x%02x,0x%04x,0x%04x,0x%02x ", data.protocol, data.address, data.command, data.flags);
      Serial.println(result);
    }
    // Button is held down: Do release time calculation
    else {
      unsigned long delay = millis() - last_time;
      Serial.print("Release Time: ");
      Serial.println(delay);
      last_time = millis();
    }
  }
}
