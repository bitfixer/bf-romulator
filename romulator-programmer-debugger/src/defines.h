#ifndef __DEFINES_H__
#define __DEFINES_H__

// pins for connection of wemos d1 mini to romulator

// when programming, MISO and MOSI are reversed wrt debug mode.
// these pins designations are for debug mode
#define  PI_ICE_MISO    13  // hardware MOSI on esp
#define  PI_ICE_MOSI    12  // hardware MISO on esp

#define  PI_ICE_CLK     14
#define  PI_ICE_CRESET  5
#define  PI_ICE_CS      4
#define  PI_DEBUG_CS    16

#define  LED_PIN        2
#define  LED_ON         0
#define  LED_OFF        1

typedef struct wifiSettings {
    char magic[6];
    char ssid[33];
    char password[65];
} WiFiSettings;

#endif