#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>

#include "common.h"
#include "sensors.h"
#include "screen.h"

Data data;

void setup(void)
{
    Serial.begin(115200);

    if (!LittleFS.begin(false))
    {
        log_e("Failed to init LittleFS");
        return;
    }

    init_sensors();
    init_screen();
    lv_xiao_touch_init();

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        display_error("WAIT WIFI");
        delay(500);
    }

    draw_bg();
}

void loop()
{
    if (update_sensors(data))
    {
        draw_screen(data);
    }
    update_brightness(is_always_on(data), chsc6x_is_pressed());
}
