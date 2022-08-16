#ifndef DEF_BUFFER8_H
#define DEF_BUFFER8_H

#include <Adafruit_GFX.h>     // Core graphics library
#include "Adafruit_RA8875.h"

class Buffer8 : public GFXcanvas8 {
public:
  Buffer8(uint16_t x, uint16_t y, uint16_t w, uint16_t h) : GFXcanvas8(w, h) {
    _x = x;
    _y = y;
  }
  void setOffset(uint16_t x, uint16_t y, uint16_t c=0) {
    _x = x;
    _y = y;
    fillScreen(c);
  }
  void drawPixel(int16_t x, int16_t y, uint16_t color) {
    GFXcanvas8::drawPixel(x-_x, y-_y, color);
  }
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    GFXcanvas8::drawFastHLine(x-_x, y-_y, w, color);
  }
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    GFXcanvas8::drawFastVLine(x-_x, y-_y, h, color);
  }
  void draw(Adafruit_RA8875& dest, int16_t w=-1, int16_t h=-1) {
    uint8_t* srcBuff = getBuffer();
    if (w==-1) { w=width(); }
    if (h==-1) { h=height(); }

    uint16_t bufferWidth = width();

    for (uint16_t y=0; y<h; y++) {
      dest.drawPixels8(srcBuff+y*bufferWidth, w, _x, _y+y);
    }
  }
private:
  uint16_t _x;
  uint16_t _y;
};

#endif
