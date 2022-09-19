#include "mainMenu.h"

#include "button.h"
#include "defs.h"
#include "pairMenu.h"

#define MIN_PRESSURE 20.0
#define MAX_PRESSURE 125.0
#define MIN_TEMPERATURE 100.0
#define MAX_TEMPERATURE 150.0

constexpr uint16_t lowerRowV = 408;

ButtonScheme backScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 3, 3 };
ButtonScheme headerScheme = { RA8875_WHITE, RA8875_BLUE, RA8875_BLUE, 4, 3 };

Button buttonBack(0, lowerRowV, -30, 58, "Back", backScheme);
Button buttonDone(buttonRightSide, lowerRowV, -20, 58, "Done", backScheme);

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
	uint16_t pageTotal = max((uint16_t)1, pageCount());
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

constexpr int16_t logColumns[] = { -110, 110, -340, -460, -624, -770 };
constexpr uint16_t logColumnCount = 6;

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

	constexpr const char* headings[] = { "Tire  ", "  ID", "Press", "Temp", "Signal", "Age" };

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
		uint16_t sizeX = _scheme.sizeX;
		uint16_t otherColors[4];

		switch (i) {
			case 0: {
					int16_t index = _tireHandler.indexOfSensor(packet.id);
					if (index != -1) {
						snprintf(lineBuff, sizeof(lineBuff), "%s  ", _tireHandler.tireName(index, true));
					}
					else {
						lineBuff[0] = 0;
					}
				}
				break;
			case 1:
				snprintf(lineBuff, sizeof(lineBuff), "%06X    ", packet.id);
				break;
			case 2:
				snprintf(lineBuff, sizeof(lineBuff), "   %3.0fpsi", packet.pressure);
				break;
			case 3:
				snprintf(lineBuff, sizeof(lineBuff), "  %3.0f\xBA", packet.temperature);
				break;
			case 4:
				snprintf(lineBuff, sizeof(lineBuff), "  %3ddB", packet.rssi);
				break;
			case 5:
				snprintf(lineBuff, sizeof(lineBuff), "  %3dm%02ds", minutes, seconds);
				break;
		}
		_textManager.drawString(lineBuff, _rect.x +  + tabOffset(logColumns[i], sizeX, lineBuff), y, sizeX, _scheme.sizeY, _scheme.textColor, -1, otherColors);
	}
}

uint8_t LogView::refresh() {
	bool result = false;
	uint32_t newHash = _packetMonitor.packetLog()->hash();
	if (_drawTime>1000 || _bufferHash != newHash) {
		updatePageLabel(); 
		_bufferHash = newHash;
		result = true;
	}
	return result ? buttonRefreshRedraw : buttonRefreshNone;
};

void AccelView::drawBackground(bool drawCursor) {
	uint16_t rowHeight = _rect.h / 3;
	ButtonScheme sc = scheme();

	if (drawCursor) {
		_display.drawFastHLine(_rect.x, _rect.y + rowHeight, _rect.w, _scheme.borderColor);
		_display.drawFastHLine(_rect.x, _rect.y + rowHeight*2, _rect.w, _scheme.borderColor);
		_display.drawFastVLine(_rect.x+_cursorX, _rect.y, _rect.h, RA8875_WHITE);
	}
	else {
		_display.drawFastVLine(_rect.x+_cursorX, _rect.y, _rect.h, RA8875_BLACK);
		_display.drawPixel(_rect.x+_cursorX, _rect.y+rowHeight, _scheme.borderColor);
		_display.drawPixel(_rect.x+_cursorX, _rect.y+rowHeight*2, _scheme.borderColor);
	}

	_textManager.drawString("X", _rect.x+4, _rect.y+4, sc.sizeX, sc.sizeY, sc.textColor);
	_textManager.drawString("Y", _rect.x+4, _rect.y+4+rowHeight, sc.sizeX, sc.sizeY, sc.textColor);
	_textManager.drawString("Z", _rect.x+4, _rect.y+4+2*rowHeight, sc.sizeX, sc.sizeY, sc.textColor);

}

