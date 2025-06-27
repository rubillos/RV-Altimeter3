#pragma once

#include "Arduino.h"
#include <PNGdec.h>
#include "Adafruit_RA8875.h"
#include "defs.h"

void drawPNG(const unsigned char* data, uint32_t length, Adafruit_GFX* dest, int16_t x, int16_t y);
void drawPNG83(const unsigned char* data, uint32_t length, Adafruit_GFX* dest, int16_t x, int16_t y, uint8_t whiteColor, uint8_t blackColor=0, uint8_t midColor=DARK_GRAY8);
void drawPNG8(const unsigned char* data, uint32_t length, Adafruit_RA8875* dest, int16_t x, int16_t y, bool midGray=false);
