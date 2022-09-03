#ifndef DEF_BUTTON_H
#define DEF_BUTTON_H

#include "Adafruit_RA8875.h"

typedef enum {
	buttonAlignCenter = 0x00,
	buttonAlignLeft = 0x01,
	buttonAlignRight = 0x02,

	buttonAlignVCenter = 0x10,

	buttonHCenter = -1,
	buttonRightSide = -2
} ButtonFlags;

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
				_dirty = true;
			};

		void setTitle(String title) { _title = title; _dirty = true; };
		void setTitleInset(uint16_t inset) { _titleInset = inset; };

		uint16_t width() { computeScreenRect(); return _rect.w; };
		uint16_t height() { computeScreenRect(); return _rect.h; };
		ButtonRect rect() { computeScreenRect(); return _rect; };

		bool (*touchFunc)(void*, void*);
		Button** subButtons;

		virtual String title() { return _title; };
		virtual ButtonScheme scheme(bool pressed = false);
		virtual bool hitTest(tsPoint_t pt, bool widen=false);
		virtual bool isHeader() { return false; };
		virtual void performDraw(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor);
		virtual void draw(bool pressed=false, bool forceBackground=false);
		virtual bool transparentText() { return true; };
		virtual bool refresh() { return false; };

	protected:
		bool hitTestInternal(tsPoint_t pt, ButtonRect rect, bool widen);
		uint16_t titleWidth();
		uint16_t titleHeight();
		void computeScreenRect();

		int16_t _x;
		int16_t _y;
		int16_t _w;
		int16_t _h;
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
		void draw(bool pressed=false, bool forceBackground=false);
		void setState(bool state) { _state=state; _dirty=true; };
		bool refresh() {
			return _dirty;
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
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
		bool transparentText() { return false; };
};

class Header : public Button {
	public:
		Header(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : 
			Button(x, y, w, h, title, scheme) {

		};
		bool isHeader() { return true; };
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
};

class FloatLabel : public Label {
	public:
		FloatLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0,
				void* param1=NULL, void* param2=NULL, void* param3=NULL, void* param4=NULL ) : 
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
		bool refresh() {
			updateValues(true);
			return _dirty;
		};
	protected:
		float* _param[4] = { NULL, NULL, NULL, NULL };
		float _values[4];
};

void drawButtons(Button** buttons);
Button* hitButton(Button** buttons, tsPoint_t pt, bool invert=true);

#endif
