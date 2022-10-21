#include "textManager.h"

#include "touchscreen.h"
#include "defs.h"

TextManager::TextManager() {
	typedef struct {
		uint8_t c;
		int8_t left;
		int8_t right;
	} hShiftRec;
	constexpr hShiftRec hShifts[] = {
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
		'_', -1, 0,
		'\xBA', -1, 0,  // degrees
	};

	typedef struct {
		uint8_t c;
		int8_t vOff;
	} vShiftRec;
	constexpr vShiftRec vShifts[] = {
		'\x0E', 1,
		'\x10', 2,
		'\x11', 2,
		'\x1A', 3,
		'\x1B', 3,
		'\x1E', 2,
		'\x1F', 2,
	};

	typedef struct {
		char left;
		char right;
	} kernRec;
	constexpr kernRec kerns[] = {
		'v', 'i',
		'i', 't',
		'i', 'f',
		'l', 't',
		'l', 'f',
		'l', 'v',
		'l', 'T',
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

	for (uint16_t i=0; i<countof(hShifts); i++) {
		uint8_t c = hShifts[i].c;
		_charLeftShift[c] = hShifts[i].left;
		_charRightShift[c] = hShifts[i].right;
	}
	_spaceRightShift = _charRightShift[' '];

	for (uint16_t i=0; i<countof(vShifts); i++) {
		_charVertShift[vShifts[i].c] = vShifts[i].vOff;
	}

    for (uint16_t i=0; i<countof(kerns); i++) {
        uint16_t index = kerns[i].left<<8 | kerns[i].right;

        _charKerns[index >> 3] |= (1 << (index & 7));
    }
};

void TextManager::setSpaceNarrowing(bool narrow) {
	_charRightShift[' '] =  (narrow) ? _spaceRightShift : 0;
}

bool TextManager::isKernPair(char c1, char c2) {
	if (!_proportionalSpacing) {
		return false;
	}
	else {
		uint16_t index = c1<<8 | c2;

		return (_charKerns[index >> 3] & (1 << (index & 7))) != 0;
	}
}

uint16_t TextManager::widthOfString(String str, uint8_t xScale) {
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
		else if (_proportionalSpacing) {
			offset += _charLeftShift[c] + _charRightShift[c];
			if (i>0) {
				if (_textManager.isKernPair(chars[i-1], c)) {
					offset -= 1;
				}
			}
		}
	}

	return (8 * charCount + offset) * xScale;
}

void TextManager::drawString(String str, int16_t x, int16_t y, uint8_t xScale, uint8_t yScale, uint16_t textColor, int32_t backColor, uint16_t* otherColors) {
	const char* chars = str.c_str();
	uint16_t len = strlen(chars);

	if (len) {
		_display.textMode();
		_display.textEnlarge(xScale-1, yScale-1);

		uint16_t charSpacing = 8 * xScale;
		uint16_t charHeight = 16 * yScale;
		uint16_t currentColor = textColor;
		int32_t lastColor = -1;

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
				int16_t leftShift = (_proportionalSpacing) ? _textManager._charLeftShift[c] * xScale : 0;
				int16_t rightShift = (_proportionalSpacing) ? _textManager._charRightShift[c] * xScale : 0;

				if (backColor != -1) {
					_display.fillRect(x, y, charSpacing + leftShift + rightShift, charHeight, backColor);
					lastColor = -1;
				}

				x += leftShift;

				if (i>0) {
					if (_textManager.isKernPair(chars[i-1], c)) {
						x -= xScale;
					}
				}

				if (currentColor != lastColor) {
					_display.textTransparent(currentColor);
					lastColor = currentColor;
				}

				_display.textSetCursor(x, y + ((_charVertShift[c] * yScale)>>1));
				_display.textWrite(chars+i, 1);
				
				x += charSpacing + rightShift;
			}
		}

		_display.textTransparent(RA8875_BLACK);
		_display.graphicsMode();
	}
}

TextManager _textManager;
