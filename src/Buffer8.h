#ifndef DEF_BUFFER8_H
#define DEF_BUFFER8_H

#include <Adafruit_GFX.h>
#include "Adafruit_RA8875.h"

class Buffer1 : public GFXcanvas1 {
	public:
		Buffer1(uint16_t x, uint16_t y, uint16_t w, uint16_t h) : GFXcanvas1(w, h) {
			_x = x;
			_y = y;
		}

		void setOffset(uint16_t x, uint16_t y, int32_t c=0) {
			_x = x;
			_y = y;
			if (c != -1) {
				fillScreen(c);
			}
		}

		void drawPixel(int16_t x, int16_t y, uint16_t color) {
			if (_altDest && _invert) {
				bool curC = getPixel(x-_x, y-_y);
				color = (curC) ? 0 : color;
			}
			GFXcanvas1::drawPixel(x-_x, y-_y, color);
			if (_altDest) {
				// Serial.println("drawPixel on alt");
				_altDest->drawPixel(x, y, color);
			}
		}

		void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
			if (_altDest && _invert) {
				startWrite();
				writeLine(x, y, x+w-1, y, color);
				endWrite();
			}
			else {
				GFXcanvas1::drawFastHLine(x-_x, y-_y, w, color);
				if (_altDest) {
					// Serial.println("drawFastHLine on alt");
					_altDest->drawFastHLine(x, y, w, color);
				}
			}
		}

		void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
			if (_altDest && _invert) {
				startWrite();
				writeLine(x, y, x, y+h-1, color);
				endWrite();
			}
			else {
				GFXcanvas1::drawFastVLine(x-_x, y-_y, h, color);
				if (_altDest) {
					// Serial.println("drawFastVLine on alt");
					_altDest->drawFastVLine(x, y, h, color);
				}
			}
		}

		void setAltDest(Adafruit_GFX* altDest) { _altDest = altDest; };
		void setInvert(bool invert) { _invert = invert; };

	private:
		uint16_t _x;
		uint16_t _y;

		Adafruit_GFX* _altDest = NULL;
		bool _invert;
};

class OutlineBuff : public GFXcanvas1 {
	public:
		OutlineBuff(uint16_t w, uint16_t h, uint8_t thickness=1) : GFXcanvas1(w, h) {
			_outlineBuff = new GFXcanvas1(w, h);
			_thickness = thickness;
		}

		void clear() {
			fillScreen(0);
			_outlineBuff->fillScreen(0);
		}

		void quickRect(Adafruit_GFX* dest, int16_t x, int16_t y, int16_t sizeX, int16_t sizeY, uint16_t color) {
			for (int16_t i = y; i < y + sizeY; i++) {
				dest->drawFastHLine(x, i, sizeX, color);
			}
		}

		void drawPixel(int16_t x, int16_t y, uint16_t color) {
			if (color) {
				GFXcanvas1::drawPixel(x, y, 1);
				uint8_t size = (_thickness*2)+1;
				quickRect(_outlineBuff, x-_thickness, y-_thickness, size, size, 1);
			}
		}

		void outline(uint16_t w, uint16_t h, Adafruit_GFX* dest, int16_t destX, int16_t destY, uint16_t color, uint16_t outlineColor) {
			if (w==0) { w = getCursorX()+1; };
			if (h==0) { h = height(); };

			for (uint16_t y=0; y<h; y++) {
				for (uint16_t x=0; x<w; x++) {
					if (getPixel(x, y)) {
						dest->drawPixel(destX+x, destY+y, color);
					}
					else if (_outlineBuff->getPixel(x, y)) {
						dest->drawPixel(destX+x, destY+y, outlineColor);
					}
				}
			}
		}

		void outline8(uint16_t w, uint16_t h, Adafruit_RA8875* dest, int16_t destX, int16_t destY, uint8_t color, uint8_t outlineColor) {
			if (w==0) { w = getCursorX()+1; };
			if (h==0) { h = height(); };

			uint16_t qCount = 0;
			uint8_t q[w];
			uint16_t qX;

			for (uint16_t y=0; y<h; y++) {
				for (uint16_t x=0; x<w; x++) {
					uint8_t outColor = 0;

					if (getPixel(x, y)) {
						outColor = 1;
					}
					else if (_outlineBuff->getPixel(x, y)) {
						outColor = 2;
					}

					if (outColor==0 && qCount>0) {
						dest->drawPixels8(q, qCount, destX+qX, destY+y);
						qCount = 0;
					}
					if (outColor) {
						if (qCount == 0) {
							qX = x;
						}
						q[qCount++] = (outColor==1) ? color : outlineColor;
					}
				}
				if (qCount>0) {
					dest->drawPixels8(q, qCount, destX+qX, destY+y);
					qCount = 0;
				}
			}
		}

	private:
		GFXcanvas1* _outlineBuff = NULL;
		uint8_t _thickness;
};

#endif
