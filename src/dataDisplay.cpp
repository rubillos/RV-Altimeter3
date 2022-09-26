#include "dataDisplay.h"

#include "defs.h"
#include "Adafruit_RA8875.h"
#include "touchscreen.h"
#include "Buffer8.h"
#include "tires.h"
#include "gps.h"
#include "menu.h"

#include "fonts/FreeSans44pt7b.h"
#include "fonts/ModFreeSans44pt.h"
#include "fonts/FreeSans16pt7b.h"
#include "fonts/FreeSans18pt7b.h"

#include "fonts/IconFont.h"

// #define PREVENT_LAYER_SWITCH

constexpr uint16_t upColor =   0b010001110001000;
constexpr uint16_t downColor = 0b111000100001000;

constexpr uint16_t speedColor = 0b0000001000001000;

void DataDisplay::begin() {
	uint16_t graphHeight = tireTopY-topRowY-40;

	_speedGraph = new DataGraph(0, topRowY+10, 400, graphHeight, RA8875_WHITE, speedColor, RA8875_BLUE, false);
	_altitudeGraph = new DataGraph(399, topRowY+10, 401, graphHeight, RA8875_WHITE, RA8875_GRAY_DK, RA8875_BLUE, true);

	_speedGraph->setRingBuff(_gps.speedHistory);
	_altitudeGraph->setRingBuff(_gps.altitudeHistory);

	_speedGraph->setScale(0, 80, 20);
	_altitudeGraph->setAutoScale(true);

	_altitudeGraph->setRampColors(10, upColor, 10, downColor);
}

uint8_t DataDisplay::ascenderForFont(const GFXfont *f, char character) {
	uint16_t index = character - f->first;
	int8_t offset = f->glyph[index].yOffset;

	return -offset;
}

void DataDisplay::getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height) {
	int16_t x, y;

	dest.getTextBounds(str, 0, 0, &x, &y, width, height);
}

uint16_t DataDisplay::getStringWidth(Adafruit_GFX& dest, String str) {
	int16_t x, y;
	uint16_t width, height;

	dest.getTextBounds(str, 0, 0, &x, &y, &width, &height);
	return width;
}

uint16_t DataDisplay::getStringGlyphWidth(const GFXfont *f, String str) {
	uint16_t width = 0;
	uint16_t count = str.length();
	const char* chars = str.c_str();

	for (uint16_t i=0; i<count; i++) {
		uint16_t index = chars[i] - f->first;
		width += f->glyph[index].xAdvance;
	}

	return width;
}

String DataDisplay::timeFromDayMinutes(double dayMinutes, bool includeSeconds) {
	uint16_t hours = dayMinutes / 60;
	uint16_t minutes = ((uint32_t)dayMinutes) % 60;
	uint16_t seconds = (dayMinutes - (uint32_t)dayMinutes) * 60.0;

	// hours = 12;

	if (hours >= 13) {
		hours -= 12;
	}
	else if (hours < 1) {
		hours += 12;
	}

	String result = String(hours) + ":";
	
	if (minutes < 10) {
		result = result + String("0");
	}

	result = result + String(minutes);
	
	if (includeSeconds) {
		result = result + String(":");
		if (seconds < 10) {
			result = result + String("0");
		}
		result = result + String(seconds);
	}

	return result;
}

String DataDisplay::suffixFromDayMinutes(double dayMinutes) {
	uint16_t hours = dayMinutes / 60;
	bool pm = hours >= 12 && hours <= 24;

	return String((pm) ? "pm" : "am");
}

