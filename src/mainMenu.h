#ifndef DEF_MAINMENU_H
#define DEF_MAINMENU_H

#include "elapsedMillis.h"
#include "button.h"
#include "touchscreen.h"
#include "textManager.h"
#include "tires.h"
#include "packets.h"
#include "tires.h"
#include "gps.h"
#include "prefs.h"
#include "accel.h"
#include "menu.h"
#include "accel.h"

constexpr uint16_t logLines = 8;
constexpr uint16_t logLineHeight = 17;

class LogButton;

class LogView : public Button {
	public:
		LogView(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, LogButton* pageNum, LogButton* previous, LogButton* next);
		bool hitTest(tsPoint_t pt, uint16_t widen=0) { return false; };
		void draw(bool pressed=false, bool forceBackground=false);
		void drawPacket(TPMSPacket& packet, uint16_t lineNumber);
        uint8_t refresh();
		void firstPage();
		void nextPage();
		void previousPage();
		void updatePageLabel();
		uint16_t pageNumber() { return _pageNumber; };
		void resetPageNumber() { _pageNumber = 0; };
		uint16_t pageCount() { 
			if (_packetMonitor.packetLog()) {
				return (_packetMonitor.packetLog()->length()+logLines-1) / logLines; 
			}
			else {
				return 1;
			}
		};
 
	private:
		elapsedMillis _drawTime;
		uint32_t _bufferHash = 0;
		uint16_t _pageNumber = 0;

		LogButton* _pageNum;
		LogButton* _previous;
		LogButton* _next; 
};

class LogButton : public Button {
	public:
		LogButton(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : Button(x, y, w, h, title, scheme) {};

		LogView* _logView;
};

class TireLabel : public Label {
	public:
		TireLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset = 0) : Label(x, y, w, h, title, scheme, titleInset) {};
		String title() {
			_mergedTitle = String(_tireHandler.tireName(_title.c_str()[0] - '0')) + String(&_title.c_str()[1]);
			return _mergedTitle.c_str();
		}

	private:
		String _mergedTitle;
};

class VoltageLabel : public FloatLabel {
	public:
		VoltageLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset = 0) :
									FloatLabel(x, y, w, h, title, scheme, titleInset) {
										setParameter(0, &_voltage);
										_voltage = voltageValue();
									};
		uint8_t refresh() {
			_voltage = voltageValue();
			return FloatLabel::refresh();
		};

		String title() {
			updateValues(false);
			if (*_param[0]<5.0) {
				String str = String(_title);

				str.replace("%0.2f", "USB");
				return str;
			}
			return FloatLabel::title();
		};

	private:
		float _voltage;
};

class SensorButton : public Label {
	public:
		SensorButton(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset, Label* label ) : Label(x, y, w, h, title, scheme, titleInset) {
			_label = label;
		};

		uint16_t sensorIndex() {
			return _title.c_str()[0] - '0';
		};

		uint32_t sensorID() {
			return _prefData.sensorIDs[sensorIndex()];
		};

		ButtonScheme scheme(bool pressed = false) {
			ButtonScheme sc = Label::scheme(pressed);

			if (sensorID()) {
				sc.textColor = RA8875_YELLOW;
			}
			else {
				sc.textColor = RA8875_ORANGE;
			}
			return sc;
		};

		//         00011122
		// 0000011111122222

		uint16_t Color8to16(uint8_t color) {
			return ((color & 0xE0) << 8) | ((color & 0x1C) << 6) | ((color & 0x03) << 3);
		};

		String title() {
			uint32_t id = sensorID();
			if (id) {
				uint8_t index = sensorIndex();
				String pressure = _tireHandler.pressureStringForSensor(index, true);
				String temperature = _tireHandler.temperatureStringForSensor(index, 2);

				_colors[0] = Color8to16(_tireHandler.pressureColorForSensor(index));
				_colors[1] = Color8to16(_tireHandler.temperatureColorForSensor(index));

				char buffer[30];
				snprintf(buffer, sizeof(buffer), "%06X  (\b1%s\b0,\b2%s\b0)", id, pressure, temperature);
				return String(buffer);
			}
			else {
				return "Not Paired";
			}
		};

		void drawTitle(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor) {
			_textManager.drawString(title, x, y, sizeX, sizeY, textColor, backColor, _colors);
		}

		bool hitTest(tsPoint_t pt, uint16_t widen=0) { return Button::hitTest(pt, widen) || Button::hitTestInternal(pt, _label->rect(), widen); };

		uint8_t refresh() {
			return buttonRefreshRedraw;
		};


	private:
		Label* _label;
		uint16_t _colors[2];
};

class AccelView : public Button {
	public:
		AccelView(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme) : Button(x, y, w, h, title, scheme) {};
		bool hitTest(tsPoint_t pt, uint16_t widen=0) { return false; };
		void draw(bool pressed=false, bool forceBackground=false);
		void drawBackground(bool drawCursor);
        uint8_t refresh();

	private:
		uint16_t _cursorX;
};

extern void menuInit();
extern void runMainMenu();

extern ButtonScheme backScheme;
extern ButtonScheme headerScheme;
extern Button buttonBack;
extern Button buttonDone;

#endif
