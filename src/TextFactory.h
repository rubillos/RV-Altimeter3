#include "GFXcanvas2.h"

class TextFactory
{
public:
    TextFactory();
    ~TextFactory(void);

    GFXcanvas2* smoothCanvas(GFXcanvas1* srcCanvas);

    uint8_t ascenderForFont(const GFXfont *f);
    uint8_t xAdvanceForChar(char ch, const GFXfont *f);
    GFXcanvas1 *makeString(const char *str, const GFXfont *f);
    GFXcanvas1 *makeCharacter(char ch, const GFXfont *f);
    GFXcanvas2 *makeStringSmooth(const char *str, const GFXfont *f, int16_t offsetGlyphs = 0, int16_t forceHeight = -1, int16_t forceWidth = -1);
    GFXcanvas2 *makeCharacterSmooth(char ch, const GFXfont *f, int16_t offsetGlyphs = 0, int16_t forceHeight = -1, int16_t forceWidth = -1);

private: 
    GFXcanvas1 *workCanvas;
};
