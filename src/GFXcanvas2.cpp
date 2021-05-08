#include "GFXcanvas2.h"

/**************************************************************************/
/*!
   @brief    Instatiate a GFX 2-bit canvas context for graphics
   @param    w   Display width, in pixels
   @param    h   Display height, in pixels
*/
/**************************************************************************/
GFXcanvas2::GFXcanvas2(uint16_t w, uint16_t h) : Adafruit_GFX(w, h)
{
    uint16_t bytes = ((w + 3) / 4) * h;
    if ((buffer = (uint8_t *)malloc(bytes)))
    {
        memset(buffer, 0, bytes);
    }
}

/**************************************************************************/
/*!
   @brief    Delete the canvas, free memory
*/
/**************************************************************************/
GFXcanvas2::~GFXcanvas2(void)
{
    if (buffer)
        free(buffer);
}

/**************************************************************************/
/*!
    @brief  Draw a pixel to the canvas framebuffer
    @param  x     x coordinate
    @param  y     y coordinate
    @param  color Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (buffer)
    {
        if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
            return;

        int16_t t;
        switch (rotation)
        {
        case 1:
            t = x;
            x = WIDTH - 1 - y;
            y = t;
            break;
        case 2:
            x = WIDTH - 1 - x;
            y = HEIGHT - 1 - y;
            break;
        case 3:
            t = x;
            x = y;
            y = HEIGHT - 1 - t;
            break;
        }

        uint8_t *ptr = &buffer[(x / 4) + y * ((WIDTH + 3) / 4)];
        uint8_t shift = x & 3;
        static uint8_t masks[] = { 0x03, 0x0C, 0x30, 0xC0 };

        *ptr = (*ptr & masks[shift]) | (color << shift * 2);
    }
}

/**********************************************************************/
/*!
        @brief    Get the pixel color value at a given coordinate
        @param    x   x coordinate
        @param    y   y coordinate
        @returns  The desired pixel's binary color value, either 0x1 (on) or 0x0
   (off)
*/
/**********************************************************************/
uint8_t GFXcanvas2::getPixel(int16_t x, int16_t y) const
{
    int16_t t;
    switch (rotation)
    {
    case 1:
        t = x;
        x = WIDTH - 1 - y;
        y = t;
        break;
    case 2:
        x = WIDTH - 1 - x;
        y = HEIGHT - 1 - y;
        break;
    case 3:
        t = x;
        x = y;
        y = HEIGHT - 1 - t;
        break;
    }
    return getRawPixel(x, y);
}

/**********************************************************************/
/*!
        @brief    Get the pixel color value at a given, unrotated coordinate.
              This method is intended for hardware drivers to get pixel value
              in physical coordinates.
        @param    x   x coordinate
        @param    y   y coordinate
        @returns  The desired pixel's binary color value, either 0x1 (on) or 0x0
   (off)
*/
/**********************************************************************/
uint8_t GFXcanvas2::getRawPixel(int16_t x, int16_t y) const
{
    if ((x < 0) || (y < 0) || (x >= WIDTH) || (y >= HEIGHT))
        return 0;

    if (this->getBuffer())
    {
        uint8_t *buffer = this->getBuffer();
        uint8_t *ptr = &buffer[(x / 4) + y * ((WIDTH + 3) / 4)];
        uint8_t shift = x & 3;

        return ((*ptr) >> (shift * 2)) & 0x03;
    }

  return 0;
}

