#pragma once

#include <Arduino.h>
#include "secrets.h"

#define UPDATE_INTERVAL_MS 30000
#define DIM_TIMEOUT_MS 30000
#define MIN_BRIGHTNESS 5
#define TIME_START 8*30+30
#define TIME_END 23*30+30

struct Data {
    String tempSejour = "N/A";
    String humiSejour = "N/A";
    String tempExt = "N/A";
    String humiExt = "N/A";
    uint16_t time = 0; // minutes
    bool atHome = false;
};
