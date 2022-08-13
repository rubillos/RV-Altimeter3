#include "button.h"

extern void wait_tft_done();

Adafruit_RA8875* _button_tft;

void drawButtons(Button** buttons) {
    while (*buttons) {
        (*buttons)->draw(false);
        buttons++;
    }
}

Button* hitButton(Button** buttons, tsPoint_t pt, bool invert) {
    for (uint8_t widen=0; widen<2; widen++) {
        Button** list = buttons;

        while (*list) {
            if ((*list)->hitTest(pt, widen)) {
                if (invert) {
                    (*list)->draw(true, true);
                }
                return *list;
            }
            list++;
        }
    }
    return NULL;
}

constexpr int16_t widen_amount = 20;

bool Button::hitTest(tsPoint_t pt, bool widen) {
    computeScreenRect();

    if (widen) {
        return (!(_scheme.flags & buttonsNoClick) && (pt.x>=_rect.x-widen_amount) && (pt.x<_rect.x+_rect.w+2*widen_amount) && (pt.y>=_rect.y-widen_amount) && (pt.y<_rect.y+_rect.h+2*widen_amount));
    }
    else {
        return (!(_scheme.flags & buttonsNoClick) && (pt.x>=_rect.x) && (pt.x<_rect.x+_rect.w) && (pt.y>=_rect.y) && (pt.y<_rect.y+_rect.h));
    }
};


void Button::drawInternal(uint16_t textColor, uint16_t backColor, uint16_t borderColor, bool forceBackground) {
    computeScreenRect();

    // Serial.printf("draw '%s': text=0x%X, back=0x%X, border=0x%X\n", _title.c_str(), textColor, backColor, borderColor);

    if (backColor == borderColor) {
        if (forceBackground || backColor) {
            _button_tft->fillRect(_rect.x, _rect.y, _rect.w, _rect.h, backColor);
            wait_tft_done();
        }
    }
    else {
        _button_tft->drawRect(_rect.x, _rect.y, _rect.w, _rect.h, borderColor);
        wait_tft_done();
        _button_tft->drawRect(_rect.x+1, _rect.y+1, _rect.w-2, _rect.h-2, borderColor);
        wait_tft_done();
        _button_tft->drawRect(_rect.x+2, _rect.y+2, _rect.w-4, _rect.h-4, borderColor);
        wait_tft_done();
        if (forceBackground || backColor) {
            _button_tft->fillRect(_rect.x+3, _rect.y+3, _rect.w-6, _rect.h-6, backColor);
            wait_tft_done();
        }
    }
    _button_tft->textMode();
    _button_tft->textEnlarge(_scheme.size-1);
    _button_tft->textTransparent(textColor);

    uint16_t x;
    uint16_t y = _rect.y + ((_rect.h - titleHeight())/2) - (_scheme.size-1)*2;

    if (_scheme.flags & buttonAlignLeft) {
        x = _rect.x;
    }
    else if (_scheme.flags & buttonAlignRight) {
        x = _rect.x + _rect.w - titleWidth();
    }
    else {
        x = _rect.x + (_rect.w - titleWidth())/2;
    }
    _button_tft->textSetCursor(x, y);

    _button_tft->textWrite(_title.c_str());
    wait_tft_done();
    _button_tft->graphicsMode();
}

void Button::draw(bool pressed, bool forceBackground) {
    uint16_t text, back, border;

    if (pressed) {
        text = _scheme.backColor;
        back = _scheme.textColor;
        if (_scheme.backColor==_scheme.borderColor) {
            if (_scheme.textColor == 0) {
                border = _scheme.backColor;
            }
            else {
                border = _scheme.textColor;
            }
        }
        else {
            border = _scheme.borderColor;
        }
    }
    else {
        text = _scheme.textColor;
        back = _scheme.backColor;
        border = _scheme.borderColor;
    }
    drawInternal(text, back, border, forceBackground);
}

void Button::setTitle(String title) {
    _title = title;
    _dirty = true;
}

uint16_t Button::titleWidth() {
    return _title.length() * 8 * _scheme.size;
}

uint16_t Button::titleHeight() {
    return 16*_scheme.size;
}

void Button::computeScreenRect() {
    if (_dirty) {
        _scheme.size = max(1, min(4, (int)_scheme.size));
        if (_w <= 0) {  // size from title, with inset
            _rect.w = titleWidth() + -_w*2;
        }
        else {
            _rect.w = _w;
        }
        if (_h <= 0) {  // size from title, with inset
            _rect.h = titleHeight() + -_h*2;
        }
        else {
            _rect.h = _h;
        }
        if (_x == buttonHCenter) {   // horiz center
            _rect.x = (_button_tft->width() - _rect.w) / 2;
        }
        else if (_x == buttonRightSide) {   // align right
            _rect.x = _button_tft->width() - _rect.w;
        }
        else {
            _rect.x = _x;
        }
        if (_y < 0) {   // vert center
            _rect.y = (_button_tft->height() - _rect.h) / 2;
        }
        else {
            _rect.y = _y;
        }
    }
}
