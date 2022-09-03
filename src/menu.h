#ifndef DEF_MENU_H
#define DEF_MENU_H

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
        bool refresh();
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
		bool refresh() {
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

		uint32_t sensorID() {
			uint8_t sensorIndex = _title.c_str()[0] - '0';
			
			return _prefData.sensorIDs[sensorIndex];
		};

		ButtonScheme scheme(bool pressed = false) {
			ButtonScheme sc = Label::scheme(pressed);

			if (sensorID()) {
				sc.textColor = RA8875_GRAY_LT;
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
				uint8_t index = _title.c_str()[0] - '0';
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

		void performDraw(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor) {
			_textManager.drawString(title, x, y, sizeX, sizeY, textColor, backColor, _colors);
		}

		bool hitTest(tsPoint_t pt, bool widen=false) { return Button::hitTest(pt, widen) || Button::hitTestInternal(pt, _label->rect(), widen); };

	private:
		Label* _label;
		uint16_t _colors[2];
};

class Menu {
	public:
		void begin();

		void run(Button** currentMenu=NULL);
		void allowNextRepeat();

		void goBack() { _goBack = true; };
		void redrawInAltLayer() { _redrawAlt = true; };
		void prefsDirty() { _prefsDirty = true; _prefsDirtyTime = 0; };

	private:
		Button** _menuStack[10];
		uint16_t _menuStackIndex = 0;

		bool _goBack = false;
		bool _prefsDirty = false;
		bool _redrawAlt = false;
		elapsedMillis _prefsDirtyTime;
};

extern Menu _menu;

extern ButtonScheme backScheme;
extern Button buttonBack;
extern Button buttonDone;

#endif
