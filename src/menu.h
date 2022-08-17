#ifndef DEF_MENU_H
#define DEF_MENU_H

#include "elapsedMillis.h"
#include "button.h"
#include "touchscreen.h"
#include "packets.h"
#include "tires.h"

extern void packetCheck();

#define ColorRA8875(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

constexpr uint16_t RA8875_GRAY_DK = 0b0101001010101010;
constexpr uint16_t RA8875_GRAY_LT = 0b1010010100010100;

constexpr uint16_t logLines = 8;
constexpr uint16_t logLineHeight = 17;

class LogView : public Button {
	public:
		LogView(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : Button(x, y, w, h, title, scheme) { };
		void setPacketLog(PacketBuff* buffer) { _buffer = buffer; };
		bool hitTest(tsPoint_t pt, bool widen=false) { return false; };
		void draw(bool pressed=false, bool forceBackground=false);
		void drawPacket(TPMS_Packet& packet, uint16_t lineNumber);
        void refresh();
		void firstPage() { if (_pageNumber > 0) { _pageNumber==0; draw(); } };
		void nextPage() { if (_pageNumber+1 < pageCount()) { _pageNumber++; draw(); } };
		void previousPage() { if (_pageNumber>0) { _pageNumber--; draw(); } };
		uint16_t pageNumber() { return _pageNumber; };
		uint16_t pageCount() { return (_buffer->length()+logLines-1) / logLines; };
 
	private:
		PacketBuff* _buffer;
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
			
			if (_tireHandler) {
				return _tireHandler->sensorIDs[sensorIndex];
			}
			else {
				return 0;
			}
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

		void setTireHandler(TireHandler* tireHandler) { _tireHandler = tireHandler; };
		bool hitTest(tsPoint_t pt, bool widen=false) { return Button::hitTest(pt, widen) || Button::hitTestInternal(pt, _label->rect(), widen); };

	private:
		TireHandler* _tireHandler;
		Label* _label;
};

class Menu {
	public:
		void begin(Adafruit_RA8875* tft, TouchScreen* touchScreen, PacketMonitor* packetMonitor, TireHandler* tireHandler, float* minPressure, float* maxPressure, float* maxTemperature);

		void run(Button** currentMenu=NULL);
		void allowNextRepeat();

		bool _goBack = false;
		bool _prefsDirty = false;
		float* _minPressure;
		float* _maxPressure;
		float* _maxTemperature;

	private:
		Adafruit_RA8875* _tft;
		TouchScreen* _touchScreen;
		PacketMonitor* _packetMonitor;
		TireHandler* _tireHandler;

		Button** _menuStack[10];
		uint16_t _menuStackIndex = 0;

};

#endif
