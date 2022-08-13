#ifndef DEF_BUTTON_H
#define DEF_BUTTON_H

#include "Adafruit_RA8875.h"

typedef enum {
    buttonAlignCenter = 0x00,
    buttonAlignLeft = 0x01,
    buttonAlignRight = 0x02,

    buttonIsHeader = 0x40,

    buttonsNoClick = 0x80,

    buttonHCenter = -1,
    buttonRightSide = -2
} ButtonFlags;

typedef struct {
    const uint16_t textColor;
    const uint16_t backColor;
    const uint16_t borderColor;
    uint8_t size;
    uint16_t flags;
} ButtonScheme;

typedef struct {
    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
} ButtonRect;

extern Adafruit_RA8875* _button_tft;

class Button {
    public:
        Button(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) :
            _x(x), _y(y), _w(w), _h(h), _title(title), _scheme(scheme) {
                _dirty = true;
            };

        void setTFT(Adafruit_RA8875* tft) {
            _button_tft = tft;
        }

        bool hitTest(tsPoint_t pt, bool widen=false);
        void draw(bool pressed=false, bool forceBackground=false);
        void setTitle(String title);

        bool isHeader() { return (_scheme.flags & buttonIsHeader) != 0; }
        uint16_t width() { computeScreenRect(); return _rect.w; }
        uint16_t height() { computeScreenRect(); return _rect.h; }

        bool (*touchFunc)(void*);
        Button** subButtons;

    private:
        void drawInternal(uint16_t textColor, uint16_t backColor, uint16_t borderColor, bool forceBackground);
        uint16_t titleWidth();
        uint16_t titleHeight();
        void computeScreenRect();

        int16_t _x;
        int16_t _y;
        int16_t _w;
        int16_t _h;
        String _title;
        ButtonScheme _scheme;

        bool _dirty;
        ButtonRect _rect;
};

void drawButtons(Button** buttons);
Button* hitButton(Button** buttons, tsPoint_t pt, bool invert=true);

#endif
