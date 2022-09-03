#include "textManager.h"

#include "touchscreen.h"

TextManager::TextManager() {
	constexpr int16_t shifts[] = {
		' ', 0, -5,
		'.', -2, -2,
		',', -2, -2,
		'\'', -3, -1,
		'!', -3, -2,
		':', -2, -2,
		';', -2, -2,
		'$', 0, 1,
		'%', 0, 1,
		'&', 0, 1,
		'*', 0, 1,
		'1', -1, 0,
		'\xBA', -1, 0,
		'a', -1, 0,
		'b', -1, 0,
		'c', -1, 0,
		'd', -1, 0,
		'e', -1, 0,
		'f', -1, -1,
		'g', -1, 0,
		'h', -1, 0,
		'i', -2, 0,
		'j', -1, 0,
		'k', -1, 0,
		'l', -2, 0,
		'm', -1, 1,
		'n', -1, 0,
		'o', -1, 0,
		'p', -1, 0,
		'q', -1, 0,
		'r', -1, 0,
		's', -1, 0,
		't', -1, -1,
		'u', -1, 0,
		'v', -1, 0,
		'w', -1, 1,
		'x', -1, 0,
		'y', -1, 0,
		'z', -1, 0,
		'I', -1, 0,
		'J', -1, 0,
		'T', 0, 1,
		'V', 0, 1,
		'Y', 0, 1,
	};
	constexpr uint16_t shiftCount = sizeof(shifts) / sizeof(int16_t) / 3;

	constexpr char kerns[] = {
		'v', 'i',
		'i', 't',
		'i', 'f',
		'l', 't',
		'r', 'i',
		'r', 'l',
		'L', 'T',
		'T', 'a',
		'T', 'e',
		'T', 'i',
		'T', 'o',
		'T', 'u',
		'T', 'y',
		'T', 'r',
		'T', 'w',
		'F', 'i',
		'F', 'l',
		'F', 'A',
		'6', '7',
		'1', '7',
	};
	constexpr uint16_t kernCount = sizeof(kerns) / sizeof(char) / 2;

	constexpr int8_t vertShifts[] = {
		'\x0E', 1,
	};
	constexpr uint16_t vertShiftCount = sizeof(vertShifts) / sizeof(int8_t) / 2;

	for (uint16_t i=0; i<shiftCount; i++) {
		uint8_t c = shifts[i*3];
		charLeftShift[c] = shifts[i*3+1];
		charRightShift[c] = shifts[i*3+2];

		if (c==' ') {
			_spaceRightShift = shifts[i*3+2];
		}
	}

    for (uint16_t i=0; i<kernCount; i++) {
        uint16_t index = kerns[i*2]<<7 | kerns[i*2+1];

        charKerns[index >> 3] |= (1 << (index & 7));
    }

	for (uint16_t i=0; i<vertShiftCount; i++) {
		charVertShift[vertShifts[i*2]] = vertShifts[i*2+1];
	}
};

void TextManager::setSpaceNarrowing(bool narrow) {
	charRightShift[' '] =  (narrow) ? _spaceRightShift : 0;
}

bool TextManager::isKernPair(char c1, char c2) {
	uint16_t index = c1<<7 | c2;

	return (charKerns[index >> 3] & (1 << (index & 7))) != 0;
}

uint16_t TextManager::widthOfString(String str, uint8_t scale) {
	const char* chars = str.c_str();
	uint16_t len = strlen(chars);
	uint16_t charCount = len;
	uint16_t offset = 0;

	for (uint16_t i=0; i<len; i++) {
		uint8_t c = chars[i];

		if (c == '\b' && i<len-1) {
			charCount -= 2;
			i += 1;
		}
		else {
			offset += charLeftShift[c] + charRightShift[c];
		}
	}

	return (8 * charCount + offset) * scale;
}

void TextManager::drawString(String str, int16_t x, int16_t y, uint8_t xScale, uint8_t yScale, uint16_t textColor, int32_t backColor, uint16_t* otherColors) {
	_display.textMode();
	_display.textEnlarge(xScale-1, yScale-1);

	const char* chars = str.c_str();
	uint16_t len = strlen(chars);
	uint16_t charSpacing = 8 * xScale;
	uint16_t charHeight = 16 * yScale;

	uint16_t currentColor = textColor;

	for (uint16_t i=0; i<len; i++) {
		uint8_t c = chars[i];

		if (c=='\b' && i<len-1) {
			if (otherColors != NULL) {
				int8_t colorIndex = chars[i+1] - '0' - 1;

				if (colorIndex==-1) {
					currentColor = textColor;
				}
				else {
					currentColor = otherColors[colorIndex];
				}
			}
			i += 1;
		}
		else {
			int16_t leftShift = _textManager.charLeftShift[c] * xScale;
			int16_t rightShift = _textManager.charRightShift[c] * xScale;

			if (backColor != -1) {
				_display.fillRect(x, y, charSpacing + leftShift + rightShift, charHeight, backColor);
			}

			x += leftShift;

			if (i>0) {
				if (_textManager.isKernPair(chars[i-1], c)) {
					x -= xScale;
				}
			}

			_display.textTransparent(currentColor);
			_display.textSetCursor(x, y + ((charVertShift[c] * yScale)>>1));
			_display.textWrite(chars+i, 1);
			
			x += charSpacing + rightShift;
		}
	}

	_display.textTransparent(RA8875_BLACK);
	_display.graphicsMode();
}

TextManager _textManager;
