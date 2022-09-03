#ifndef DEF_DEFS_H
#define DEF_DEFS_H

#include "Adafruit_GFX.h"

#define defaultMaxWait 100

#define Color8(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))
#define Color16(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

#define BLACK8 Color8(0x00, 0x00, 0x00)
#define WHITE8 Color8(0xFF, 0xFF, 0xFF)
#define GREEN8 Color8(0x00, 0xFF, 0x00)
#define LIGHTRED8 Color8(0xFF, 0x40, 0x40)
#define RED8 Color8(0xFF, 0x00, 0x00)
#define BLUE8 Color8(0x00, 0x00, 0xFF)
#define LIGHTBLUE8 Color8(0x00, 0x40, 0xFF)
#define DARKORANGE8 Color8(0xFF, 0x40, 0x00)
#define ORANGE8 Color8(0xFF, 0x80, 0x00)
#define YELLOW8 Color8(0xFF, 0xFF, 0x00)
#define CYAN8 Color8(0x00, 0xFF, 0xFF)
#define MAGENTA8 Color8(0xFF, 0x00, 0xFF)

#define DARK_GRAY8 0b01101101

#define WHITE16 0xFFFF

constexpr int16_t NUM_TIRES = 6;

constexpr int16_t display_width = 800;
constexpr int16_t display_height = 480;

constexpr int16_t cellWidth = 330;
constexpr int16_t cellHeight = 72;

constexpr uint16_t tire_top_y = 300;

extern void drawPNG(const unsigned char* data, uint32_t length, Adafruit_GFX* dest, int16_t x, int16_t y);
extern void getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height);
extern uint16_t getStringWidth(Adafruit_GFX& dest, String str);
extern void writePrefs();

extern void sleepUntilTouch();
extern float sequenceInterp(float* values, int16_t count, float percent);

extern bool switchState();
extern float voltageValue();
extern bool systemUpdate();

#endif
