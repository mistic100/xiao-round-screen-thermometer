#include <SPI.h>
#define USE_TFT_ESPI_LIBRARY
#include <lv_xiao_round_screen.h>
#include <PNGdec.h>
#include "common.h"

#define SCREEN_W 240
#define SPRITE_X 40
#define SPRITE_Y 40
#define SPRITE_W 160
#define SPRITE_H 160

uint8_t brightness = 0;
uint32_t nextStartDim = 0;
uint32_t nextStepDim = 0;

PNG png;
File pngfile;
TFT_eSprite spr = TFT_eSprite(&tft);

void set_brightness(uint8_t b)
{
    if (b != brightness)
    {
        analogWrite(TFT_BL, b * 2.55);
        brightness = b;
    }
}

void update_brightness(bool is_always_on, bool is_touch)
{
    uint32_t now = millis();

    if (is_touch)
    {
        // on touch: set 100% & start dim timer
        set_brightness(100);
        nextStartDim = now + DIM_TIMEOUT_MS;
    }
    else if (nextStartDim != 0)
    {
        // on dim timer end: start step timer
        if (nextStartDim < now)
        {
            nextStartDim = 0;
            nextStepDim = now;
        }
    }
    else if (nextStepDim != 0)
    {
        // on step timer end
        if (nextStepDim < now)
        {
            // > min brightness: set -1 & restart step timer
            if (brightness > (is_always_on ? MIN_BRIGHTNESS : 0))
            {
                set_brightness(brightness - 1);
                nextStepDim = now + 20;
            }
            // stop step timer
            else
            {
                nextStepDim = 0;
            }
        }
    }
    else if (!is_always_on)
    {
        // if away: set 0%
        if (brightness > 0)
        {
            set_brightness(0);
        }
    }
    else
    {
        // if at home: set min brightness
        if (brightness == 0)
        {
            set_brightness(MIN_BRIGHTNESS);
        }
    }
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
    tft.drawString(string, tft.width() / 2, tft.height() / 2);

    isblinked = !isblinked;
}

void init_screen()
{
    tft.init();
    tft.setRotation(3);

    set_brightness(100);

    if (spr.createSprite(SPRITE_W, SPRITE_H) == nullptr)
    {
        log_e("cannot create sprite");
    }

    tft.fillScreen(TFT_BLACK);

    nextStartDim = millis() + DIM_TIMEOUT_MS;
}

void *pngOpen(const char *filename, int32_t *size)
{
    pngfile = LittleFS.open(filename, "r");
    *size = pngfile.size();
    return &pngfile;
}

void pngClose(void *handle)
{
    File pngfile = *((File *)handle);
    if (pngfile)
    {
        pngfile.close();
    }
}

int32_t pngRead(PNGFILE *page, uint8_t *buffer, int32_t length)
{
    if (!pngfile)
    {
        return 0;
    }
    return pngfile.read(buffer, length);
}

int32_t pngSeek(PNGFILE *page, int32_t position)
{
    if (!pngfile)
    {
        return 0;
    }
    return pngfile.seek(position);
}

void draw_bg()
{
    auto draw = [](PNGDRAW *pDraw)
    {
        uint16_t lineBuffer[SCREEN_W];
        png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
        tft.pushImage(0, 0 + pDraw->y, pDraw->iWidth, 1, lineBuffer);
    };

    png.open("/bg.png", pngOpen, pngClose, pngRead, pngSeek, draw);
    tft.startWrite();
    png.decode(NULL, 0);
    png.close();
    tft.endWrite();
}

void init_sprite()
{
    auto draw = [](PNGDRAW *pDraw)
    {
        if (pDraw->y >= SPRITE_Y && pDraw->y < SPRITE_Y + SPRITE_H)
        {
            uint16_t lineBuffer[SCREEN_W];
            png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
            spr.pushImage(-SPRITE_X, pDraw->y - SPRITE_Y, pDraw->iWidth, 1, lineBuffer);
        }
    };

    png.open("/bg.png", pngOpen, pngClose, pngRead, pngSeek, draw);
    png.decode(NULL, 0);
    png.close();
}

void draw_screen(const Data &data)
{
    init_sprite();

    int xTemp = 200 - SPRITE_X;
    int yTemp1 = 115 - SPRITE_Y;
    int yTemp2 = 130 - SPRITE_Y;

    int xHumi = 150 - SPRITE_X;
    int yHumi1 = 70 - SPRITE_Y;
    int yHumi2 = 175 - SPRITE_Y;

    spr.loadFont("RobotoMono-Bold-40", LittleFS);
    spr.setTextColor(TFT_WHITE, TFT_NAVY);

    spr.setTextDatum(BR_DATUM);
    spr.drawString(data.tempSejour, xTemp, yTemp1);

    spr.setTextDatum(TR_DATUM);
    spr.drawString(data.tempExt, xTemp, yTemp2);

    spr.loadFont("RobotoMono-Bold-30", LittleFS);
    spr.setTextColor(TFT_LIGHTGREY, TFT_NAVY);

    spr.setTextDatum(BR_DATUM);
    spr.drawString(data.humiSejour, xHumi, yHumi1);

    spr.setTextDatum(TR_DATUM);
    spr.drawString(data.humiExt, xHumi, yHumi2);

    spr.unloadFont();

    spr.pushSprite(SPRITE_X, SPRITE_Y);
}
