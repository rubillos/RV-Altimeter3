#ifndef DEF_DATADISPLAY_H
#define DEF_DATADISPLAY_H

#include "Adafruit_GFX.h"

bool showData(uint16_t* drawIndex, uint32_t time, int16_t altitude, float heading, float speed, uint32_t sunriseTime, uint32_t sunsetTime, uint16_t satCount, bool haveFix, String status);
String suffixFromDayMinutes(double dayMinutes);
String timeFromDayMinutes(double dayMinutes, bool includeSeconds);
uint8_t ascenderForFont(const GFXfont *f, char character = 'A');
void getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height);
uint16_t getStringWidth(Adafruit_GFX& dest, String str);

#endif

