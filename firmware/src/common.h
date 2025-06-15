#pragma once

#include <Arduino.h>
#include "secrets.h"

// enable Wifi OTA
#define WIFI_OTA
#define HOSTNAME "xiao-screen"
#define UPDATE_INTERVAL_MS 30000
#define DIM_TIMEOUT_MS 30000
#define MIN_BRIGHTNESS 10
#define TIME_START 8*60+30
#define TIME_END 23*60+30

struct Data {
    uint16_t time = 0; // minutes
    bool atHome = false;
    String power;
    String temp1;
    String humi1;
    String temp2;
    String humi2;
    String mode1;
    String mode2;
};
