#ifndef DEF_BUFFER8_H
#define DEF_BUFFER8_H

#include <Adafruit_GFX.h>
#include "Adafruit_RA8875.h"

class Buffer8 : public GFXcanvas8 {
	public:
		Buffer8(uint16_t x, uint16_t y, uint16_t w, uint16_t h) : GFXcanvas8(w, h) {
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
			GFXcanvas8::drawPixel(x-_x, y-_y, color);
		}

		void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
			GFXcanvas8::drawFastHLine(x-_x, y-_y, w, color);
		}

		void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
			GFXcanvas8::drawFastVLine(x-_x, y-_y, h, color);
		}

		void drawCenteredText(String str, int16_t x, int16_t y) {
			int16_t ax, ay;
			uint16_t width, height;

            getTextBounds(str, 0, 0, &ax, &ay, &width, &height);
            setCursor(x - width/2, y);
            print(str);
		}

		void draw(Adafruit_RA8875& dest, int16_t w=0, int16_t h=0) {
			uint8_t* srcBuff = getBuffer();
			if (w<0) { w=getCursorX()-_x; }
			if (w==0) { w=width(); }
			
			if (h==0) { h=height(); }

			uint16_t bufferWidth = width();

			for (uint16_t y=0; y<h; y++) {
				dest.drawPixels8(srcBuff+y*bufferWidth, w, _x, _y+y);
			}
		}

	private:
		uint16_t _x;
		uint16_t _y;
};

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

#endif
