#pragma once

#include <Arduino.h>
#include "secrets.h"

#define UPDATE_INTERVAL_MS 30000
#define DIM_TIMEOUT_MS 30000
#define MIN_BRIGHTNESS 10
#define TIME_START 8*60+30
#define TIME_END 23*60+30

struct Data {
    String temp1 = "";
    String humi1 = "";
    String temp2 = "";
    String humi2 = "";
    String mode1 = "";
    String mode2 = "";
    uint16_t time = 0; // minutes
    bool atHome = false;
};