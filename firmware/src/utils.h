#include <PNGdec.h>
#include <TFT_eSPI.h>
#include "common.h"

File pngfile;

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

/**
 * Version of `TFT_eSPI::pushMaskedImage` that can be called on a sprite (same code without SPI transaction)
 */
void pushMaskedImageToSprite(TFT_eSprite *spr, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask)
{
    uint8_t *mptr = mask;
    uint8_t *eptr = mask + ((w + 7) >> 3);
    uint16_t *iptr = img;
    uint32_t setCount = 0;

    // For each line in the image
    while (h--)
    {
        uint32_t xp = 0;
        uint32_t clearCount = 0;
        uint8_t mbyte = *mptr++;
        uint32_t bits = 8;
        // Scan through each byte of the bitmap and determine run lengths
        do
        {
            setCount = 0;

            // Get run length for clear bits to determine x offset
            while ((mbyte & 0x80) == 0x00)
            {
                // Check if remaining bits in byte are clear (reduce shifts)
                if (mbyte == 0)
                {
                    clearCount += bits; // bits not always 8 here
                    if (mptr >= eptr)
                        break; // end of line
                    mbyte = *mptr++;
                    bits = 8;
                    continue;
                }
                mbyte = mbyte << 1; // 0's shifted in
                clearCount++;
                if (--bits)
                    continue;
                ;
                if (mptr >= eptr)
                    break;
                mbyte = *mptr++;
                bits = 8;
            }

            // Get run length for set bits to determine render width
            while ((mbyte & 0x80) == 0x80)
            {
                // Check if all bits are set (reduces shifts)
                if (mbyte == 0xFF)
                {
                    setCount += bits;
                    if (mptr >= eptr)
                        break;
                    mbyte = *mptr++;
                    // bits  = 8; // NR, bits always 8 here unless 1's shifted in
                    continue;
                }
                mbyte = mbyte << 1; // or mbyte += mbyte + 1 to shift in 1's
                setCount++;
                if (--bits)
                    continue;
                if (mptr >= eptr)
                    break;
                mbyte = *mptr++;
                bits = 8;
            }

            // A mask boundary or mask end has been found, so render the pixel line
            if (setCount)
            {
                xp += clearCount;
                clearCount = 0;
                spr->pushImage(x + xp, y, setCount, 1, iptr + xp); // pushImage handles clipping
                if (mptr >= eptr)
                    break;
                xp += setCount;
            }
        } while (setCount || mptr < eptr);

        y++;
        iptr += w;
        eptr += ((w + 7) >> 3);
    }
}
