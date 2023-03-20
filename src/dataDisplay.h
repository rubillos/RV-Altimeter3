#ifndef DEF_DATADISPLAY_H
#define DEF_DATADISPLAY_H

#include "Adafruit_GFX.h"
#include "graph.h"
#include "defs.h"
#include "gps.h"

extern const GFXfont* textFont;
extern const GFXfont* suffixFont;
extern const GFXfont* glyphFont;

class DataDisplay {
	public:
		void begin();
		bool showData(uint16_t* drawIndex, uint32_t time, GPSData* gpsData, uint32_t sunriseTime, uint32_t sunsetTime);
		void showCell(Adafruit_GFX& dest, int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, String suffix, const GFXfont* strFont = textFont, const GFXfont* sufFont = suffixFont);
		String suffixFromDayMinutes(double dayMinutes);
		String timeFromDayMinutes(double dayMinutes, bool includeSeconds);
		uint8_t ascenderForFont(const GFXfont *f, char character = 'A');
		void getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height);
		uint16_t getStringWidth(Adafruit_GFX& dest, String str);
		uint16_t getStringGlyphWidth(const GFXfont *f, String str);
		void drawThickLine(Adafruit_GFX& dest, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color, bool fast=false);
		void drawPolarLine(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t length, int16_t thickness, uint8_t color=WHITE8);
		void drawTime(Adafruit_GFX& dest, uint16_t x, uint16_t y, uint16_t hours, uint16_t minutes, uint8_t color=WHITE8);
		void drawPointer(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t majorLen, uint16_t minorLen, uint16_t cornerAngle, uint8_t color=WHITE8);

		void toggleDrawGraphs() { _drawGraphs = !_drawGraphs; };
		void setDrawGraphs(bool drawGraphs) { _drawGraphs = drawGraphs; };

		void drawInAltLayer();
		void drawInCurrentLayer();
		void switchToAltLayer();
		
	private:
		bool _drawLayer = false;
		bool _drawGraphs = false;
		DataGraph* _speedGraph;
		DataGraph* _altitudeGraph;
};

extern DataDisplay _dataDisplay;

#endif

