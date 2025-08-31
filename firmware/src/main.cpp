#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "common.h"
#include "sensors.h"
#include "screen.h"

#ifdef WIFI_OTA
#include <ArduinoOTA.h>
#endif

#define TOUCH_INT D7

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

    pinMode(TOUCH_INT, INPUT_PULLUP);

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

    log_i("Init done");
}

// From https://github.com/Seeed-Studio/Seeed_Arduino_RoundDisplay
bool chsc6x_is_pressed(void)
{
    if (digitalRead(TOUCH_INT) != LOW)
    {
        delay(1);
        if (digitalRead(TOUCH_INT) != LOW)
        {
            return false;
        }
    }
    return true;
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
