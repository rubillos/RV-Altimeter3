#include "button.h"
#include "textManager.h"
#include "defs.h"

void drawButtons(Button** buttons) {
    while (*buttons) {
        (*buttons)->draw(false);
        buttons++;
		_touchScreen.touchRefresh();
    }
}

constexpr int16_t button_widen_amount = 10;

Button* hitButton(Button** buttons, tsPoint_t pt, bool invert) {
    for (uint8_t widen=0; widen<3; widen++) {
        Button** list = buttons;

        while (*list) {
            if ((*list)->hitTest(pt, widen*button_widen_amount)) {
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

void setupButtons(Button** buttons) {
    Button* prior = NULL;

    while (*buttons) {
        Button* button = *buttons;
        if (button->_y & (buttonVPriorBelow | buttonVPriorSame)) {
            button->_priorButton = prior;
        }
        prior = button;
        buttons++;
    }
}

bool Button::hitTestInternal(tsPoint_t pt, ButtonRect rect, uint16_t widen) {
    if (!visible()) {
        return false;
    }

    computeScreenRect();

    return ((pt.x>=rect.x-widen) && (pt.x<rect.x+rect.w+2*widen) && (pt.y>=rect.y-widen) && (pt.y<rect.y+rect.h+2*widen));
};

bool Button::hitTest(tsPoint_t pt, uint16_t widen) {
    return hitTestInternal(pt, _rect, widen);
};

void Button::setVisible(bool vis) {
    if (vis != _visible) {
        _visible = vis;
        _dirty = true;
    }
}

void Button::hide() {
    setVisible(false);
}

void Button::show() {
    setVisible(true);
}

ButtonScheme Button::scheme(bool pressed) {
    uint16_t text, back, border;
    ButtonScheme scheme = _scheme;

    scheme.sizeX = max(1, min(4, (int)scheme.sizeX));
    scheme.sizeY = max(1, min(4, (int)scheme.sizeY));

    if (pressed) {
        scheme.textColor = _scheme.backColor;
        scheme.backColor = _scheme.textColor;
        if (_scheme.backColor==_scheme.borderColor) {
            if (_scheme.textColor == 0) {
                scheme.borderColor = _scheme.backColor;
            }
            else {
                scheme.borderColor = _scheme.textColor;
            }
        }
        else {
            scheme.borderColor = _scheme.borderColor;
        }
    }

    return scheme;
}

void Button::draw(bool pressed, bool forceBackground) {
    if (!visible()) {
        return;
    }

    ButtonScheme sc = scheme(pressed);
    
    uint16_t textColor = sc.textColor;
    uint16_t backColor = sc.backColor;
    uint16_t borderColor = sc.borderColor;

    computeScreenRect();

    // Serial.printf("draw '%s': text=0x%X, back=0x%X, border=0x%X\n", title().c_str(), textColor, backColor, borderColor);

    bool transparent = transparentText();

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
    uint16_t x;
    uint16_t y = _rect.y + ((_rect.h - titleHeight())/2) - (sc.sizeY-1)*2;

    if (sc.flags & buttonAlignLeft) {
        x = _rect.x + _titleInset;
    }
    else if (sc.flags & buttonAlignRight) {
        x = _rect.x + _rect.w - titleWidth() - _titleInset;
    }
    else {
        x = _rect.x + (_rect.w - titleWidth())/2;
    }

    drawTitle(title(), x, y, sc.sizeX, sc.sizeY, textColor, (transparent)?-1:backColor);
}

void Button::drawTitle(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor) {
    if (!visible()) {
        return;
    }
    
    _textManager.drawString(title, x, y, sizeX, sizeY, textColor, backColor);
}

uint16_t Button::titleWidth() {
    return _textManager.widthOfString(title(), scheme().sizeX);
}

uint16_t Button::titleHeight() {
    return 16 * scheme().sizeY;
}

void Button::computeScreenRect() {
    if (_dirty) {
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
        if (_y & (buttonVPriorBelow | buttonVPriorSame)) {
            if (_priorButton) {
                ButtonRect priorRect = _priorButton->rect();
                if (_y & buttonVPriorSame) {
                    _rect.y = priorRect.y;
                }
                else {
                    _rect.y = priorRect.y + priorRect.h + (_y & buttonVPriorMask);
                }
            }
            else {
                _rect.y = 0;
            }
        }
        else if (_y < 0) {   // vert center
            _rect.y = (_display.height() - _rect.h) / 2;
        }
        else {
            _rect.y = _y;
        }
        _dirty = false;
    }
}