const char* directionNames[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
const char *directionGlyphs = "KLMFGHIJ";
const char *speedGlyphs = "NOPQRSTUV";
char timeGlyph = 'A';
char altitudeGlyph = 'B';
char speedGlyph = 'C';
char sunriseGlyph = 'D';
char sunsetGlyph = 'E';
char satelliteGlyph = 'Z';
char emptySpeedlyph = 'W';
char emptyTimeGlyph = 'X';
char emptyDirectionGlyph = 'Y';

const GFXfont *textFont = &ModFreeSans44pt7b;
const GFXfont *suffixFont = &FreeSans16pt7b;
const GFXfont *glyphFont = &iconFont;

void offsetCursor(Adafruit_GFX& dest, int16_t xOffset, int16_t yOffset) {
	dest.setCursor(dest.getCursorX() + xOffset, dest.getCursorY() + yOffset);
}

void DataDisplay::showCell(Adafruit_GFX& dest, int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, String suffix, const GFXfont* strFont, const GFXfont* sufFont) {
	uint16_t ascender = ascenderForFont(strFont, glyph);
	uint16_t xOffset = -2;

	if (str.startsWith(String("1"))) {
		xOffset += oneOffset;
	}

	dest.setFont(glyphFont);
	dest.setCursor(x, y + ascender + 6);
	dest.print(glyph);
	dest.setFont(strFont);
	offsetCursor(dest, xOffset + 4, -5);
	dest.print(str);
	if (suffix) {
		dest.setFont(sufFont);
		dest.print(suffix);
	}
}

#define swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void writePixel(Adafruit_GFX& dest, int16_t x, int16_t y, uint16_t color, uint16_t thickness, bool fast = false) {
	if (fast) {
		uint8_t halfThick = thickness/2;
		dest.fillRect(x-halfThick, y-halfThick, thickness, thickness, color);
	}
	else if (thickness >= 5) {
		dest.writeFastHLine(x - 1, y - 2, 3, color);
		dest.writeFastHLine(x - 2, y - 1, 5, color);
		dest.writeFastHLine(x - 2, y, 5, color);
		dest.writeFastHLine(x - 2, y + 1, 5, color);
		dest.writeFastHLine(x - 1, y + 2, 3, color);
	}
	else if (thickness >= 3) {
		dest.writePixel(x, y - 1, color);
		dest.writeFastHLine(x - 1, y, 3, color);
		dest.writePixel(x, y + 1, color);
	}
	else {
		dest.writePixel(x, y, color);
	}
}

