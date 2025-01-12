#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "common.h"

WiFiClient client;
HTTPClient http;
JsonDocument doc;

uint32_t nextUpdate = 0;

static const char* TAG = "SENSORS";

void init_sensors()
{
}

bool update_sensors(Data &data)
{
    if (nextUpdate < millis())
    {
        ESP_LOGI(TAG, "Update");

        http.useHTTP10(true);
        http.begin(client, HA_URL);
        http.addHeader("Authorization", HA_TOKEN);
        http.GET();

        doc.clear();
        deserializeJson(doc, http.getStream());

        auto time = doc["attributes"]["time"].as<String>();
        data.time = time.substring(0, 2).toInt() * 60 + time.substring(3, 5).toInt();
        data.atHome = doc["attributes"]["at_home"].as<bool>();
        data.tempSejour = doc["attributes"]["temp_sejour"].as<String>();
        data.tempExt = doc["attributes"]["temp_ext"].as<String>();
        data.humiSejour = doc["attributes"]["humi_sejour"].as<String>();
        data.humiExt = doc["attributes"]["humi_ext"].as<String>();
        data.modeSalon = doc["attributes"]["mode_salon"].as<String>();
        auto modeSejour = doc["attributes"]["mode_sejour"].as<String>();
        data.modeSejour = modeSejour == "heating" ? "heat" : "off";

        http.end();

        ESP_LOGI(TAG, "Time: %d mins", data.time);
        ESP_LOGI(TAG, "At home: %d", data.atHome);
        ESP_LOGI(TAG, "Séjour: %s %s", data.tempSejour.c_str(), data.humiSejour.c_str());
        ESP_LOGI(TAG, "Ext: %s %s", data.tempExt.c_str(), data.humiExt.c_str());
        ESP_LOGI(TAG, "Mode salon: %s", data.modeSalon);
        ESP_LOGI(TAG, "Mode séjour: %s", data.modeSejour);

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
