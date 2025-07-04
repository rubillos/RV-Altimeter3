#pragma once

#include "Adafruit_RA8875.h"

#include "dataDisplay.h"
#include "touchscreen.h"

typedef enum {
	buttonAlignCenter = 0x00,
	buttonAlignLeft = 0x01,
	buttonAlignRight = 0x02,

	buttonAlignVCenter = 0x10,

	buttonHCenter = -1,
	buttonRightSide = -2,

	buttonVPriorBelow = 0x4000,
	buttonVPriorSame = 0x2000,
	buttonVPriorMask = 0x1FFF,
} ButtonFlags;

enum {
	buttonRefreshNone = 0,
	buttonRefreshRedraw,
	buttonRefreshFast
};

typedef struct {
	uint16_t textColor;
	uint16_t backColor;
	uint16_t borderColor;
	uint8_t sizeX;
	uint8_t sizeY;
	uint16_t flags;
} ButtonScheme;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
} ButtonRect;

class Button {
	public:
		Button(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) :
			_x(x), _y(y), _w(w), _h(h), _title(title), _scheme(scheme) {
				_visible = true;
				_dirty = true;
			};

		void setTitleInset(uint16_t inset) { _titleInset = inset; };

		uint16_t width() { computeScreenRect(); return _rect.w; };
		uint16_t height() { computeScreenRect(); return _rect.h; };
		ButtonRect rect() { computeScreenRect(); return _rect; };

		void setVisible(bool visible);
		void hide();
		void show();

		bool (*touchFunc)(void*, void*) = NULL;
		Button** subButtons;

		virtual void setTitle(String title) { _title = title; _dirty = true; };
		virtual String title() { return _title; };
		virtual ButtonScheme scheme(bool pressed = false);
		virtual bool hitTest(tsPoint_t pt, uint16_t widen=0);
		virtual bool isHeader() { return false; };
		virtual void drawTitle(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor);
		virtual void draw(bool pressed=false, bool forceBackground=false);
		virtual bool transparentText() { return true; };
		virtual uint8_t refresh() { return false; };
		virtual bool visible() { return _visible; };

		Button* _priorButton = NULL;
		int16_t _x;
		int16_t _y;
		int16_t _w;
		int16_t _h;

	protected:
		bool hitTestInternal(tsPoint_t pt, ButtonRect rect, uint16_t widen);
		uint16_t titleWidth();
		uint16_t titleHeight();
		void computeScreenRect();

		bool _visible;
		int16_t _titleInset;
		String _title;
		ButtonScheme _scheme;

		bool _dirty;
		ButtonRect _rect;
};

class SlashButton : public Button {
	public:
		SlashButton(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, bool state=false ) :
			Button(x, y, w, h, title, scheme) {
				_state = state;
		};
		void setState(bool state) { _state=state; _dirty=true; };
		uint8_t refresh() {
			return _dirty ? buttonRefreshRedraw : buttonRefreshNone;
		};
		void draw(bool pressed=false, bool forceBackground=false) {
			Button::draw(pressed, forceBackground);
			if (!_state) {
				_dataDisplay.drawThickLine(_display, _rect.x+5, _rect.y+_rect.h-6, _rect.x+_rect.w-6, _rect.y+5, 5, RA8875_RED, true);
			}
		};
	private:
		bool _state;
};

class Label : public Button {
	public:
		Label(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0 ) :
			Button(x, y, w, h, title, scheme) {
				_titleInset = titleInset;
		};
		bool hitTest(tsPoint_t pt, uint16_t widen=0) { return false; };
		bool transparentText() { return true; };
};

class Header : public Button {
	public:
		Header(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : 
			Button(x, y, w, h, title, scheme) {

		};
		bool isHeader() { return true; };
		bool hitTest(tsPoint_t pt, uint16_t widen=0) { return false; };
};

class FloatLabel : public Label {
	public:
		FloatLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0) : 
			Label(x, y, w, h, title, scheme, titleInset) {	
		};
		void setParameter(uint8_t index, float* param) {
			_param[index] = param;
			_values[index] = *param;
		};
		void updateValues(bool setDirty) {
			for (uint8_t i=0; i<4; i++) {
				if (_param[i]) {
					float value = *(_param[i]);
					if (value != _values[i]) {
						_values[i] = value;
						if (setDirty) {
							_dirty = true;
						}
					}
				}
			}
		};
		String title() {
			char buffer[50];
			updateValues(false);
			snprintf(buffer, sizeof(buffer), _title.c_str(), _values[0], _values[1], _values[2], _values[3]);
			return buffer;
		};
		uint8_t refresh() {
			updateValues(true);
			return _dirty ? buttonRefreshRedraw : buttonRefreshNone;
		};
	protected:
		float* _param[4] = { NULL, NULL, NULL, NULL };
		float _values[4];
};

class StringLabel : public Label {
	public:
		StringLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0) : 
			Label(x, y, w, h, title, scheme, titleInset) { };
		void setParameter(uint8_t index, const char* param) {
			_param[index] = param;
			_dirty = true;
		};
		String title() {
			char buffer[80];
			snprintf(buffer, sizeof(buffer), _title.c_str(), _param[0], _param[1], _param[2], _param[3]);
			return buffer;
		};
		uint8_t refresh() {
			return _dirty ? buttonRefreshRedraw : buttonRefreshNone;
		};
	protected:
		const char* _param[4] = { NULL, NULL, NULL, NULL };
};

void drawButtons(Button** buttons);
Button* hitButton(Button** buttons, tsPoint_t pt, bool invert=true);
void setupButtons(Button** buttons);
