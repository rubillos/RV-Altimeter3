#ifndef DEF_TEXTMANAGER_H
#define DEF_TEXTMANAGER_H

#include "Arduino.h"

class TextManager {
	public:
		TextManager();

        void setSpaceNarrowing(bool narrow=true);

        bool isKernPair(char c1, char c2);
        uint16_t widthOfString(String str, uint8_t scale);
        void drawString(String str, int16_t x, int16_t y, uint8_t xScale, uint8_t yScale, uint16_t textColor, int32_t backColor=-1);

        int8_t charLeftShift[256];
        int8_t charRightShift[256];
        uint8_t charKerns[2048];
        int8_t charVertShift[256];

	private:
        int8_t _spaceRightShift;
};

extern TextManager _textManager;

#endif
