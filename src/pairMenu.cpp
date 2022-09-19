#include "pairMenu.h"

#include "mainMenu.h"
#include "tires.h"
#include "beep.h"
#include "packets.h"

extern void checkForNewPackets();
extern String hexTitle(uint32_t id);

class StringHeader : public StringLabel {
	public:
		StringHeader(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0,
				void* param1=NULL, void* param2=NULL, void* param3=NULL, void* param4=NULL ) : 
			StringLabel(x, y, w, h, title, scheme, titleInset) {};
		bool isHeader() { return true; };
};

class IDEntryLabel : public Label {
	public:
		IDEntryLabel(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, uint16_t titleInset=0,
				void* param1=NULL, void* param2=NULL, void* param3=NULL, void* param4=NULL ) : 
			Label(x, y, w, h, title, scheme, titleInset) {};
		void drawTitle(String title, uint16_t x, uint16_t y, uint8_t sizeX, uint8_t sizeY, uint16_t textColor, int32_t backColor) {
			Label::drawTitle(title, x, y, sizeX, sizeY, textColor, backColor);
			if ((_blinkTime % 1000) < 800) {
				uint16_t offset = titleWidth();
				ButtonScheme sc = scheme();

				_display.fillRect(_rect.x + _titleInset + offset, _rect.y + 6, 8*sc.sizeX - 8, 16*sc.sizeY - 10, sc.textColor);
			}
		};
		ButtonScheme scheme(bool pressed = false) {
			ButtonScheme sc = Label::scheme(pressed);

			if (title().length()==6) {
				sc.borderColor = RA8875_GREEN;
			}
			else {
				sc.borderColor = RA8875_RED;
			}
			return sc;
		};
		uint8_t refresh() { 
			checkForNewPackets();
			return buttonRefreshRedraw;
		};
		void setTitle(String title) {
			Label::setTitle(title);
			_blinkTime = 0;
		};
	private:
		elapsedMillis _blinkTime;
};

class RecentSensorButton : public Button {
	public:
		RecentSensorButton(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme,
				void* param1=NULL, void* param2=NULL, void* param3=NULL, void* param4=NULL ) : 
			Button(x, y, w, h, title, scheme) {};
		bool visible() { return title().length() > 0; };
		void setTitle(String title) {
			_title = title;
			_dirty = true; 
			_setTime = 0;
		};
		ButtonScheme scheme(bool pressed = false) {
			ButtonScheme sc = Button::scheme(pressed);

			if (_setTime < 500 ) {
				sc.borderColor = RA8875_GREEN;
				sc.textColor = RA8875_ORANGE;
			}
			return sc;
		};
	private:
		elapsedMillis _setTime;
};

IDEntryLabel* entryField;
Button* pairButton;
bool performPair = false;
RecentSensorButton* sensorButtons[4];
uint16_t sensorButtonIndex = 0;
uint32_t pairTimeStamp;

void checkForNewPackets() {
	PacketBuff* log = _packetMonitor.packetLog();
	uint16_t numPackets = log->length();
	bool done = false;

	for (uint16_t i=0; !done && i<numPackets; i++) {
		TPMSPacket packet = log->lookup(i);

		if (packet.timeStamp > pairTimeStamp) {
			sensorButtons[sensorButtonIndex]->setTitle(hexTitle(packet.id));
			sensorButtonIndex = (sensorButtonIndex+1) % 4;
		}
		else {
			done = true;
		}
	}
	pairTimeStamp = millis();
}

bool doEntryKey(Menu* menu, Button* button) {
	button->draw(true, true);

	String currentStr = entryField->title();

	if (currentStr.length()<6) {
		entryField->setTitle(currentStr + button->title());
		entryField->draw(false, true);
		delay(200);
	}
	else {
		_beeper.beep(200);
	}

	pairButton->setVisible(entryField->title().length()==6);
	pairButton->draw(false, true);

	return false;
}

bool doBackspace(Menu* menu, Button* button) {
	String currentStr = entryField->title();

	button->draw(true, true);

	if (currentStr.length()>0) {
		entryField->setTitle(currentStr.substring(0, currentStr.length()-1));
		pairButton->hide();
		pairButton->draw(false, true);
		menu->allowNextRepeat();
	}
	else {
		_beeper.beep(200);
	}

	delay(buttonFlashTime);
	return false;
}

bool doClear(Menu* menu, Button* button) {
	button->draw(true, true);
	pairButton->hide();
	pairButton->draw(false, true);
	entryField->setTitle("");
	delay(buttonFlashTime);
	return false;
}

bool doSensorButton(Menu* menu, Button* button) {
	button->draw(true, true);
	pairButton->show();
	pairButton->draw(false, true);
	entryField->setTitle(button->title());
	entryField->draw(false, true);
	delay(buttonFlashTime);
	return false;
}

bool doPair(Menu* menu, Button* button) {
	performPair = true;
	menu->goBack();
	return false;
}

