#pragma once

#include <Arduino.h>
#include "secrets.h"

#define UPDATE_INTERVAL_MS 30000
#define DIM_TIMEOUT_MS 10000
#define MIN_BRIGHTNESS 5

struct Data {
    String tempSejour = "N/A";
    String humiSejour = "N/A";
    String tempExt = "N/A";
    String humiExt = "N/A";
};
