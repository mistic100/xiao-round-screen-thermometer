#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "common.h"
#include "sensors.h"
#include "screen.h"

#ifdef WIFI_OTA
#include <ArduinoOTA.h>
#endif

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

#ifdef WIFI_OTA
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setMdnsEnabled(true);
    ArduinoOTA.setPassword(OTA_PASS.c_str());
    ArduinoOTA.begin();
#endif

    draw_bg();
}

void loop()
{
#ifdef WIFI_OTA
    ArduinoOTA.handle();
#endif

    if (update_sensors(data))
    {
        draw_screen(data);
    }
    
    update_brightness(is_always_on(data), chsc6x_is_pressed());
}
