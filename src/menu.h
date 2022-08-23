#ifndef DEF_MENU_H
#define DEF_MENU_H

#include "elapsedMillis.h"
#include "button.h"
#include "touchscreen.h"
#include "tires.h"
#include "packets.h"
#include "tires.h"
#include "gps.h"
#include "prefs.h"

extern void packetCheck();

#define ColorRA8875(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

constexpr uint16_t RA8875_GRAY_DK = 0b0101001010101010;
constexpr uint16_t RA8875_GRAY_LT = 0b1010010100010100;

constexpr uint16_t logLines = 8;
constexpr uint16_t logLineHeight = 17;

class LogView : public Button {
	public:
		LogView(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : Button(x, y, w, h, title, scheme) { };
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
		void draw(bool pressed=false, bool forceBackground=false);
		void drawPacket(TPMSPacket& packet, uint16_t lineNumber);
        void refresh();
		void firstPage() { if (_pageNumber > 0) { _pageNumber==0; draw(); } };
		void nextPage() { if (_pageNumber+1 < pageCount()) { _pageNumber++; draw(); } };
		void previousPage() { if (_pageNumber>0) { _pageNumber--; draw(); } };
		uint16_t pageNumber() { return _pageNumber; };
		uint16_t pageCount() { return (_packetMonitor.packetLog()->length()+logLines-1) / logLines; };
 
	private:
		elapsedMillis _drawTime;
		uint32_t _bufferHash = 0;
		uint16_t _pageNumber;
};

class SensorButton : public Label {
	public:
		SensorButton(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset, Label* label ) : Label(x, y, w, h, title, scheme, titleInset) {
			_label = label;
		};

		uint32_t sensorID() {
			uint8_t sensorIndex = _title.c_str()[0] - '0';
			
			return _tireHandler.sensorIDs[sensorIndex];
		};

		ButtonScheme* scheme() {
			if (sensorID()) {
				_scheme.textColor = RA8875_GREEN;
			}
			else {
				_scheme.textColor = RA8875_ORANGE;
			}
			return &_scheme;
		};

		String title() {
			uint32_t id = sensorID();
			if (id) {
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "%06X", id);
				return String(buffer);
			}
			else {
				return "Not Paired";
			}
		};

		bool hitTest(tsPoint_t pt, bool widen=false) { return Button::hitTest(pt, widen) || Button::hitTestInternal(pt, _label->rect(), widen); };

	private:
		Label* _label;
};

class Menu {
	public:
		void begin();

		void run(Button** currentMenu=NULL);
		void allowNextRepeat();

		void goBack() { _goBack = true; };
		void prefsDirty() { _prefsDirty = true; };

	private:
		Button** _menuStack[10];
		uint16_t _menuStackIndex = 0;

		bool _goBack = false;
		bool _prefsDirty = false;
};

extern Menu _menu;

extern ButtonScheme backScheme;
extern Button buttonBack;
extern Button buttonDone;

#endif
