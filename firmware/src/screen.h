#include <SPI.h>
#include <TFT_eSPI.h>
#include <PNGdec.h>
#include "common.h"

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

#define ICON_W 32
#define SPRITE_X 40
#define SPRITE_Y 40
#define SPRITE_W 160
#define SPRITE_H 160

#define ANTIALIAS_COLOR 0x1006

TFT_eSPI tft = TFT_eSPI(SCREEN_WIDTH, SCREEN_HEIGHT);
TFT_eSprite spr = TFT_eSprite(&tft);

uint8_t brightness = 0;
uint32_t nextStartDim = 0;
uint32_t nextStepDim = 0;

PNG png;
File pngfile;

static const char *TAG_BRIGHTNESS = "BRIGHT";
static const char *TAG_SCREEN = "SCREEN";

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
        ESP_LOGI(TAG_BRIGHTNESS, "Restart");
    }
    else if (nextStartDim != 0)
    {
        // on dim timer end: start step timer
        if (nextStartDim < now)
        {
            nextStartDim = 0;
            nextStepDim = now;
            ESP_LOGI(TAG_BRIGHTNESS, "Start dim");
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
                ESP_LOGI(TAG_BRIGHTNESS, "End dim");
            }
        }
    }
    else if (!is_always_on)
    {
        // if away: set 0%
        if (brightness > 0)
        {
            set_brightness(0);
            ESP_LOGI(TAG_BRIGHTNESS, "Off");
        }
    }
    else
    {
        // if at home: set min brightness
        if (brightness == 0)
        {
            set_brightness(MIN_BRIGHTNESS);
            ESP_LOGI(TAG_BRIGHTNESS, "On");
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
    tft.fillScreen(TFT_BLACK);

    if (spr.createSprite(SPRITE_W, SPRITE_H) == nullptr)
    {
        log_e("cannot create sprite");
    }

    set_brightness(100);
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
        uint16_t lineBuffer[SCREEN_WIDTH];
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
            uint16_t lineBuffer[SCREEN_WIDTH];
            png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
            spr.pushImage(-SPRITE_X, pDraw->y - SPRITE_Y, pDraw->iWidth, 1, lineBuffer);
        }
    };

    png.open("/bg.png", pngOpen, pngClose, pngRead, pngSeek, draw);
    png.decode(NULL, 0);
    png.close();
}

typedef struct
{
    uint8_t x, y;
} COORDS;

// note: pushMaskedImage cannot be used on sprite
void draw_icon(const String &name, uint8_t x, uint8_t y)
{
    String path = "/" + name + ".png";
    COORDS coords = {x, y};

    auto draw = [](PNGDRAW *pDraw)
    {
        COORDS *coords = (COORDS *)pDraw->pUser;
        uint16_t lineBuffer[ICON_W];
        uint8_t maskBuffer[1 + ICON_W / 8];
        png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
        if (png.getAlphaMask(pDraw, maskBuffer, 255))
        {
            tft.pushMaskedImage(coords->x - ICON_W / 2, coords->y + pDraw->y - ICON_W / 2, pDraw->iWidth, 1, lineBuffer, maskBuffer);
        }
    };

    png.open(path.c_str(), pngOpen, pngClose, pngRead, pngSeek, draw);
    png.decode((void *)&coords, 0);
    png.close();
}

void draw_screen(const Data &data)
{
    ESP_LOGI(TAG_SCREEN, "Start update");

    init_sprite();

    static const int xTemp = 200 - SPRITE_X;
    static const int yTemp1 = 68 - SPRITE_Y;
    static const int yTemp2 = 180 - SPRITE_Y;

    static const int xHumi = 150 - SPRITE_X;
    static const int yHumi1 = 40 - SPRITE_Y;
    static const int yHumi2 = 205 - SPRITE_Y;

    static const int xPower = 0;
    static const int yPower = SPRITE_H / 2.0 + 2;

    static const int xIcon = 165;
    static const int yIcon = 120;

    spr.loadFont("RobotoMono-Bold-40", LittleFS);
    spr.setTextColor(TFT_WHITE, ANTIALIAS_COLOR);

    spr.setTextDatum(TR_DATUM);
    spr.drawString(data.temp1, xTemp, yTemp1);

    spr.setTextDatum(BR_DATUM);
    spr.drawString(data.temp2, xTemp, yTemp2);

    spr.loadFont("RobotoMono-Bold-30", LittleFS);
    spr.setTextColor(TFT_LIGHTGREY, ANTIALIAS_COLOR);

    spr.setTextDatum(TR_DATUM);
    spr.drawString(data.humi1, xHumi, yHumi1);

    spr.setTextDatum(BR_DATUM);
    spr.drawString(data.humi2, xHumi, yHumi2);

    spr.setTextColor(TFT_DARKCYAN, ANTIALIAS_COLOR);
    spr.setTextDatum(CL_DATUM);
    spr.drawString(data.power, xPower, yPower);

    spr.unloadFont();

    spr.pushSprite(SPRITE_X, SPRITE_Y);

    if (data.mode1 == "heat_cool" || data.mode1 == "heat" || data.mode1 == "cool" || data.mode1 == "off")
    {
        draw_icon(data.mode1, xIcon - 16, yIcon);
    }

    if (data.mode2 == "heat_cool" || data.mode2 == "heat" || data.mode2 == "cool" || data.mode2 == "off")
    {
        draw_icon(data.mode2, xIcon + 16, yIcon);
    }

    ESP_LOGI(TAG_SCREEN, "Screen updated");
}