String hexTitle(uint32_t id) {
	char buffer[10];
	snprintf(buffer, sizeof(buffer), "%06X", id);
	return buffer;
}

void runPairMenu(uint16_t index) {
	pairTimeStamp = millis();
	sensorButtonIndex = 0;

	ButtonScheme sensorLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 3 };
	ButtonScheme sensorButtonScheme = { RA8875_YELLOW, RA8875_GRAY_DK, RA8875_WHITE, 3, 3 };
	ButtonScheme sensorKeyScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 3, 3 };
	ButtonScheme IDEntryScheme = { RA8875_YELLOW, RA8875_BLACK, RA8875_GREEN, 4, 3, buttonAlignLeft };
	ButtonScheme clearScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 2, 2 };
	ButtonScheme backspaceScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 4, 4 };
	ButtonScheme pairSensorScheme = { RA8875_WHITE, RA8875_GREEN, RA8875_GREEN, 3, 3 };

	ButtonScheme sc = headerScheme;

	sc.sizeX = 3;

	StringHeader headerPairSensor(0,  0, 800, 60, "Pair %s Tire", sc);

	String tireName = _tireHandler.tireName(index);
	headerPairSensor.setParameter(0, tireName.c_str());

	Serial.println(tireName);

	constexpr uint16_t sensLeft = 440;
	constexpr uint16_t sensTop = 148;
	constexpr uint16_t sensWidth = 360;
	constexpr uint16_t sensHeight = 50;
	constexpr uint16_t sensVGap = 14;

	Label labelChooseSensor(sensLeft,  80, sensWidth, 50, "Recent Sensors", sensorLabelScheme);

	for (uint16_t i=0; i<4; i++) {
		sensorButtons[i] = new RecentSensorButton(sensLeft+70, sensTop+i*(sensHeight+sensVGap), sensWidth-70*2, sensHeight, "", sensorButtonScheme);
		sensorButtons[i]->touchFunc = (bool(*)(void*, void*))&doSensorButton;
	}

	constexpr uint16_t keysLeft = 20;
	constexpr uint16_t keysTop = 148;
	constexpr uint16_t keysWidth = 100;
	constexpr uint16_t keysHeight = 60;

	IDEntryLabel IDEntryField(keysLeft+80,  80, 240, 50, "", IDEntryScheme);

	IDEntryField.setTitleInset(14);
	entryField = &IDEntryField;

	uint32_t id = _prefData.sensorIDs[index];

	if (id != 0) {
		IDEntryField.setTitle(hexTitle(id));
	}

	Button buttonClear(keysLeft, 80, 80-16, 50, "X", clearScheme);
	Button buttonBackspace(keysLeft+320+16, 80, 80-16, 50, "\x1B", backspaceScheme);

	buttonClear.touchFunc = (bool(*)(void*, void*))&doClear;
	buttonBackspace.touchFunc = (bool(*)(void*, void*))&doBackspace;

	Button* keys[16];
	const char keyLabels[] = "0123456789ABCDEF";

	for (uint16_t i=0; i<16; i++) {
		keys[i] = new Button(keysLeft+(i%4)*keysWidth,  keysTop+(i/4)*keysHeight, keysWidth+2, keysHeight+2, String(keyLabels[i]), sensorKeyScheme);
		keys[i]->touchFunc = (bool(*)(void*, void*))&doEntryKey;
	}

	Button buttonPair(buttonRightSide, 408, -20, 58, "Pair Sensor", pairSensorScheme);
	buttonPair.touchFunc = (bool(*)(void*, void*))&doPair;
	buttonPair.hide();

	pairButton = &buttonPair;

	Button* pairSensorMenu[] = { &headerPairSensor, &buttonBack,
									&labelChooseSensor,
									sensorButtons[0], sensorButtons[1], sensorButtons[2], sensorButtons[3],
									&IDEntryField, &buttonBackspace, &buttonClear,
									keys[0], keys[1], keys[2], keys[3], keys[4], keys[5], keys[6], keys[7],
									keys[8], keys[9], keys[10], keys[11], keys[12], keys[13], keys[14], keys[15],
									&buttonPair,
									NULL };

	Menu menu;

	menu.run(pairSensorMenu);

	if (performPair) {
		int32_t id = -1;
		String str = IDEntryField.title();

		if (str.length()==6) {
			const char* chars = str.c_str();
			uint32_t value = 0;
			
			for (uint16_t i=0; i<6; i++) {
				uint8_t c = chars[i];

				if (c>='0' && c<='9') {
					c = c - '0';
				}
				else {
					c = c - 'A' + 10;
				}

				value |= c << ((5-i)*4);
			}
			id = value;
		}

		if (id != -1 && id != 0) {
			_tireHandler.setSensorID(index, id);
		}
		else {
			_beeper.beep(200);
		}
	}
}
