#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "common.h"

WiFiClient client;
HTTPClient http;
JsonDocument doc;

uint32_t nextUpdate = 0;

static const char *TAG = "SENSORS";

void init_sensors()
{
}

bool update_sensors(Data &data)
{
    if (nextUpdate < millis())
    {
        ESP_LOGI(TAG, "Update");

        http.useHTTP10(true);
        http.begin(client, HA_URL + HA_SENSOR);
        http.addHeader("Authorization", HA_TOKEN);
        http.GET();

        doc.clear();
        deserializeJson(doc, http.getStream());

        auto time = doc["attributes"]["time"].as<String>();
        data.time = time.substring(0, 2).toInt() * 60 + time.substring(3, 5).toInt();
        data.atHome = doc["attributes"]["at_home"].as<bool>();
        data.power = doc["attributes"]["power"].as<String>();
        data.temp1 = doc["attributes"]["temp_1"].as<String>();
        data.temp2 = doc["attributes"]["temp_2"].as<String>();
        data.humi1 = doc["attributes"]["humi_1"].as<String>();
        data.humi2 = doc["attributes"]["humi_2"].as<String>();
        data.mode1 = doc["attributes"]["mode_1"].as<String>();
        data.mode2 = doc["attributes"]["mode_2"].as<String>();

        http.end();

        ESP_LOGI(TAG, "Time: %d mins", data.time);
        ESP_LOGI(TAG, "At home: %d", data.atHome);
        ESP_LOGI(TAG, "Zone 1: %s %s %s", data.temp1.c_str(), data.humi1.c_str(), data.mode1);
        ESP_LOGI(TAG, "Zone 2: %s %s %s", data.temp2.c_str(), data.humi2.c_str(), data.mode2);
        ESP_LOGI(TAG, "Power: %s", data.power);

        nextUpdate = millis() + UPDATE_INTERVAL_MS;

        return true;
    }

    return false;
}

bool is_always_on(const Data &data)
{
    if (!data.atHome)
    {
        return false;
    }
    else
    {
        return data.time >= TIME_START && data.time < TIME_END;
    }
}