void AccelView::draw(bool pressed, bool forceBackground) {
    computeScreenRect();
	_cursorX = 0;
	drawBackground(true);
}

constexpr float accelYScale = 9.8 * 1.0;

uint8_t AccelView::refresh() {
	uint16_t newSamples = _accel._xRecents->sampleCount();

	if (newSamples) {
		newSamples = 1;
		uint16_t rowHeight = _rect.h / 3;
		uint16_t halfHeight = rowHeight / 2;
		ButtonScheme sc = scheme();
		RingBuff<float>* recents[3] = { _accel._xRecents, _accel._yRecents, _accel._zRecents };
		float offsets[3] = { 0.0, 0.0, -9.8 };
		float averages[3] = { _accel._xShakeBuff->average(), _accel._yShakeBuff->average(), _accel._zShakeBuff->average() };

		drawBackground(false);

		if (_accel.didShake()) {
			_display.drawFastVLine(_rect.x+_cursorX, _rect.y, 4, RA8875_RED);
		}

		for (uint16_t j=0; j<3; j++) {
			int16_t height = (averages[j] + offsets[j]) / accelYScale * halfHeight;
			_display.drawFastHLine(_rect.x+_cursorX, _rect.y+j*rowHeight+halfHeight+height, newSamples, RA8875_GRAY_DK);
		}

		for (uint16_t i=0; i<newSamples; i++) {
			for (uint16_t j=0; j<3; j++) {
				float sample = recents[j]->nextSample() + offsets[j];
				int16_t height = sample / accelYScale * halfHeight;
				uint16_t x = (_rect.x+_cursorX+i) % _rect.w;

				_display.drawPixel(x, _rect.y+j*rowHeight+halfHeight+height, RA8875_CYAN);
			}
		}

		_cursorX = (_cursorX + newSamples) % _rect.w;
		_display.drawFastVLine(_rect.x+_cursorX, _rect.y, _rect.h, RA8875_WHITE);
	}

	return buttonRefreshFast;
};

