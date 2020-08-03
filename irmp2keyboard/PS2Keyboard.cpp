/*
    IRMP2Keyboard infrared remote to PS2/USB keyboard converter
    Copyright (C) 2020 Manuel Reimer <manuel.reimer@gmx.de>

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

// Based on "ps2dev.h" and "ps2dev.cpp"
// http://playground.arduino.cc/ComponentLib/Ps2mouse
// All modifications done here are subject to the GPL3 license!


#include "config.h"
#ifdef PS2_KEYBOARD // Only build the whole file if we are not building for USB!

#include "Arduino.h"
#include "PS2Keyboard.h"

// Switch to true to get some debug stuff on serial monitor
#define DEBUG false

//since for the device side we are going to be in charge of the clock,
//the two defines below are how long each _phase_ of the clock cycle is
#define CLKFULL 40 //µs
// we make changes in the middle of a phase, this how long from the
// start of phase to the when we drive the data line
#define CLKHALF 20 //µs
// The keyboard controller needs some time to accept the sent byte.
// The following value was measured from a "real" no-name PS2 keyboard.
#define BYTE_SPACING 2 //ms

/*
 * the clock and data pins can be wired directly to the clk and data pins
 * of the PS2 connector.  No external parts are needed.
 */
PS2Keyboard::PS2Keyboard(int clk, int data) {
  _ps2clk = clk;
  _ps2data = data;
  gohi(_ps2clk);
  gohi(_ps2data);
  for (int index = 0; index < MAX_KEYS; index++)
    _keylist[index] = KEY_NONE;
}

// Pull line "high"
// Configure as input first
// --> Pin either high impedance or pulled up
// Then write "high"
// --> Pin pulled up
void PS2Keyboard::gohi(int pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

// Pull line "low"
// Write "low" first
// --> Pin either high impedance or pulled low
// Then set pin as output
// --> Pin pulled low
void PS2Keyboard::golo(int pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

int PS2Keyboard::write(unsigned char data) {
  unsigned char i;
  unsigned char parity = 1;

  if (digitalRead(_ps2clk) == LOW) {
    return -1;
  }

  if (digitalRead(_ps2data) == LOW) {
    return -2;
  }

  golo(_ps2data);
  delayMicroseconds(CLKHALF);
  // device sends on falling clock
  golo(_ps2clk);	// start bit
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);

  for (i=0; i < 8; i++) {
    if (data & 0x01) {
      gohi(_ps2data);
    } else {
      golo(_ps2data);
    }
    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }

  // parity bit
  if (parity) {
    gohi(_ps2data);
  } else {
    golo(_ps2data);
  }
  delayMicroseconds(CLKHALF);
  golo(_ps2clk);
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);

  // stop bit
  gohi(_ps2data);
  delayMicroseconds(CLKHALF);
  golo(_ps2clk);
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);

  // At this point we are disengaged from the the clock line. The PS2
  // controller should pull the line low for a few microseconds to acknowledge
  // our transfer. We wait for that or timeout after BYTE_SPACING ms.
  unsigned long start = millis();
  while (digitalRead(_ps2clk) == HIGH) {
    if (millis() - start >= BYTE_SPACING)
      return 0;
  }
  while (digitalRead(_ps2clk) == LOW) {
    if (millis() - start >= BYTE_SPACING)
      return 0;
  }

  return 0;
}


int PS2Keyboard::read(unsigned char * value) {
  unsigned char data = 0x00;
  unsigned char i;
  unsigned char bit = 0x01;

  unsigned char parity = 1;

  //wait for data line to go low
  unsigned long start = millis();
  while (digitalRead(_ps2data) == HIGH) {
    // If something went wrong (stuck?)
    if (millis() - start > 500)
      return -1;
  }
  //wait for clock line to go high
  while (digitalRead(_ps2clk) == LOW) {

  }

  delayMicroseconds(CLKHALF);
  golo(_ps2clk);
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);

  for (i=0; i < 8; i++) {
    if (digitalRead(_ps2data) == HIGH) {
      data = data | bit;
    } else {
    }

    bit = bit << 1;

    delayMicroseconds(CLKHALF);
    golo(_ps2clk);
    delayMicroseconds(CLKFULL);
    gohi(_ps2clk);
    delayMicroseconds(CLKHALF);

    parity = parity ^ (data & 0x01);
  }
  // we do the delay at the end of the loop, so at this point we have
  // already done the delay for the parity bit

  // stop bit
  delayMicroseconds(CLKHALF);
  golo(_ps2clk);
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);

  delayMicroseconds(CLKHALF);
  golo(_ps2data);
  golo(_ps2clk);
  delayMicroseconds(CLKFULL);
  gohi(_ps2clk);
  delayMicroseconds(CLKHALF);
  gohi(_ps2data);

  *value = data;

  return 0;
}

// Send "acknowledge" to the host.
// This is used in "HandleCommand" below.
void PS2Keyboard::ack() {
  //acknowledge commands
  while(write(0xFA));
}

