#include "TextFactory.h"

TextFactory::TextFactory()
{
    workCanvas = new GFXcanvas1(256, 100);
}

TextFactory::~TextFactory(void)
{

}

GFXcanvas2 *TextFactory::smoothCanvas(GFXcanvas1 *srcCanvas)
{
    GFXcanvas2 *destCanvas;
    uint16_t smallWidth = (srcCanvas->width() + 1) / 2;
    uint16_t smallHeight = (srcCanvas->height() + 1) / 2;

    destCanvas = new GFXcanvas2(smallWidth, smallHeight);

    uint8_t grayLookup[] = {0x00, 0x01, 0x01, 0x02, 0x03};

    for (uint16_t y = 0; y < smallHeight; y++)
    {
        for (uint16_t x = 0; x < smallWidth; x++)
        {
            uint8_t count = 0;

            count += srcCanvas->getPixel(x * 2, y * 2);
            count += srcCanvas->getPixel(x * 2 + 1, y * 2);
            count += srcCanvas->getPixel(x * 2, y * 2 + 1);
            count += srcCanvas->getPixel(x * 2 + 1, y * 2 + 1);

            destCanvas->drawPixel(x, y, grayLookup[count]);
        }
    }

    return destCanvas;
}

uint8_t TextFactory::ascenderForFont(const GFXfont *f)
{
    uint16_t index = 'A' - f->first;
    int8_t offset = f->glyph[index].yOffset;

    return -offset;
}

uint8_t TextFactory::xAdvanceForChar(char ch, const GFXfont *f)
{
    uint16_t index = ch - f->first;
    uint8_t offset = f->glyph[index].xAdvance;

    return offset;
}

GFXcanvas1* TextFactory::makeString(const char *str, const GFXfont *f)
{
    GFXcanvas1 *newChar;
    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;

    workCanvas->setFont(f);
    workCanvas->setTextSize(1);
    workCanvas->setTextWrap(false);
    workCanvas->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

    newChar = new GFXcanvas1(w + 1, f->yAdvance);
    newChar->setTextWrap(false);
    newChar->fillScreen(0);
    newChar->setTextColor(0XFFFF);
    newChar->setFont(f);
    newChar->setTextSize(1);
    newChar->setCursor(-x1, ascenderForFont(f));
    newChar->print(str);

    // oled.println(String(str) + String(": w=") + String(w) + String(", x1=") + String(x1) + String(", adv=") + String(xAdvanceForChar(str[0], &FreeSansBold24pt7b)) );

    return newChar;
}

GFXcanvas1* TextFactory::makeCharacter(char ch, const GFXfont *f)
{
    char str[2];

    str[0] = ch;
    str[1] = 0;

    return makeString(str, f);
}

GFXcanvas2* TextFactory::makeStringSmooth(const char *str, const GFXfont *f, int16_t offsetGlyphs, int16_t forceHeight, int16_t forceWidth)
{
    GFXcanvas1 *newChar;
    int16_t x1;
    int16_t y1;
    uint16_t w;
    uint16_t h;

    workCanvas->setFont(f);
    workCanvas->setTextSize(1);
    workCanvas->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

    uint16_t xOffset = 0;

    if (forceWidth > 0)
    {
        xOffset = (forceWidth - w) / 2;
        w = forceWidth;
    }

    newChar = new GFXcanvas1(w, (forceHeight == -1) ? f->yAdvance : forceHeight);
    newChar->setTextWrap(false);
    newChar->fillScreen(0);
    newChar->setTextColor(0XFFFF);
    newChar->setFont(f);
    newChar->setTextSize(1);
    newChar->setCursor(xOffset - x1, ascenderForFont(f) + offsetGlyphs);
    newChar->print(str);

    return smoothCanvas(newChar);
}

GFXcanvas2* TextFactory::makeCharacterSmooth(char ch, const GFXfont *f, int16_t offsetGlyphs, int16_t forceHeight, int16_t forceWidth)
{
    char str[2];

    str[0] = ch;
    str[1] = 0;

    return makeStringSmooth(str, f, offsetGlyphs, forceHeight, forceWidth);
}