void menuInit() {
	buttonBack.touchFunc = (bool(*)(void*, void*))&menuBack;
	buttonDone.touchFunc = (bool(*)(void*, void*))&menuBack;
}

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
	ButtonScheme infoTextScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2 };
	ButtonScheme systemTextScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 2 };

	constexpr uint16_t gapS = buttonVPriorSame;
	constexpr uint16_t gap0 = buttonVPriorBelow;
	constexpr uint16_t gap8 = buttonVPriorBelow | 8;

	//-------------------------------------
	Header headerSetAlarms(0,  0, 800, 60, "Set Tire Alarms", headerScheme);

	Label labelMinPressure(0,  120, 400, 50, "Minimum Pressure:", limitsLabelScheme);
	Button buttonMinPressureMinus(440,  gapS, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMinPressure(500,  gapS, 170, 50, "%0.0fpsi", limitsValueScheme);
	Button buttonMinPressurePlus(680,  gapS, 50, 50, "+", plusButtonScheme);

	Label labelMaxPressure(0,  200, 400, 50, "Maximum Pressure:", limitsLabelScheme);
	Button buttonMaxPressureMinus(440,  gapS, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMaxPressure(500,  gapS, 170, 50, "%0.0fpsi", limitsValueScheme);
	Button buttonMaxPressurePlus(680,  gapS, 50, 50, "+", plusButtonScheme);

	Label labelMaxTemperature(0,  280, 400, 50, "Maximum Temperature:", limitsLabelScheme);
	Button buttonMaxTemperatureMinus(440,  gapS, 50, 50, "-", minusButtonScheme);
	FloatLabel valueMaxTemperature(500,  gapS, 170, 50, "%0.0f\xBA", limitsValueScheme);
	Button buttonMaxTemperaturePlus(680,  gapS, 50, 50, "+", plusButtonScheme);

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
	TireLabel sensorLabel0(0, sensTop, sensLabelWidth, sensOffset, "0:", sensorLabelScheme, sensInset);
	SensorButton sensor0(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "0", sensorIDScheme, sensInset, &sensorLabel0);
	TireLabel sensorLabel1(0, gap0, sensLabelWidth, sensOffset, "1:", sensorLabelScheme, sensInset);
	SensorButton sensor1(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "1", sensorIDScheme, sensInset, &sensorLabel1);
	TireLabel sensorLabel2(0, gap0, sensLabelWidth, sensOffset, "2:", sensorLabelScheme, sensInset);
	SensorButton sensor2(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "2", sensorIDScheme, sensInset, &sensorLabel2);
	TireLabel sensorLabel3(0, gap0, sensLabelWidth, sensOffset, "3:", sensorLabelScheme, sensInset);
	SensorButton sensor3(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "3", sensorIDScheme, sensInset, &sensorLabel3);
	TireLabel sensorLabel4(0, gap0, sensLabelWidth, sensOffset, "4:", sensorLabelScheme, sensInset);
	SensorButton sensor4(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "4", sensorIDScheme, sensInset, &sensorLabel4);
	TireLabel sensorLabel5(0, gap0, sensLabelWidth, sensOffset, "5:", sensorLabelScheme, sensInset);
	SensorButton sensor5(sensLabelWidth, gapS, sensButtonWidth, sensOffset, "5", sensorIDScheme, sensInset, &sensorLabel5);

	Button* editSensorsMenu[] = { &headerEditSensors, &buttonBack, 
										&sensorLabel0, &sensor0, &sensorLabel1, &sensor1, &sensorLabel2, &sensor2, &sensorLabel3, &sensor3,
										&sensorLabel4, &sensor4, &sensorLabel5, &sensor5,
										NULL };

	//-------------------------------------
	Header headerPacketMonitor(0,  0, 800, 60, "Sensor Log", headerScheme);
	LogButton logPrevious(430, lowerRowV, 60, 58, "\x1E", logButtonScheme);
	LogButton logPageLabel(500, lowerRowV, 180, 58, "Page %d of %d", logLabelScheme);
	LogButton logNext(690, lowerRowV, 60, 58, "\x1F", logButtonScheme);
	LogView logView(0, 72, 800, 333, "LogView", logScheme, &logPageLabel, &logPrevious, &logNext);
	Button* packetMonitorMenu[] = { &headerPacketMonitor, &buttonBack, &logView, &logPrevious, &logPageLabel, &logNext, NULL };

	//-------------------------------------
	constexpr uint16_t mb_y = 82;
	constexpr uint16_t mb_w = 500;
	constexpr uint16_t mb_h = 48;
	constexpr uint16_t mb_gap = buttonVPriorBelow | 17;

	Header headerSystemInfo(0,  0, 800, 60, "System Info", headerScheme);
	Button buttonGPSInfo(buttonHCenter,  mb_y, mb_w, mb_h, "GPS", mainButtonScheme);
	Button buttonAccelInfo(buttonHCenter, mb_gap, mb_w, mb_h, "Accelerometer", mainButtonScheme);
	Button buttonFontInfo(buttonHCenter, mb_gap, mb_w, mb_h, "Fonts", mainButtonScheme);
	VoltageLabel systemVoltage(100, lowerRowV, 600, 58, "    Voltage=%0.2f    ", systemTextScheme);
	Button systemSleep(buttonRightSide, lowerRowV, -20, 58, "Sleep", backScheme);
	Button* systemInfoMenu[] = { &headerSystemInfo, &buttonBack, &buttonGPSInfo, &buttonAccelInfo, &buttonFontInfo, &systemVoltage, &systemSleep, NULL };

	//-------------------------------------
	Header headerFontStatus(0,  0, 800, 60, "Font Test", headerScheme);
	Label labelTest1(0,  80, 800, 32, "zabcdefghijklmnopqrstuvwxyza", infoTextScheme);
	Label labelTest2(0,  gap8, 800, 32, "ZABCDEFGHIJKLMNOPQRSTUVWXYZA", infoTextScheme);
	Label labelTest3(0,  gap8, 800, 32, "Z01234567890A", infoTextScheme);
	Label labelTest4(0,  gap8, 800, 32, "Z!@#$%^&*-=_+:;'\",./<>?(){}[]\\|A", infoTextScheme);
	Label labelTest5(0,  gap8, 800, 32, "Z!B@C#D$E%F&G*H-I/J<K>L(M)N{O}P[Q]/R_S-W?X", infoTextScheme);
	Label labelTest6(0,  gap8, 800, 32, "Z0.1,2:3;4'5(6)7[8]90Arirlltfifl0717FA", infoTextScheme);
	Button* fontInfoMenu[] = { &headerFontStatus, &buttonBack, &labelTest1, &labelTest2, &labelTest3, &labelTest4, &labelTest5, &labelTest6, NULL };

	//-------------------------------------
	Header headerGPSStatus(0,  0, 800, 60, "GPS Status", headerScheme);
	FloatLabel gpsCoordinate(0, 80, 800, 32, "    Lat=%0.5f, Lon=%0.5f    ", infoTextScheme);
	FloatLabel gpsSatellites(0, gap8, 800, 32, "    Satellites=%0.0f    ", infoTextScheme);
	FloatLabel gpsSpeed(0, gap8, 800, 32, "    Speed=%0.1f, Heading=%0.1f    ", infoTextScheme);
	FloatLabel gpsMotion(0, gap8, 800, 32, "     Moving=%0.0fs, Stopped=%0.0fs     ", infoTextScheme);
	Button* gpsInfoMenu[] = { &headerGPSStatus, &buttonBack, &gpsCoordinate, &gpsSatellites, &gpsSpeed, &gpsMotion, NULL };

	//-------------------------------------
	Header headerAccelMonitor(0,  0, 800, 60, "Accelerometer Data", headerScheme);
	AccelView accelView(0, 72, 800, 333, "AccelView", logScheme);
	Button* accelInfoMenu[] = { &headerAccelMonitor, &buttonBack, &accelView, NULL };

	//-------------------------------------
	Header headerMain(0,  0, 800, 60, "Main Menu", headerScheme);
	Button buttonSetAlarms(buttonHCenter,  mb_y, mb_w, mb_h, "Set Tire Alarms", mainButtonScheme);
	Button buttonEditSensors(buttonHCenter, mb_gap, mb_w, mb_h, "Pair Sensors", mainButtonScheme);
	Button buttonCalibrate(buttonHCenter, mb_gap, mb_w, mb_h, "Calibrate Screen", mainButtonScheme);
	Button buttonMonitor(buttonHCenter, mb_gap, mb_w, mb_h, "Sensor Log", mainButtonScheme);
	Button buttonStatus(buttonHCenter, mb_gap, mb_w, mb_h, "System Info", mainButtonScheme);
	SlashButton systemMute(365, lowerRowV, 70, 58, "\x0E", backScheme);
	Button* mainMenu[] = { &headerMain, &buttonDone, &systemMute, &buttonSetAlarms, &buttonEditSensors, &buttonCalibrate, &buttonMonitor, &buttonStatus, NULL };

	buttonSetAlarms.subButtons = setAlarmsMenu;
	buttonEditSensors.subButtons = editSensorsMenu;
	buttonCalibrate.touchFunc = (bool(*)(void*, void*))&doScreenCalibrate;
	buttonMonitor.subButtons = packetMonitorMenu;
	buttonStatus.subButtons = systemInfoMenu;

	buttonGPSInfo.subButtons = gpsInfoMenu;
	buttonAccelInfo.subButtons = accelInfoMenu;
	buttonFontInfo.subButtons = fontInfoMenu;

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

	gpsCoordinate.setParameter(0, &_gpsData.latitude);
	gpsCoordinate.setParameter(1, &_gpsData.longitude);
	gpsSatellites.setParameter(0, &_gpsData.satellites);
	gpsSpeed.setParameter(0, &_gpsData.speed);
	gpsSpeed.setParameter(1, &_gpsData.heading);
	gpsMotion.setParameter(0, &_gpsData.movingSeconds);
	gpsMotion.setParameter(1, &_gpsData.stoppedSeconds);

	systemMute.setState(!_beeper.muted());
	systemMute.touchFunc = (bool(*)(void*, void*))toggleMute;
	systemSleep.touchFunc = (bool(*)(void*, void*))&doSleep;

	logView.resetPageNumber();

	menu.run(mainMenu);
}
