#include "button.h"
#include "touchscreen.h"

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

bool Button::hitTestInternal(tsPoint_t pt, ButtonRect rect, bool widen) {
    computeScreenRect();

   if (widen) {
        return ((pt.x>=rect.x-widen_amount) && (pt.x<rect.x+rect.w+2*widen_amount) && (pt.y>=rect.y-widen_amount) && (pt.y<rect.y+rect.h+2*widen_amount));
    }
    else {
        return ((pt.x>=rect.x) && (pt.x<rect.x+rect.w) && (pt.y>=rect.y) && (pt.y<rect.y+_rect.h));
    }
};

bool Button::hitTest(tsPoint_t pt, bool widen) {
    return hitTestInternal(pt, _rect, widen);
};

void Button::drawInternal(uint16_t textColor, uint16_t backColor, uint16_t borderColor, bool forceBackground) {
    computeScreenRect();

    // Serial.printf("draw '%s': text=0x%X, back=0x%X, border=0x%X\n", title().c_str(), textColor, backColor, borderColor);

    ButtonScheme* sc = scheme();

    if (backColor == borderColor) {
        if (forceBackground || backColor) {
            _display.fillRect(_rect.x, _rect.y, _rect.w, _rect.h, backColor);
        }
    }
    else {
        _display.drawRect(_rect.x, _rect.y, _rect.w, _rect.h, borderColor);
        _display.drawRect(_rect.x+1, _rect.y+1, _rect.w-2, _rect.h-2, borderColor);
        _display.drawRect(_rect.x+2, _rect.y+2, _rect.w-4, _rect.h-4, borderColor);
        if (forceBackground || backColor) {
            _display.fillRect(_rect.x+3, _rect.y+3, _rect.w-6, _rect.h-6, backColor);
        }
    }
    _display.textMode();
    _display.textEnlarge(sc->sizeX-1, sc->sizeY-1);
    _display.textTransparent(textColor);

    uint16_t x;
    uint16_t y = _rect.y + ((_rect.h - titleHeight())/2) - (sc->sizeY-1)*2;

    if (sc->flags & buttonAlignLeft) {
        x = _rect.x + _titleInset;
    }
    else if (sc->flags & buttonAlignRight) {
        x = _rect.x + _rect.w - titleWidth() - _titleInset;
    }
    else {
        x = _rect.x + (_rect.w - titleWidth())/2;
    }
    _display.textSetCursor(x, y);

    _display.textWrite(title().c_str());
    _display.textTransparent(RA8875_BLACK);
    _display.graphicsMode();
}

void Button::draw(bool pressed, bool forceBackground) {
    uint16_t text, back, border;
    ButtonScheme* sc = scheme();

    if (pressed) {
        text = sc->backColor;
        back = sc->textColor;
        if (sc->backColor==sc->borderColor) {
            if (sc->textColor == 0) {
                border = sc->backColor;
            }
            else {
                border = sc->textColor;
            }
        }
        else {
            border = sc->borderColor;
        }
    }
    else {
        text = sc->textColor;
        back = sc->backColor;
        border = sc->borderColor;
    }
    drawInternal(text, back, border, forceBackground);
}

uint16_t Button::titleWidth() {
    return title().length() * 8 * scheme()->sizeX;
}

uint16_t Button::titleHeight() {
    return 16 * scheme()->sizeY;
}

void Button::computeScreenRect() {
    if (_dirty) {
        ButtonScheme* sc = scheme();;
        
        sc->sizeX = max(1, min(4, (int)sc->sizeX));
        sc->sizeY = max(1, min(4, (int)sc->sizeY));
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
            _rect.x = (_display.width() - _rect.w) / 2;
        }
        else if (_x == buttonRightSide) {   // align right
            _rect.x = _display.width() - _rect.w;
        }
        else {
            _rect.x = _x;
        }
        if (_y < 0) {   // vert center
            _rect.y = (_display.height() - _rect.h) / 2;
        }
        else {
            _rect.y = _y;
        }
    }
}