// Handles commands sent from the host to us.
// Most of them are just ignored and blindli "acked".
void PS2Keyboard::HandleCommand(unsigned char command) {
  if (DEBUG) {
    Serial.println("Got request from PC");
    Serial.print(command, HEX);
    Serial.println();
  }
  unsigned char val;
  switch (command) {
  case 0xFF: //reset
    ack();
    //the while loop lets us wait for the host to be ready
    while(write(0xAA)!=0);
    _enabled = true;
    break;
  case 0xFE: //resend
    ack();
    break;
  case 0xF6: //set defaults
    //enter stream mode
    ack();
    break;
  case 0xF5: //disable data reporting
    //FM
    _enabled = false;
    ack();
    break;
  case 0xF4: //enable data reporting
    //FM
    _enabled = true;
    ack();
    break;
  case 0xF3: //set typematic rate
    ack();
    read(&val); //do nothing with the rate
    ack();
    break;
  case 0xF2: //get device id
    ack();
    while (write(0xAB)!=0);
    while (write(0x83)!=0);
    break;
  case 0xF0: //set scan code set
    ack();
    read(&val); //do nothing with the rate
    ack();
    break;
  case 0xEE: //echo
    //ack();
    write(0xEE);
    break;
  case 0xED: //set/reset LEDs
    ack();
    read(&val); //do nothing with the rate
    ack();
    break;
  default:
    Serial.println("Got unknown request from PC");
    Serial.print(command, HEX);
    Serial.println();
  }
}

// Tell host that we finished self test.
void PS2Keyboard::begin() {
  // send the keyboard start up
  while(write(0xAA)!=0);
  _enabled = true;
}

// This function checks if the host wants to send data to the
// keyboard. If so, it handles the communication.
bool PS2Keyboard::poll() {
  // "Communication interrupt" (Host pulls Clock low)
  if (digitalRead(_ps2clk) == LOW) {
    //Serial.println("Clock low");
    while (digitalRead(_ps2clk) == LOW);
  }

  // Host wants to talk to us (Data is pulled low)
  unsigned char c = 0;
  if(digitalRead(_ps2data) == LOW) {
    //Serial.println("Data low");
    read(&c);
    HandleCommand(c);
    return true;
  }

  return false;
}

size_t PS2Keyboard::press(KeyboardKeycode key) {
  // Is there a free position to save the key?
  int pos = -1;
  for (int index = 0; index < MAX_KEYS; index++) {
    if (_keylist[index] == KEY_NONE && pos == -1)
      pos = index;
    if (_keylist[index] == key)
      return 0;
  }
  if (pos == -1)
    return 0;

  // Split the two bytes
  KeycodeUnion k;
  k.key = key;

  // If communication is interrupted, send complete sequence again.
  while (true) {
    if (poll())
      continue;

    if (k.bytes[1] != 0xFF) { // All "regular" scancodes
      if (k.bytes[1] != 0x00) {
        if (write(k.bytes[1]))
          continue;
      }

      if (write(k.bytes[0]))
        continue;
    }
    else if (k.bytes[0] == 0x01) { // Print Screen
      if (write(0xE0))
        continue;
      if (write(0x12))
        continue;
      if (write(0xE0))
        continue;
      if (write(0x7C))
        continue;
    }
    else if (k.bytes[0] == 0x02) { // Pause
      if (write(0xE1))
        continue;
      if (write(0x14))
        continue;
      if (write(0x77))
        continue;
      if (write(0xE1))
        continue;
      if (write(0xF0))
        continue;
      if (write(0x14))
        continue;
      if (write(0xF0))
        continue;
      if (write(0x77))
        continue;
    }

    break;
  }

  _keylist[pos] = key;
  return 1;
}
size_t PS2Keyboard::release(KeyboardKeycode key) {
  // KEY_NONE is a placeholder for "no key"
  if (key == KEY_NONE)
    return 0;

  // Is the key pressed?
  int pos = -1;
  for (int index = 0; index < MAX_KEYS; index++) {
    if (_keylist[index] == key) {
      pos = index;
      break;
    }
  }
  if (pos == -1)
    return 0;

  // Split the two bytes
  KeycodeUnion k;
  k.key = key;

  // If communication is interrupted, send complete sequence again.
  while (true) {
    if (poll())
      continue;

    if (k.bytes[1] != 0xFF) { // All "regular" scancodes
      if (k.bytes[1] != 0x00) {
        if (write(k.bytes[1]))
          continue;
      }

      if (write(0xF0))
        continue;

      if (write(k.bytes[0]))
        continue;
    }
    else if (k.bytes[0] == 0x01) { // Print Screen
      if (write(0xE0))
        continue;
      if (write(0xF0))
        continue;
      if (write(0x7C))
        continue;
      if (write(0xE0))
        continue;
      if (write(0xF0))
        continue;
      if (write(0x12))
        continue;
    }

    break;
  }

  _keylist[pos] = KEY_NONE;
  return 1;
}
size_t PS2Keyboard::releaseAll(void) {
  for (int index = MAX_KEYS - 1; index >= 0; index--)
    release(_keylist[index]);
  return 0;
}

#endif // ifdef PS2_KEYBOARD
