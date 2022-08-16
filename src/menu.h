#ifndef DEF_MENU_H
#define DEF_MENU_H

#include "elapsedMillis.h"
#include "button.h"
#include "touchscreen.h"
#include "packets.h"

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

class Menu {
	public:
		void begin(Adafruit_RA8875* tft, TouchScreen* touchScreen, PacketMonitor* packetMonitor, float* minPressure, float* maxPressure, float* maxTemperature);

		void run();
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

		Button** _menuStack[10];
		uint16_t _menuStackIndex = 0;

};

#endif
