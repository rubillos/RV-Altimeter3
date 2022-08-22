#ifndef DEF_PNG_H
#define DEF_PNG_H

#include "Arduino.h"
#include <PNGdec.h>
#include "Adafruit_RA8875.h"

void drawPNG(const unsigned char* data, uint32_t length, Adafruit_GFX* dest, int16_t x, int16_t y);
void drawPNG8(const unsigned char* data, uint32_t length, Adafruit_RA8875* dest, int16_t x, int16_t y, bool midGray=false);

#endif
