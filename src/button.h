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
		virtual ButtonScheme* scheme() { return &_scheme; };
		virtual bool hitTest(tsPoint_t pt, bool widen=false);
		virtual bool isHeader() { return false; };
		virtual void draw(bool pressed=false, bool forceBackground=false);
		virtual void refresh() {};

	protected:
		void drawInternal(uint16_t textColor, uint16_t backColor, uint16_t borderColor, bool forceBackground);
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

class Label : public Button {
	public:
		Label(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0 ) : Button(x, y, w, h, title, scheme) {
			_titleInset = titleInset;
		};
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
};

class Header : public Button {
	public:
		Header(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : Button(x, y, w, h, title, scheme) {

		};
		bool isHeader() { return true; };
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
};

void drawButtons(Button** buttons);
Button* hitButton(Button** buttons, tsPoint_t pt, bool invert=true);

#endif
