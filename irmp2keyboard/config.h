/*
 * General configuration options for this project
 */

// Comment this line to build for USB support
#define PS2_KEYBOARD

// Timeout in milliseconds after which we expect a remote button to be released.
// Keep this as low as possible but as high as needed for continuous button presses to be still detected.
// Can be measured with "irmpdump".
#define RELEASE_TIME 120

// Wakeup feature. Only supported on the ATMega32u4 (Pro Micro)
// If you configure an IR keycode here and this keycode is
// detected *and* the connected USB host is shut down, then
// pin A3 on the Arduino is pulled low for a few seconds.
//#define WAKEUP_CODE {0x07,0x000a,0x000c,0x00}
