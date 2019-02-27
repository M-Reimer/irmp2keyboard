/*
 * General configuration options for this project
 */

// Comment this line to build for USB support
#define PS2_KEYBOARD

// Timeout in milliseconds after which we expect a remote button to be released.
// Keep this as low as possible but as high as needed for continuous button presses to be still detected.
// Can be measured with "irmpdump".
#define RELEASE_TIME 120
