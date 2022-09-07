#include "mainMenu.h"

#include "button.h"
#include "defs.h"
#include "pairMenu.h"

#define MIN_PRESSURE 20.0
#define MAX_PRESSURE 125.0
#define MIN_TEMPERATURE 100.0
#define MAX_TEMPERATURE 150.0

ButtonScheme backScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 3, 3 };
ButtonScheme headerScheme = { RA8875_WHITE, RA8875_BLUE, RA8875_BLUE, 4, 3 };

Button buttonBack(0, 408, -30, 58, "Back", backScheme);
Button buttonDone(buttonRightSide, 408, -20, 58, "Done", backScheme);

void buttonRepeat(Menu* menu, Button* button) {
	button->draw(false, true);
	menu->allowNextRepeat();
	menu->prefsDirty();
	delay(buttonFlashTime);
}

bool minPressurePlus(Menu* menu, Button* button) {
	_prefData.alarmPressureMin = min(min(MAX_PRESSURE, _prefData.alarmPressureMax-10.0), _prefData.alarmPressureMin + 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool minPressureMinus(Menu* menu, Button* button) {
	_prefData.alarmPressureMin = _max(MIN_PRESSURE, _prefData.alarmPressureMin - 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool maxPressurePlus(Menu* menu, Button* button) {
	_prefData.alarmPressureMax = min(MAX_PRESSURE, _prefData.alarmPressureMax + 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool maxPressureMinus(Menu* menu, Button* button) {
	_prefData.alarmPressureMax = max(max(MIN_PRESSURE, _prefData.alarmPressureMin+10.0), _prefData.alarmPressureMax - 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool maxTemperaturePlus(Menu* menu, Button* button) {
	_prefData.alarmTempMax = min(MAX_TEMPERATURE, _prefData.alarmTempMax + 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool maxTemperatureMinus(Menu* menu, Button* button) {
	_prefData.alarmTempMax = max(MIN_TEMPERATURE, _prefData.alarmTempMax - 1.0);
	buttonRepeat(menu, button);
	return false;
}

bool doScreenCalibrate(Menu* menu, Button* button) {
	delay(buttonFlashTime);
	doCalibrate();
	return true;
}

bool doSleep(Menu* menu, Button* button) {
	delay(buttonFlashTime);
	sleepUntilTouch();
	return false;
}

bool toggleMute(Menu* menu, SlashButton* button) {
	delay(buttonFlashTime);
	_beeper.setMute(!_beeper.muted());
	button->setState(!_beeper.muted());
	menu->prefsDirty();
	return false;
}

bool menuBack(Menu* menu, Button* button) {
	menu->goBack();
	delay(buttonFlashTime);
	return true;
}

bool menuDone(Menu* menu, Button* button) {
	menu->goBack();
	if (!menu->isAtTopLevel()){
		delay(buttonFlashTime);
	}
	return true;
}

//-------------------------------------------------------
bool doPairSensor(Menu* menu, SensorButton* button) {
	delay(buttonFlashTime);
	runPairMenu(button->sensorIndex());
	return true;
}

//-------------------------------------------------------
bool doLogPrevious(Menu* menu, LogButton* button) {
	delay(buttonFlashTime);
	button->_logView->previousPage();
	menu->redrawInAltLayer();
	buttonRepeat(menu, button);
	return false;
}

bool doLogNext(Menu* menu, LogButton* button) {
	delay(buttonFlashTime);
	button->_logView->nextPage();
	menu->redrawInAltLayer();
	buttonRepeat(menu, button);
	return false;
}

bool doLogFirst(Menu* menu, LogButton* button) {
	delay(buttonFlashTime);
	button->_logView->firstPage();
	menu->redrawInAltLayer();
	buttonRepeat(menu, button);
	return false;
}

LogView::LogView(int16_t x, int16_t y, int16_t w, int16_t h, String title, ButtonScheme& scheme, LogButton* pageNum, LogButton* previous, LogButton* next) : Button(x, y, w, h, title, scheme) {
	_pageNum = pageNum;
	_previous = previous;
	_next = next;

	_pageNum->_logView = this;
	_previous->_logView = this;
	_next->_logView = this;

	_pageNum->touchFunc = (bool(*)(void*, void*))&doLogFirst;
	_previous->touchFunc = (bool(*)(void*, void*))&doLogPrevious;
	_next->touchFunc = (bool(*)(void*, void*))&doLogNext;

	updatePageLabel();
};

void LogView::updatePageLabel() {
	uint16_t pageNum = pageNumber() + 1;
	uint16_t pageTotal = pageCount();
	char buffer[20];

	snprintf(buffer, sizeof(buffer), "Page %d of %d", pageNum, pageTotal);
	_pageNum->setTitle(buffer);
}

void LogView::firstPage() {
	if (_pageNumber > 0) { 
		_pageNumber=0; 
		_dirty = true;
		updatePageLabel(); 
	} 
};

void LogView::nextPage() { 
	if (_pageNumber+1 < pageCount()) { 
		_pageNumber += 1; 
		_dirty = true;
		updatePageLabel(); 
	} 
};

void LogView::previousPage() { 
	if (_pageNumber>0) { 
		_pageNumber -= 1; 
		_dirty = true;
		updatePageLabel(); 
	} 
};

constexpr int16_t logColumns[] = { 64, -304, -432, -592, -736 };
constexpr uint16_t logColumnCount = 5;

uint16_t tabOffset(int16_t tabStop, uint16_t scale, const char* str) {
	if (tabStop >= 0) {
		return tabStop;
	}
	else {
		return -tabStop - _textManager.widthOfString(str, scale);
	}
}

void LogView::draw(bool pressed, bool forceBackground) {
	_drawTime = 0;

	computeScreenRect();

	_textManager.setSpaceNarrowing(false);

	uint16_t count = _packetMonitor.packetLog()->length();

	constexpr const char* headings[] = { "  ID", "Press", "Temp", "Signal", "Age" };

	for (uint16_t i=0; i<logColumnCount; i++) {
		_textManager.drawString(headings[i], _rect.x + tabOffset(logColumns[i], _scheme.sizeX, headings[i]), _rect.y, _scheme.sizeX, _scheme.sizeY, RA8875_GREEN);
	}

	PacketBuff* buffer = _packetMonitor.packetLog();
	int16_t firstLine = _pageNumber * logLines;
	int16_t lastLine = min((uint32_t)(firstLine+logLines), (uint32_t)count);
	uint16_t lineHeight = logLineHeight * _scheme.sizeY;

	// Serial.printf("Draw logView: draw from %d to %d\n", firstLine, lastLine);;

	uint16_t line = 1;
	for (uint16_t i=firstLine; i<lastLine; i++) {
		TPMSPacket packet = buffer->lookup(i);
		uint16_t y = _rect.y + line++ * lineHeight + 3;

		_display.drawFastHLine(_rect.x, y, _rect.w, _scheme.borderColor);
		drawPacket(packet, y+1);
		_touchScreen.touchRefresh();
	}
	_display.drawFastHLine(_rect.x, _rect.y + line * lineHeight + 3, _rect.w, _scheme.borderColor);

	_textManager.setSpaceNarrowing(true);
	_display.textTransparent(RA8875_BLACK);
    _display.graphicsMode();
}

void LogView::drawPacket(TPMSPacket& packet, uint16_t y) {
	char lineBuff[51];

	// Serial.printf("drawPacket: line=%d, id=0x%X06\n", lineNumber, packet.id);

	_display.fillRect(0, y, _display.width(), 15, _scheme.backColor);

	uint32_t time = (millis() - packet.timeStamp) / 1000;
	uint16_t minutes = time / 60;
	uint16_t seconds = time % 60;

	for (uint16_t i=0; i<logColumnCount; i++) {
		switch (i) {
			case 0:
				snprintf(lineBuff, sizeof(lineBuff), "%06X    ", packet.id);
				break;
			case 1:
				snprintf(lineBuff, sizeof(lineBuff), "   %3.0fpsi", packet.pressure);
				break;
			case 2:
				snprintf(lineBuff, sizeof(lineBuff), "  %3.0f\xBA", packet.temperature);
				break;
			case 3:
				snprintf(lineBuff, sizeof(lineBuff), "  %3ddB", packet.rssi);
				break;
			case 4:
				snprintf(lineBuff, sizeof(lineBuff), "  %3dm%02ds", minutes, seconds);
				break;
		}
		_textManager.drawString(lineBuff, _rect.x +  + tabOffset(logColumns[i], _scheme.sizeX, lineBuff), y, _scheme.sizeX, _scheme.sizeY, _scheme.textColor);
	}
}

bool LogView::refresh() {
	bool result = false;
	uint32_t newHash = _packetMonitor.packetLog()->hash();
	if (_drawTime>1000 || _bufferHash != newHash) {
		updatePageLabel(); 
		_bufferHash = newHash;
		result = true;
	}
	return result;
};

void runMainMenu() {
	Menu menu;
	ButtonScheme mainButtonScheme = { RA8875_WHITE, RA8875_GRAY_DK, RA8875_GRAY_LT, 3, 2 };
	ButtonScheme limitsLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2, buttonAlignRight };
	ButtonScheme limitsValueScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 3 };
	ButtonScheme minusButtonScheme = { RA8875_BLACK, RA8875_RED, RA8875_RED, 3, 3 };
	ButtonScheme plusButtonScheme = { RA8875_BLACK, RA8875_GREEN, RA8875_GREEN, 3, 3 };
	ButtonScheme logScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 2, 2 };
	ButtonScheme logLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2 };
	ButtonScheme logButtonScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 3, 3 };
	ButtonScheme sensorLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2, buttonAlignRight };
	ButtonScheme sensorIDScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 2, buttonAlignLeft };
	ButtonScheme systemTextScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2 };

	//-------------------------------------
	Header headerSetAlarms(0,  0, 800, 60, "Set Tire Alarms", headerScheme);

	Label labelMinPressure(0,  120, 400, 50, "Minimum Pressure:", limitsLabelScheme);
	Button buttonMinPressureMinus(440,  120, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMinPressure(500,  120, 170, 50, "%0.0fpsi", limitsValueScheme);
	Button buttonMinPressurePlus(680,  120, 50, 50, "+", plusButtonScheme);

	Label labelMaxPressure(0,  200, 400, 50, "Maximum Pressure:", limitsLabelScheme);
	Button buttonMaxPressureMinus(440,  200, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMaxPressure(500,  200, 170, 50, "%0.0fpsi", limitsValueScheme);
	Button buttonMaxPressurePlus(680,  200, 50, 50, "+", plusButtonScheme);

	Label labelMaxTemperature(0,  280, 400, 50, "Maximum Temperature:", limitsLabelScheme);
	Button buttonMaxTemperatureMinus(440,  280, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMaxTemperature(500,  280, 170, 50, "%0.0f\xBA", limitsValueScheme);
	Button buttonMaxTemperaturePlus(680,  280, 50, 50, "+", plusButtonScheme);

	Button* setAlarmsMenu[] = { &headerSetAlarms, &buttonBack,
										&labelMinPressure, &buttonMinPressureMinus, &valueMinPressure, &buttonMinPressurePlus,
										&labelMaxPressure, &buttonMaxPressureMinus, &valueMaxPressure, &buttonMaxPressurePlus,
										&labelMaxTemperature, &buttonMaxTemperatureMinus, &valueMaxTemperature, &buttonMaxTemperaturePlus,
									NULL };

	//-------------------------------------
	constexpr uint16_t sensTop = 80;
	constexpr uint16_t sensOffset = 50;
	constexpr uint16_t sensInset = 10;
	constexpr uint16_t sensLabelWidth = 300;
	constexpr uint16_t sensButtonWidth = 500;

	Header headerEditSensors(0,  0, 800, 60, "Tap Sensor to Pair", headerScheme);
	TireLabel sensorLabel0(0, sensTop+0*sensOffset, sensLabelWidth, sensOffset, "0:", sensorLabelScheme, sensInset);
	TireLabel sensorLabel1(0, sensTop+1*sensOffset, sensLabelWidth, sensOffset, "1:", sensorLabelScheme, sensInset);
	TireLabel sensorLabel2(0, sensTop+2*sensOffset, sensLabelWidth, sensOffset, "2:", sensorLabelScheme, sensInset);
	TireLabel sensorLabel3(0, sensTop+3*sensOffset, sensLabelWidth, sensOffset, "3:", sensorLabelScheme, sensInset);
	TireLabel sensorLabel4(0, sensTop+4*sensOffset, sensLabelWidth, sensOffset, "4:", sensorLabelScheme, sensInset);
	TireLabel sensorLabel5(0, sensTop+5*sensOffset, sensLabelWidth, sensOffset, "5:", sensorLabelScheme, sensInset);
	SensorButton sensor0(sensLabelWidth, sensTop+0*sensOffset, sensButtonWidth, sensOffset, "0", sensorIDScheme, sensInset, &sensorLabel0);
	SensorButton sensor1(sensLabelWidth, sensTop+1*sensOffset, sensButtonWidth, sensOffset, "1", sensorIDScheme, sensInset, &sensorLabel1);
	SensorButton sensor2(sensLabelWidth, sensTop+2*sensOffset, sensButtonWidth, sensOffset, "2", sensorIDScheme, sensInset, &sensorLabel2);
	SensorButton sensor3(sensLabelWidth, sensTop+3*sensOffset, sensButtonWidth, sensOffset, "3", sensorIDScheme, sensInset, &sensorLabel3);
	SensorButton sensor4(sensLabelWidth, sensTop+4*sensOffset, sensButtonWidth, sensOffset, "4", sensorIDScheme, sensInset, &sensorLabel4);
	SensorButton sensor5(sensLabelWidth, sensTop+5*sensOffset, sensButtonWidth, sensOffset, "5", sensorIDScheme, sensInset, &sensorLabel5);

	Button* editSensorsMenu[] = { &headerEditSensors, &buttonBack, 
										&sensorLabel0, &sensorLabel1, &sensorLabel2, &sensorLabel3, &sensorLabel4, &sensorLabel5,
										&sensor0, &sensor1, &sensor2, &sensor3, &sensor4, &sensor5, NULL };

	//-------------------------------------
	Header headerPacketMonitor(0,  0, 800, 60, "Sensor Log", headerScheme);
	LogButton logPrevious(430, 408, 60, 58, "\x1E", logButtonScheme);
	LogButton logPageLabel(500, 408, 180, 58, "Page %d of %d", logLabelScheme);
	LogButton logNext(690, 408, 60, 58, "\x1F", logButtonScheme);
	LogView logView(0, 72, 800, 333, "LogView", logScheme, &logPageLabel, &logPrevious, &logNext);
	Button* packetMonitorMenu[] = { &headerPacketMonitor, &buttonBack, &logView, &logPrevious, &logPageLabel, &logNext, NULL };

	//-------------------------------------
	Header headerSystemStatus(0,  0, 800, 60, "System Status", headerScheme);
	FloatLabel systemCoordinate(0, 80, 800, 32, "    Lat=%0.5f, Lon=%0.5f, Sats=%0.0f    ", systemTextScheme);
	FloatLabel systemSpeed(0, 120, 800, 32, "    Speed=%0.1f, Heading=%0.1f    ", systemTextScheme);
	FloatLabel systemMotion(0, 160, 800, 32, "     Moving=%0.0fs, Stopped=%0.0fs     ", systemTextScheme);
	VoltageLabel systemVoltage(0, 200, 800, 32, "    Voltage=%0.2f    ", systemTextScheme);
	Label labelTest1(0,  240, 800, 32, "zabcdefghijklmnopqrstuvwxyza", systemTextScheme);
	Label labelTest2(0,  280, 800, 32, "ZABCDEFGHIJKLMNOPQRSTUVWXYZA", systemTextScheme);
	Label labelTest3(0,  320, 800, 32, "Z0.1,2:3;4'5(6)7[8]90Arirlltfifl0717FA", systemTextScheme);
	Label labelTest4(0,  360, 800, 32, "Z01234567890A!B@C#D$E%F&G*H-I/J<K>L", systemTextScheme);
	Button systemSleep(buttonRightSide, 408, -20, 58, "Sleep", backScheme);
	Button* systemStatusMenu[] = { &headerSystemStatus, &buttonBack, &systemSleep, &systemCoordinate,
									&systemSpeed, &systemMotion, &systemVoltage,
									&labelTest1, &labelTest2, &labelTest3, &labelTest4, NULL };

	//-------------------------------------
	constexpr uint16_t mb_y = 82;
	constexpr uint16_t mb_off = 65;
	constexpr uint16_t mb_w = 500;
	constexpr uint16_t mb_h = 48;

	Header headerMain(0,  0, 800, 60, "Main Menu", headerScheme);
	Button buttonSetAlarms(buttonHCenter,  mb_y, mb_w, mb_h, "Set Tire Alarms", mainButtonScheme);
	Button buttonEditSensors(buttonHCenter, mb_y+1*mb_off, mb_w, mb_h, "Pair Sensors", mainButtonScheme);
	Button buttonCalibrate(buttonHCenter, mb_y+2*mb_off, mb_w, mb_h, "Calibrate Screen", mainButtonScheme);
	Button buttonMonitor(buttonHCenter, mb_y+3*mb_off, mb_w, mb_h, "Sensor Log", mainButtonScheme);
	Button buttonStatus(buttonHCenter, mb_y+4*mb_off, mb_w, mb_h, "System Status", mainButtonScheme);
	SlashButton systemMute(0, 408, 70, 58, "\x0E", backScheme);
	Button* mainMenu[] = { &headerMain, &buttonDone, &systemMute, &buttonSetAlarms, &buttonEditSensors, &buttonCalibrate, &buttonMonitor, &buttonStatus, NULL };

	buttonBack.touchFunc = (bool(*)(void*, void*))&menuBack;

	buttonDone.touchFunc = (bool(*)(void*, void*))&menuDone;

	buttonSetAlarms.subButtons = setAlarmsMenu;
	buttonEditSensors.subButtons = editSensorsMenu;
	buttonCalibrate.touchFunc = (bool(*)(void*, void*))&doScreenCalibrate;
	buttonMonitor.subButtons = packetMonitorMenu;
	buttonStatus.subButtons = systemStatusMenu;

	buttonMinPressureMinus.touchFunc = (bool(*)(void*, void*))&minPressureMinus;
	buttonMinPressurePlus.touchFunc = (bool(*)(void*, void*))&minPressurePlus;
	valueMinPressure.setParameter(0, &_prefData.alarmPressureMin);

	buttonMaxPressureMinus.touchFunc = (bool(*)(void*, void*))&maxPressureMinus;
	buttonMaxPressurePlus.touchFunc = (bool(*)(void*, void*))&maxPressurePlus;
	valueMaxPressure.setParameter(0, &_prefData.alarmPressureMax);

	buttonMaxTemperatureMinus.touchFunc = (bool(*)(void*, void*))&maxTemperatureMinus;
	buttonMaxTemperaturePlus.touchFunc = (bool(*)(void*, void*))&maxTemperaturePlus;
	valueMaxTemperature.setParameter(0, &_prefData.alarmTempMax);

	sensor0.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor1.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor2.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor3.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor4.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor5.touchFunc = (bool(*)(void*, void*))&doPairSensor;

	systemCoordinate.setParameter(0, &_gpsData.latitude);
	systemCoordinate.setParameter(1, &_gpsData.longitude);
	systemCoordinate.setParameter(3, &_gpsData.satellites);
	systemSpeed.setParameter(0, &_gpsData.speed);
	systemSpeed.setParameter(1, &_gpsData.heading);
	systemMotion.setParameter(0, &_gpsData.movingSeconds);
	systemMotion.setParameter(1, &_gpsData.stoppedSeconds);
	systemMute.setState(!_beeper.muted());
	systemMute.touchFunc = (bool(*)(void*, void*))toggleMute;
	systemSleep.touchFunc = (bool(*)(void*, void*))&doSleep;

	logView.resetPageNumber();

	menu.run(mainMenu);
}
