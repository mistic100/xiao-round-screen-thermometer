#include <SPI.h>
#define USE_TFT_ESPI_LIBRARY
#include <lv_xiao_round_screen.h>
#include "RawImage.h"
#include "RobotoMonoBold16.h"
#include "RobotoMonoBold18.h"
#include "RobotoMonoBold24.h"
#include "RobotoMonoBold30.h"

#include "common.h"

uint8_t brightness = 0;
uint32_t nextStartDim = 0;
uint32_t nextStepDim = 0;

void set_brightness(uint8_t b)
{
    if (b != brightness)
    {
        analogWrite(TFT_BL, b * 2.55);
        brightness = b;
    }
}

void update_brightness(bool is_touch)
{
    uint32_t now = millis();

    if (is_touch)
    {
        set_brightness(100);
        nextStartDim = now + DIM_TIMEOUT_MS;
    }
    else if (nextStartDim != 0 && nextStartDim < now)
    {
        nextStartDim = 0;
        nextStepDim = now;
    }
    else if (nextStepDim != 0 && nextStepDim < now)
    {
        if (brightness > MIN_BRIGHTNESS)
        {
            set_brightness(brightness - 1);
            nextStepDim = now + 20;
        }
        else
        {
            nextStepDim = 0;
        }
    }
}

void init_screen()
{
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);

    set_brightness(100);
}

void draw_bg()
{
    drawImage<uint8_t>("/bg.bmp", 0, 0);

    nextStartDim = millis() + DIM_TIMEOUT_MS;
}

void display_error(const char *string)
{
    static bool isblinked = false;

    tft.setTextDatum(MC_DATUM);

    if (isblinked)
    {
        tft.setTextColor(TFT_RED, TFT_BLACK);
    }
    else
    {
        tft.setTextColor(TFT_BLACK, TFT_RED);
    }
    tft.drawString("INSERT SD", tft.width() / 2, tft.height() / 2);

    isblinked = !isblinked;
}

void draw_screen(const Data &data)
{
    int xTemp = 150;
    int yTemp1 = 115;
    int yTemp2 = 240 - yTemp1;

    int xHumi = 105;
    int yHumi1 = 65;
    int yHumi2 = 240 - yHumi1;

    tft.setFreeFont(&RobotoMono_Bold24pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK, true);

    tft.setTextDatum(BR_DATUM);
    tft.drawString(data.tempSejour, xTemp, yTemp1);

    tft.setTextDatum(TR_DATUM);
    tft.drawString(data.tempExt, xTemp, yTemp2);

    tft.setFreeFont(&RobotoMono_Bold16pt7b);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK, true);

    tft.setTextDatum(BR_DATUM);
    tft.drawString(data.humiSejour, xHumi, yHumi1);

    tft.setTextDatum(TR_DATUM);
    tft.drawString(data.humiExt, xHumi, yHumi2);
}
