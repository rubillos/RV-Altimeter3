#include "png.h"
#include "defs.h"

#define rBits 0b1100000000000000
#define gBits 0b0000011000000000
#define bBits 0b0000000000011000

#define rShift 8
#define gShift 6
#define bShift 3

// #define RGB16to8(v) (((v&rBits)>>rShift) | ((v&gBits)>>gShift) | ((v&bBits)>>bShift))

inline uint16_t RGB16to8(uint16_t v) {
	uint8_t c = (((v&rBits)>>rShift) | ((v&gBits)>>gShift) | ((v&bBits)>>bShift));
	c |= (c & 0b10010000) >> 2;
	return c;
}

PNG png;
int16_t pngX, pngY;
Adafruit_GFX* pngDest;
Adafruit_RA8875* pngDest8;

void PNGDraw(PNGDRAW *pDraw) {
	uint16_t pixels[pDraw->iWidth];

	png.getLineAsRGB565(pDraw, pixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
	for (uint16_t i = 0; i<pDraw->iWidth; i++) {
		uint8_t pixel8 = RGB16to8(pixels[i]);
		if (pixel8) {
			pngDest->drawPixel(pngX+i, pngY+pDraw->y, pixel8);
		}
	}
}

void drawPNG(const unsigned char* data, uint32_t length, Adafruit_GFX* dest, int16_t x, int16_t y) {
	if (png.openRAM((uint8_t*)data, length, PNGDraw) == PNG_SUCCESS) {
		pngX = x;
		pngY = y;
		pngDest = dest;
		if (png.decode(NULL, 0) != PNG_SUCCESS) {
			Serial.println("PNG Decode Error!");
		}
	}
}

void PNGDraw8(PNGDRAW *pDraw) {
	uint16_t pixels[pDraw->iWidth];
	uint8_t pixels8[pDraw->iWidth];

	png.getLineAsRGB565(pDraw, pixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
	for (uint16_t i = 0; i<pDraw->iWidth; i++) {
		pixels8[i] = RGB16to8(pixels[i]);
	}
	pngDest8->drawPixels8(pixels8, pDraw->iWidth, pngX, pngY+pDraw->y);
}

void PNGDraw8MidGray(PNGDRAW *pDraw) {
	uint16_t pixels[pDraw->iWidth];
	uint8_t pixels8[pDraw->iWidth];

	png.getLineAsRGB565(pDraw, pixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
	for (uint16_t i = 0; i<pDraw->iWidth; i++) {
		uint16_t pixel = pixels[i];
		uint8_t pixel8;

		if (pixel==0) {
			pixel8 = 0;
		}
		else if (pixel==0xFFFF) {
			pixel8 = WHITE8;
		}
		else {
			pixel8 = DARK_GRAY8;
		}
		pixels8[i] = pixel8;
	}
	pngDest8->drawPixels8(pixels8, pDraw->iWidth, pngX, pngY+pDraw->y);
}

void PNGDraw8MidGray2(PNGDRAW *pDraw) {
	uint16_t pixels[pDraw->iWidth];

	png.getLineAsRGB565(pDraw, pixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);

	int16_t lastPixel = -1;
	uint16_t lastX = 0;
	uint16_t width = 0;

	for (uint16_t i = 0; i<pDraw->iWidth; i++) {
		uint16_t pixel = pixels[i];
		uint8_t pixel8;

		if (pixel==0) { pixel8 = 0; }
		else if (pixel==0xFFFF) { pixel8 = WHITE8; }
		else { pixel8 = DARK_GRAY8; }
		
		if (pixel8 == lastPixel) {
			width++;
		}
		else {
			if (width) {
				pngDest8->drawFastHLine(pngX+lastX, pngY+pDraw->y, width, lastPixel);
			}

			lastPixel = pixel8;
			lastX = i;
			width = 1;
		}
	}
	if (width) {
		pngDest8->drawFastHLine(pngX+lastX, pngY+pDraw->y, width, lastPixel);
	}
}

void drawPNG8(const unsigned char* data, uint32_t length, Adafruit_RA8875* dest, int16_t x, int16_t y, bool midGray) {
	if (png.openRAM((uint8_t*)data, length, (midGray)?PNGDraw8MidGray:PNGDraw8) == PNG_SUCCESS) {
		pngX = x;
		pngY = y;
		pngDest8 = dest;
		if (png.decode(NULL, 0) != PNG_SUCCESS) {
			Serial.println("PNG Decode Error!");
		}
	}
}