void DataDisplay::drawThickLine(Adafruit_GFX& dest, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color, bool fast) {
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap_int16_t(x0, y0);
		swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		swap_int16_t(x0, x1);
		swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			writePixel(dest, y0, x0, color, thickness, fast);
		}
		else {
			writePixel(dest, x0, y0, color, thickness, fast);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

#ifndef M_PI
		#define M_PI 3.14159265358979323846
#endif

void centerRotLenToPoint(int16_t centerX, int16_t centerY, float angle, float length, int16_t* outX, int16_t* outY) {
	float radAngle = angle * (2.0 * M_PI) / 360.0;

	*outX = round(centerX + sin(radAngle) * length);
	*outY = round(centerY - cos(radAngle) * length);
}

uint16_t hoursLen = 9;
uint16_t minutesLen = 16;

void DataDisplay::drawPolarLine(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t length, int16_t thickness) {
	int16_t destX, destY;

	centerRotLenToPoint(x, y, angle, length, &destX, &destY);
	
	if (thickness > 1) {
		drawThickLine(dest, x, y, destX, destY, thickness, WHITE8);
	}
	else {
		dest.drawLine(x, y, destX, destY, WHITE8);
	}
}

void DataDisplay::drawPointer(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t majorLen, uint16_t minorLen, uint16_t cornerAngle) {
	int16_t x1, x2, x3, y1, y2, y3;

	centerRotLenToPoint(x, y, angle, majorLen, &x1, &y1);
	centerRotLenToPoint(x, y, angle - cornerAngle, minorLen, &x2, &y2);
	centerRotLenToPoint(x, y, angle + cornerAngle, minorLen, &x3, &y3);
	dest.fillTriangle(x1, y1, x2, y2, x3, y3, WHITE8);
}

void DataDisplay::drawTime(Adafruit_GFX& dest, uint16_t x, uint16_t y, uint16_t hours, uint16_t minutes) {
	drawPolarLine(dest, x, y, hours * 360.0 / 12.0, hoursLen, 5);
	drawPolarLine(dest, x, y, minutes * 360.0 / 60.0, minutesLen, 5);
}

void DataDisplay::drawInAltLayer() {
	#ifndef PREVENT_LAYER_SWITCH
	_display.setDrawLayer(!_drawLayer);
	#endif
}

void DataDisplay::drawInCurrentLayer() {
	#ifndef PREVENT_LAYER_SWITCH
	_display.setDrawLayer(_drawLayer);
	#endif
}

void DataDisplay::switchToAltLayer() {
	#ifndef PREVENT_LAYER_SWITCH
	_display.showLayer(!_drawLayer);
	_drawLayer = !_drawLayer;
	#endif
}

bool DataDisplay::showData(uint16_t* drawIndex, uint32_t time, int16_t altitude, float heading, float speed, uint32_t sunriseTime, uint32_t sunsetTime, uint16_t satCount, bool haveFix, String status) {
	constexpr int16_t xOffset = 60;
	constexpr int16_t xGap = 40;
	constexpr int16_t yOffset = 18;
	constexpr int16_t yGap = 20;
	static uint8_t colon = 1;
	uint16_t xStart, yStart;
	String timeString;
	int16_t direction;
	bool result = false;

	drawInAltLayer();

	enum {
		dataDrawSpeed = 0,
		dataDrawAltitude,
		dataDrawTime,
		dataDrawHeading,
		dataDrawSunrise,
		dataDrawSunset,
		dataDrawSpeedGraph,
		dataDrawAltitudeGraph,
		dataDrawAcquiring,
		dataDrawTires
	};

	if (*drawIndex == dataDrawSpeed) {
		_display.fillScreen(BLACK8);
		_displayBuffer8.setTextColor(WHITE8);
		if (!haveFix) {
			*drawIndex = dataDrawAcquiring;
		}
	}

	bool useBuffer = *drawIndex>=dataDrawSpeed && *drawIndex<=dataDrawSunset;

	uint16_t col = *drawIndex & 1;
	uint16_t row = (*drawIndex) / 2;

	xStart = xOffset+col*(cellWidth+xGap);
	yStart = yOffset+row*(cellHeight+yGap);

	if (useBuffer) {
		_displayBuffer8.setOffset(xStart, yStart);
	}

	// _displayBuffer8.fillRect(xStart, yStart+2, cellWidth, cellHeight, GREEN8);

	switch (*drawIndex) {
		case dataDrawSpeed: {
				String speedStr = String(speed, 1);
				if (_drawGraphs) {
					xStart += 80 - getStringGlyphWidth(textFont, speedStr) / 2;
					_displayBuffer8.setOffset(xStart, yStart, -1);
				}
				showCell(_displayBuffer8, xStart, yStart, emptySpeedlyph, speedStr, 0, String("mph"));
				drawPolarLine(_displayBuffer8, xStart+32, yStart+44, -100.0+(speed / 75.0) * 200.0, 14, 5);
			}
			break;
		case dataDrawAltitude: {
				String altStr = String(altitude);
				if (_drawGraphs) {
					xStart += 130 - getStringGlyphWidth(textFont, altStr) / 2;
					_displayBuffer8.setOffset(xStart, yStart, -1);
				}
				showCell(_displayBuffer8, xStart, yStart, altitudeGlyph, altStr, -2, String("ft"));
			}
			break;
		case dataDrawTime:
			timeString = timeFromDayMinutes(time, false);
			if (!colon) {
				timeString.replace(String(":"), String(" "));
			}
			colon = (colon+1)%4;

			showCell(_displayBuffer8, xStart, yStart, emptyTimeGlyph, timeString, -2, suffixFromDayMinutes(time));
			drawTime(_displayBuffer8, xStart+32, yStart+36, time / 60, time % 60);
			break;
		case dataDrawHeading:
			direction = ((int16_t)((heading+11.25) / 22.5)) % 16; 
			showCell(_displayBuffer8, xStart, yStart, emptyDirectionGlyph, String(directionNames[direction]), 0, String(""));
			drawPointer(_displayBuffer8, xStart+32, yStart+36, heading, 15, 8, 140);
			break;
		case dataDrawSunrise:
			showCell(_displayBuffer8, xStart, yStart, sunriseGlyph, timeFromDayMinutes(sunriseTime, false), -9, suffixFromDayMinutes(sunriseTime));
			break;
		case dataDrawSunset:
			showCell(_displayBuffer8, xStart, yStart, sunsetGlyph, timeFromDayMinutes(sunsetTime, false), -9, suffixFromDayMinutes(sunsetTime));
			break;
		case dataDrawSpeedGraph:
			_speedGraph->draw();
			break;
		case dataDrawAltitudeGraph:
			_altitudeGraph->draw();
			break;
		case dataDrawAcquiring:
			static uint8_t dotCount = 1;
			static String acquiring = String("Acquiring");
			static String dots = String(".....");

			status = acquiring+dots.substring(0, dotCount);
			dotCount = (dotCount % 5)+1;

			_displayBuffer8.setFont(&FreeSans18pt7b);
			_displayBuffer8.setOffset(_display.width()/2 - 100, 100);
			_displayBuffer8.setCursor(_display.width()/2 - 100, 130);
			_displayBuffer8.print(status);
			_displayBuffer8.draw(_display, -1);
			break;
	}

	if (useBuffer) {
		_displayBuffer8.draw(_display, -1);
	}

	if (*drawIndex>=dataDrawTires) {
		_tireHandler.drawTires();
		switchToAltLayer();
		*drawIndex = dataDrawSpeed;
		result = true;
	}
	else {
		drawInCurrentLayer();
		if (*drawIndex==dataDrawAltitude && _drawGraphs) {
			*drawIndex = dataDrawSpeedGraph;
		}
		else if (*drawIndex==dataDrawSunset || *drawIndex==dataDrawAltitudeGraph) {
			*drawIndex = dataDrawTires;
		}
		else {
			(*drawIndex)++;
		}
	}

	return result;
}

DataDisplay _dataDisplay;