/**************************************************************************/
/*!
    @brief  Fill the framebuffer completely with one color
    @param  color Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::fillScreen(uint16_t color)
{
    if (buffer)
    {
        uint8_t fills[] = { 0x00, 0x55, 0xAA, 0xFF };
        uint16_t bytes = ((WIDTH + 3) / 4) * HEIGHT;
        memset(buffer, fills[color], bytes);
    }
}

/**************************************************************************/
/*!
   @brief    Speed optimized vertical line drawing
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    h   length of vertical line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::drawFastVLine(int16_t x, int16_t y, int16_t h,
                               uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    {
        return;
    }

    if (y + h > height())
    {
        h = height() - y;
    } else if (h < 0) {
    // convert negative heights to their postive equivalent
    h *= -1;
    y -= h - 1;
    if (y < 0) {
      h += y;
      y = 0;
    }
  }

  if (getRotation() == 0) {
    drawFastRawVLine(x, y, h, color);
  } else if (getRotation() == 1) {
    int16_t t = x;
    x = WIDTH - 1 - y;
    y = t;
    x -= h - 1;
    drawFastRawHLine(x, y, h, color);
  } else if (getRotation() == 2) {
    x = WIDTH - 1 - x;
    y = HEIGHT - 1 - y;

    y -= h - 1;
    drawFastRawVLine(x, y, h, color);
  } else if (getRotation() == 3) {
    int16_t t = x;
    x = y;
    y = HEIGHT - 1 - t;
    drawFastRawHLine(x, y, h, color);
  }
}

/**************************************************************************/
/*!
   @brief    Speed optimized horizontal line drawing
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    w   length of horizontal line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::drawFastHLine(int16_t x, int16_t y, int16_t w,
                               uint16_t color)
{
    if ((x < 0) || (x >= width()) || (y < 0) || (y >= height()))
    {
        return;
    }

    if (x + w > width())
    {
        w = width() - x;
    } else if (w < 0) {
    // convert negative widths to their postive equivalent
    w *= -1;
    x -= w - 1;
    if (x < 0) {
      w += x;
      x = 0;
    }
  }

  if (getRotation() == 0) {
    drawFastRawHLine(x, y, w, color);
  } else if (getRotation() == 1) {
    int16_t t = x;
    x = WIDTH - 1 - y;
    y = t;
    drawFastRawVLine(x, y, w, color);
  } else if (getRotation() == 2) {
    x = WIDTH - 1 - x;
    y = HEIGHT - 1 - y;

    x -= w - 1;
    drawFastRawHLine(x, y, w, color);
  } else if (getRotation() == 3) {
    int16_t t = x;
    x = y;
    y = HEIGHT - 1 - t;
    y -= w - 1;
    drawFastRawVLine(x, y, w, color);
  }
}

/**************************************************************************/
/*!
   @brief    Speed optimized vertical line drawing into the raw canvas buffer
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    h   length of vertical line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::drawFastRawVLine(int16_t x, int16_t y, int16_t h,
                                  uint16_t color)
{
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    int16_t row_bytes = ((WIDTH + 3) / 4);
    uint8_t *buffer = this->getBuffer();
    uint8_t *ptr = &buffer[(x / 4) + y * row_bytes];
    uint8_t shift = x & 3;
    static uint8_t masks[] = { 0x03, 0x0C, 0x30, 0xC0 };
    uint8_t mask = masks[shift];

    color <<= shift * 2;

    for (int16_t i = 0; i < h; i++) {
        *ptr = ((*ptr) & mask) | color;
        ptr += row_bytes;
    }
}

/**************************************************************************/
/*!
   @brief    Speed optimized horizontal line drawing into the raw canvas buffer
   @param    x   Line horizontal start point
   @param    y   Line vertical start point
   @param    w   length of horizontal line to be drawn, including first point
   @param    color   Binary (on or off) color to fill with
*/
/**************************************************************************/
void GFXcanvas2::drawFastRawHLine(int16_t x, int16_t y, int16_t w,
                                  uint16_t color)
{
    // x & y already in raw (rotation 0) coordinates, no need to transform.
    int16_t rowBytes = ((WIDTH + 3) / 4);
    uint8_t *buffer = this->getBuffer();
    uint8_t *ptr = &buffer[(x / 4) + y * rowBytes];
    size_t remainingWidthBits = w;

    // check to see if first byte needs to be partially filled
    if ((x & 7) > 0)
    {
        // create bit mask for first byte
        uint8_t startByteBitMask = 0x00;
        for (int8_t i = (x & 3); ((i < 8) && (remainingWidthBits > 0)); i++)
        {
      startByteBitMask |= (0x80 >> i);
      remainingWidthBits--;
    }
    if (color > 0) {
      *ptr |= startByteBitMask;
    } else {
      *ptr &= ~startByteBitMask;
    }

    ptr++;
  }

  // do the next remainingWidthBits bits
  if (remainingWidthBits > 0) {
    size_t remainingWholeBytes = remainingWidthBits / 8;
    size_t lastByteBits = remainingWidthBits % 8;
    uint8_t wholeByteColor = color > 0 ? 0xFF : 0x00;

    memset(ptr, wholeByteColor, remainingWholeBytes);

    if (lastByteBits > 0) {
      uint8_t lastByteBitMask = 0x00;
      for (uint8_t i = 0; i < lastByteBits; i++) {
        lastByteBitMask |= (0x80 >> i);
      }
      ptr += remainingWholeBytes;

      if (color > 0) {
        *ptr |= lastByteBitMask;
      } else {
        *ptr &= ~lastByteBitMask;
      }
    }
  }
}

