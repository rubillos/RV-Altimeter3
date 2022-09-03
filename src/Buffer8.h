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

		void drawCenteredText(String str, int16_t x, int16_t y) {
			int16_t ax, ay;
			uint16_t width, height;

            getTextBounds(str, 0, 0, &ax, &ay, &width, &height);
            setCursor(x - width/2, y);
            print(str);
		}

		void draw(Adafruit_RA8875& dest, int16_t w=0, int16_t h=0) {
			uint8_t* srcBuff = getBuffer();
			if (w==0) { w=width(); }
			else if (w==-1) { w=getCursorX()-_x; }
			
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

class Buffer1 : public GFXcanvas8 {
	public:
		Buffer1(uint16_t x, uint16_t y, uint16_t w, uint16_t h) : GFXcanvas8(w, h) {
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

		void draw(Adafruit_RA8875& dest, int16_t w=0, int16_t h=0, bool layer2=false) {
			uint8_t* srcBuff = getBuffer();
			if (w==0) { w=width(); }
			else if (w==-1) { w=getCursorX()-_x; }
			
			if (h==0) { h=height(); }

			uint16_t bufferWidth = width();
			uint16_t rowBytes = (bufferWidth+7)/8;
			uint16_t byteWidth = (w+7)/8;

			dest.writeReg16(0x58, _x);
			dest.writeReg16(0x5A, _y | (layer2?0x8000:0));
			dest.writeReg16(0x5C, w);
			dest.writeReg16(0x5E, h);
			dest.writeColor(0x60, RA8875_BLACK);
			dest.writeColor(0x63, RA8875_WHITE);

			dest.writeReg(0x51, 0x08);
			dest.writeReg(0x50, 0x80);

			// Serial.println("Set up BTE");
			// delay(1);
			// dest.waitUntilDone();
			dest.Chk_BTE_Busy();
			// Serial.println("Ready to go.");

			for (uint16_t y=0; y<h; y++) {
				// Serial.printf("Writing line %d\n", y);
				for (uint16_t x=0; x<byteWidth; x++) {
					// Serial.printf("Writing byte %d\n", x);
					dest.writeData(srcBuff[y*rowBytes+x]);
					// delay(1);
					// dest.waitUntilDone();
				}
			}
			dest.Chk_BTE_Busy();
		}

	private:
		uint16_t _x;
		uint16_t _y;
};

#endif
