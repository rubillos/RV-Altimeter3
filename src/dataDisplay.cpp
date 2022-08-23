#include "dataDisplay.h"

#include "defs.h"
#include "Adafruit_RA8875.h"
#include "touchscreen.h"
#include "Buffer8.h"
#include "tires.h"

#include "fonts/FreeSans44pt7b.h"
#include "fonts/ModFreeSans44pt.h"
#include "fonts/FreeSans16pt7b.h"
#include "fonts/FreeSans18pt7b.h"

#include "fonts/IconFont.h"

uint8_t ascenderForFont(const GFXfont *f, char character)
{
	uint16_t index = character - f->first;
	int8_t offset = f->glyph[index].yOffset;

	return -offset;
}

void getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height) {
	int16_t x, y;

	dest.getTextBounds(str, 0, 0, &x, &y, width, height);
}

uint16_t getStringWidth(Adafruit_GFX& dest, String str)
{
	int16_t x, y;
	uint16_t width, height;

	dest.getTextBounds(str, 0, 0, &x, &y, &width, &height);
	return width;
}

String timeFromDayMinutes(double dayMinutes, bool includeSeconds) {
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

String suffixFromDayMinutes(double dayMinutes) {
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

void showCell(Adafruit_GFX& dest, int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, String suffix, const GFXfont* strFont = textFont, const GFXfont* sufFont = suffixFont) {
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

void writePixel(Adafruit_GFX& dest, int16_t x, int16_t y, uint16_t color, uint16_t thickness) {
	if (thickness >= 5) {
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

void drawThickLine(Adafruit_GFX& dest, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color)
{
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
			writePixel(dest, y0, x0, color, thickness);
		}
		else {
			writePixel(dest, x0, y0, color, thickness);
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

void drawPolarLine(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t length, int16_t thickness) {
	int16_t destX, destY;

	centerRotLenToPoint(x, y, angle, length, &destX, &destY);
	
	if (thickness > 1) {
		drawThickLine(dest, x, y, destX, destY, thickness, WHITE8);
	}
	else {
		dest.drawLine(x, y, destX, destY, WHITE8);
	}
}

void drawPointer(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t majorLen, uint16_t minorLen, uint16_t cornerAngle) {
	int16_t x1, x2, x3, y1, y2, y3;

	centerRotLenToPoint(x, y, angle, majorLen, &x1, &y1);
	centerRotLenToPoint(x, y, angle - cornerAngle, minorLen, &x2, &y2);
	centerRotLenToPoint(x, y, angle + cornerAngle, minorLen, &x3, &y3);
	dest.fillTriangle(x1, y1, x2, y2, x3, y3, WHITE8);
}

void drawTime(Adafruit_GFX& dest, uint16_t x, uint16_t y, uint16_t hours, uint16_t minutes) {
	drawPolarLine(dest, x, y, hours * 360.0 / 12.0, hoursLen, 5);
	drawPolarLine(dest, x, y, minutes * 360.0 / 60.0, minutesLen, 5);
}

static bool drawLayer = false;

bool showData(uint16_t* drawIndex, uint32_t time, int16_t altitude, float heading, float speed, uint32_t sunriseTime, uint32_t sunsetTime, uint16_t satCount, bool haveFix, String status) {
	constexpr int16_t xOffset = 50;
	constexpr int16_t xGap = 45;
	constexpr int16_t yOffset = 20;
	constexpr int16_t yGap = 22;
	static uint8_t colon = 1;
	uint16_t xStart, yStart;
	String timeString;
	int16_t direction;
	bool result = false;

	_display.setDrawLayer(!drawLayer);

	if (*drawIndex == 0) {
		_display.fillScreen(BLACK8);

		_displayBuffer.setTextSize(1);
		_displayBuffer.setTextColor(WHITE8);
		_displayBuffer.setTextWrap(false);
	}

	if (haveFix) {
		uint16_t col = *drawIndex & 1;
		uint16_t row = (*drawIndex) / 2;

		xStart = xOffset+col*(cellWidth+xGap);
		yStart = yOffset+row*(cellHeight+yGap);
		_displayBuffer.setOffset(xStart, yStart);

		switch (*drawIndex) {
			case 0:
				// speed
				showCell(_displayBuffer, xStart, yStart, emptySpeedlyph, String(speed, 1), 0, String("mph"));
				drawPolarLine(_displayBuffer, xStart+32, yStart+44, -100.0+(speed / 75.0) * 200.0, 14, 5);
				break;
			case 1:
				// altitude
				// _displayBuffer.fillRect(xStart, yStart, cellWidth, cellHeight, GREEN8);
				showCell(_displayBuffer, xStart, yStart, altitudeGlyph, String(altitude), -2, String("ft"));
				break;
			case 2:
				// time
				timeString = timeFromDayMinutes(time, false);
				if (!colon) {
					timeString.replace(String(":"), String(" "));
				}
				colon = (colon+1)%4;

				showCell(_displayBuffer, xStart, yStart, emptyTimeGlyph, timeString, -2, suffixFromDayMinutes(time));
				drawTime(_displayBuffer, xStart+32, yStart+36, time / 60, time % 60);
				break;
			case 3:
				// heading
				direction = ((int16_t)((heading+11.25) / 22.5)) % 16; 
				showCell(_displayBuffer, xStart, yStart, emptyDirectionGlyph, String(directionNames[direction]), 0, String(""));
				drawPointer(_displayBuffer, xStart+32, yStart+36, heading, 15, 8, 140);
				break;
			case 4:
				// sunrise
				showCell(_displayBuffer, xStart, yStart, sunriseGlyph, timeFromDayMinutes(sunriseTime, false), -9, suffixFromDayMinutes(sunriseTime));
				break;
			case 5:
				// sunset
				showCell(_displayBuffer, xStart, yStart, sunsetGlyph, timeFromDayMinutes(sunsetTime, false), -9, suffixFromDayMinutes(sunsetTime));
				break;
		}
		_displayBuffer.draw(_display, -1);
		(*drawIndex)++;
	}
	else {
		if (*drawIndex == 0) {
			_displayBuffer.setOffset(_display.width()/2 - 100, 100);
			static uint8_t dotCount = 1;
			static String acquiring = String("Acquiring");
			static String dots = String(".....");

			status = acquiring+dots.substring(0, dotCount);
			dotCount = (dotCount % 5)+1;

			_displayBuffer.setFont(&FreeSans18pt7b);

			_displayBuffer.setCursor(_display.width()/2 - 100, 130);
			_displayBuffer.print(status);
			_displayBuffer.draw(_display);
		}
		*drawIndex = 6;
	}

	if (*drawIndex==6) {
		_tireHandler.drawTires();
		_display.showLayer(!drawLayer);
		drawLayer = !drawLayer;
		*drawIndex = 0;
		result = true;
	}
	else {
		_display.setDrawLayer(drawLayer);
	}

	return result;
}

