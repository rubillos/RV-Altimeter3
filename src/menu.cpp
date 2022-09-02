#include "menu.h"

#include "elapsedMillis.h"
#include "touchscreen.h"
#include "beep.h"
#include "defs.h"

extern void doCalibrate();

constexpr uint32_t prefWriteTime = 2000;

#define MIN_PRESSURE 20.0
#define MAX_PRESSURE 125.0
#define MIN_TEMPERATURE 100.0
#define MAX_TEMPERATURE 150.0

ButtonScheme backScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_LT, 3, 3 };
ButtonScheme headerScheme = { RA8875_WHITE, RA8875_BLUE, RA8875_BLUE, 4, 3 };
ButtonScheme mainButtonScheme = { RA8875_GREEN, RA8875_GRAY_DK, RA8875_GRAY_LT, 3, 2 };
ButtonScheme limitsLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2, buttonAlignRight };
ButtonScheme limitsValueScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 3 };
ButtonScheme minusButtonScheme = { RA8875_BLACK, RA8875_RED, RA8875_RED, 3, 3 };
ButtonScheme plusButtonScheme = { RA8875_BLACK, RA8875_GREEN, RA8875_GREEN, 3, 3 };
ButtonScheme logScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_DK, 2, 2 };
ButtonScheme sensorLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2, buttonAlignRight };
ButtonScheme sensorIDScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, 2, buttonAlignLeft };
ButtonScheme systemTextScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, 2 };

Button buttonBack(0, 408, -20, 58, "Back", backScheme);
Button buttonDone(buttonRightSide, 408, -20, 58, "Done", backScheme);

//-------------------------------------
Header headerSetAlarms(0,  0, 800, 60, "Set Tire Alarms", headerScheme);

Label labelMinPressure(0,  120, 400, 50, "Minimum Pressure:", limitsLabelScheme);
Button buttonMinPressureMinus(440,  120, 50, 50, "-", minusButtonScheme);
Label valueMinPressure(500,  120, 170, 50, "", limitsValueScheme);
Button buttonMinPressurePlus(680,  120, 50, 50, "+", plusButtonScheme);

Label labelMaxPressure(0,  200, 400, 50, "Maximum Pressure:", limitsLabelScheme);
Button buttonMaxPressureMinus(440,  200, 50, 50, "-", minusButtonScheme);
Label valueMaxPressure(500,  200, 170, 50, "", limitsValueScheme);
Button buttonMaxPressurePlus(680,  200, 50, 50, "+", plusButtonScheme);

Label labelMaxTemperature(0,  280, 400, 50, "Maximum Temperature:", limitsLabelScheme);
Button buttonMaxTemperatureMinus(440,  280, 50, 50, "-", minusButtonScheme);
Label valueMaxTemperature(500,  280, 170, 50, "", limitsValueScheme);
Button buttonMaxTemperaturePlus(680,  280, 50, 50, "+", plusButtonScheme);

Button* setAlarmsMenu[] = { &headerSetAlarms, &buttonBack,
									&labelMinPressure, &buttonMinPressureMinus, &valueMinPressure, &buttonMinPressurePlus,
									&labelMaxPressure, &buttonMaxPressureMinus, &valueMaxPressure, &buttonMaxPressurePlus,
									&labelMaxTemperature, &buttonMaxTemperatureMinus, &valueMaxTemperature, &buttonMaxTemperaturePlus,
								NULL };

//-------------------------------------
constexpr uint16_t sensor_top = 80;
constexpr uint16_t sensor_offset = 50;
constexpr uint16_t sensor_inset = 20;

Header headerEditSensors(0,  0, 800, 60, "Tap Sensor to Pair", headerScheme);
Label sensorLabel0(100,  sensor_top, 300, sensor_offset, "Left Font:", sensorLabelScheme, sensor_inset);
Label sensorLabel1(100, sensor_top+1*sensor_offset, 300, sensor_offset, "Right Front:", sensorLabelScheme, sensor_inset);
Label sensorLabel2(100, sensor_top+2*sensor_offset, 300, sensor_offset, "Left Rear Inner:", sensorLabelScheme, sensor_inset);
Label sensorLabel3(100, sensor_top+3*sensor_offset, 300, sensor_offset, "Right Rear Inner:", sensorLabelScheme, sensor_inset);
Label sensorLabel4(100, sensor_top+4*sensor_offset, 300, sensor_offset, "Left Rear Outer:", sensorLabelScheme, sensor_inset);
Label sensorLabel5(100, sensor_top+5*sensor_offset, 300, sensor_offset, "Right Rear Outer:", sensorLabelScheme, sensor_inset);
SensorButton sensor0(400, sensor_top, 300, sensor_offset, "0", sensorIDScheme, sensor_inset, &sensorLabel0);
SensorButton sensor1(400, sensor_top+1*sensor_offset, 300, sensor_offset, "1", sensorIDScheme, sensor_inset, &sensorLabel1);
SensorButton sensor2(400, sensor_top+2*sensor_offset, 300, sensor_offset, "2", sensorIDScheme, sensor_inset, &sensorLabel2);
SensorButton sensor3(400, sensor_top+3*sensor_offset, 300, sensor_offset, "3", sensorIDScheme, sensor_inset, &sensorLabel3);
SensorButton sensor4(400, sensor_top+4*sensor_offset, 300, sensor_offset, "4", sensorIDScheme, sensor_inset, &sensorLabel4);
SensorButton sensor5(400, sensor_top+5*sensor_offset, 300, sensor_offset, "5", sensorIDScheme, sensor_inset, &sensorLabel5);

Button* editSensorsMenu[] = { &headerEditSensors, &buttonDone, 
									&sensorLabel0, &sensorLabel1, &sensorLabel2, &sensorLabel3, &sensorLabel4, &sensorLabel5,
									&sensor0, &sensor1, &sensor2, &sensor3, &sensor4, &sensor5, NULL };

//-------------------------------------
Header headerPacketMonitor(0,  0, 800, 60, "Sensor Log", headerScheme);
LogView logView_packetLog(0, 77, 800, 333, "LogView", logScheme);
Button* packetMonitorMenu[] = { &headerPacketMonitor, &buttonBack, &logView_packetLog, NULL };

//-------------------------------------
Header headerSystemStatus(0,  0, 800, 60, "System Status", headerScheme);
FloatLabel systemCoordinate(0, 80, 800, 32, "    Lat=%0.5f, Lon=%0.5f    ", systemTextScheme);
FloatLabel systemSpeed(0, 120, 800, 32, "    Speed=%0.1f, Heading=%0.1f    ", systemTextScheme);
FloatLabel systemSatellites(0, 160, 800, 32, "    Satellites=%0.0f    ", systemTextScheme);
FloatLabel systemMotion(0, 200, 800, 32, "    Moving=%0.0f, Stopped=%0.0f    ", systemTextScheme);
Button systemSleep(buttonRightSide, 408, -20, 58, "Sleep", backScheme);
Button* systemStatusMenu[] = { &headerSystemStatus, &buttonBack, &systemSleep, &systemCoordinate, &systemSpeed, &systemSatellites, &systemMotion, NULL };

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

void setMinPressureTitle(Menu* menu) {
	valueMinPressure.setTitle(String(_prefData.alarmPressureMin, 0)+"psi");
}

void setMaxPressureTitle(Menu* menu) {
	valueMaxPressure.setTitle(String(_prefData.alarmPressureMax, 0)+"psi");
}

void setMaxTemperatureTitle(Menu* menu) {
	valueMaxTemperature.setTitle(String(_prefData.alarmTempMax, 0)+"\xBA");
}

void buttonRepeat(Menu* menu, Button& button) {
	button.draw(false, true);
	menu->allowNextRepeat();
	menu->prefsDirty();
	delay(200);
}

bool minPressurePlus(Menu* menu, Button* button) {
	_prefData.alarmPressureMin = min(min(MAX_PRESSURE, _prefData.alarmPressureMax-10.0), _prefData.alarmPressureMin + 1.0);
	setMinPressureTitle(menu);
	buttonRepeat(menu, valueMinPressure);
	return false;
}

bool minPressureMinus(Menu* menu, Button* button) {
	_prefData.alarmPressureMin = _max(MIN_PRESSURE, _prefData.alarmPressureMin - 1.0);
	setMinPressureTitle(menu);
	buttonRepeat(menu, valueMinPressure);
	return false;
}

bool maxPressurePlus(Menu* menu, Button* button) {
	_prefData.alarmPressureMax = min(MAX_PRESSURE, _prefData.alarmPressureMax + 1.0);
	setMaxPressureTitle(menu);
	buttonRepeat(menu, valueMaxPressure);
	return false;
}

bool maxPressureMinus(Menu* menu, Button* button) {
	_prefData.alarmPressureMax = max(max(MIN_PRESSURE, _prefData.alarmPressureMin+10.0), _prefData.alarmPressureMax - 1.0);
	setMaxPressureTitle(menu);
	buttonRepeat(menu, valueMaxPressure);
	return false;
}

bool maxTemperaturePlus(Menu* menu, Button* button) {
	_prefData.alarmTempMax = min(MAX_TEMPERATURE, _prefData.alarmTempMax + 1.0);
	setMaxTemperatureTitle(menu);
	buttonRepeat(menu, valueMaxTemperature);
	return false;
}

bool maxTemperatureMinus(Menu* menu, Button* button) {
	_prefData.alarmTempMax = max(MIN_TEMPERATURE, _prefData.alarmTempMax - 1.0);
	setMaxTemperatureTitle(menu);
	buttonRepeat(menu, valueMaxTemperature);
	return false;
}

bool doScreenCalibrate(Menu* menu, Button* button) {
	delay(200);
	doCalibrate();
	return true;
}

bool doSleep(Menu* menu, Button* button) {
	delay(200);
	sleepUntilTouch();
	return false;
}

bool toggleMute(Menu* menu, SlashButton* button) {
	delay(200);
	_beeper.setMute(!_beeper.muted());
	button->setState(!_beeper.muted());
	menu->prefsDirty();
	return false;
}

bool menuBack(Menu* menu, Button* button) {
	menu->goBack();
	delay(200);
	return true;
}

bool menuDone(Menu* menu, Button* button) {
	menu->goBack();
	return true;
}

//-------------------------------------------------------
bool doPairSensor(Menu* menu, SensorButton* button) {
	delay(200);
	return true;
}

//-------------------------------------------------------
void LogView::draw(bool pressed, bool forceBackground) {
	_drawTime = 0;

	computeScreenRect();

	uint16_t count = _packetMonitor.packetLog()->length();

	_display.textMode();
	_display.textEnlarge(_scheme.sizeX-1, _scheme.sizeY-1);
	_display.textTransparent(RA8875_GREEN);

	_display.textSetCursor(_rect.x, _rect.y);
	_display.textWrite("         ID       Press    Temp       Age");

	if (count) {
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
		}

	}
	_display.textTransparent(RA8875_BLACK);
    _display.graphicsMode();
}

void LogView::drawPacket(TPMSPacket& packet, uint16_t y) {
	char lineBuff[51];

	// Serial.printf("drawPacket: line=%d, id=0x%X06\n", lineNumber, packet.id);

    _display.textSetCursor(_rect.x, y);
	_display.textColor(_scheme.textColor, _scheme.backColor);

	uint32_t time = (millis() - packet.timeStamp) / 1000;
	uint16_t minutes = time / 60;
	uint16_t seconds = time % 60;

	snprintf(lineBuff, sizeof(lineBuff), "       %06X    %3.0fpsi    %3.0f\xBA     %3d:%02d        ", packet.id, packet.pressure, packet.temperature, minutes, seconds);

    _display.textWrite(lineBuff);
}

void LogView::refresh() {
	uint32_t newHash = _packetMonitor.packetLog()->hash();
	if (_drawTime>1000 || _bufferHash != newHash) {
		_bufferHash = newHash;
		draw(false, true);
	}
};

//-------------------------------------------------------
void Menu::allowNextRepeat() {
	_touchScreen.allowNextRepeat();
}

void Menu::begin() {
	buttonBack.touchFunc = (bool(*)(void*, void*))&menuBack;

	buttonDone.touchFunc = (bool(*)(void*, void*))&menuDone;

	buttonSetAlarms.subButtons = setAlarmsMenu;
	buttonEditSensors.subButtons = editSensorsMenu;
	buttonCalibrate.touchFunc = (bool(*)(void*, void*))&doScreenCalibrate;
	buttonMonitor.subButtons = packetMonitorMenu;
	buttonStatus.subButtons = systemStatusMenu;

	buttonMinPressureMinus.touchFunc = (bool(*)(void*, void*))&minPressureMinus;
	buttonMinPressurePlus.touchFunc = (bool(*)(void*, void*))&minPressurePlus;

	buttonMaxPressureMinus.touchFunc = (bool(*)(void*, void*))&maxPressureMinus;
	buttonMaxPressurePlus.touchFunc = (bool(*)(void*, void*))&maxPressurePlus;

	buttonMaxTemperatureMinus.touchFunc = (bool(*)(void*, void*))&maxTemperatureMinus;
	buttonMaxTemperaturePlus.touchFunc = (bool(*)(void*, void*))&maxTemperaturePlus;

	sensor0.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor1.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor2.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor3.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor4.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor5.touchFunc = (bool(*)(void*, void*))&doPairSensor;

	setMinPressureTitle(this);
	setMaxPressureTitle(this);
	setMaxTemperatureTitle(this);

	systemCoordinate.setParameter(0, &_gpsData.latitude);
	systemCoordinate.setParameter(1, &_gpsData.longitude);
	systemSpeed.setParameter(0, &_gpsData.speed);
	systemSpeed.setParameter(1, &_gpsData.heading);
	systemSatellites.setParameter(0, &_gpsData.satellites);
	systemMotion.setParameter(0, &_gpsData.movingSeconds);
	systemMotion.setParameter(1, &_gpsData.stoppedSeconds);
	systemMute.setState(!_beeper.muted());
	systemMute.touchFunc = (bool(*)(void*, void*))toggleMute;
	systemSleep.touchFunc = (bool(*)(void*, void*))&doSleep;
}

void Menu::run(Button** currentMenu) {
	if (currentMenu==NULL) {
		currentMenu = mainMenu;
	}
	
	bool done = false;
	bool needUpdate = true;
	static elapsedMillis updateTime;

	while (!done) {
		tsPoint_t touchPt;
		
		if (!needUpdate && _touchScreen.screenTouch(&touchPt)) {
			Button* button = hitButton(currentMenu, touchPt, true);
			
			if (button) {
				if (button->touchFunc) {
					_goBack = false;
					bool update = button->touchFunc(this, button);
					if (_goBack) {
						if (_menuStackIndex > 0) {
							currentMenu = _menuStack[--_menuStackIndex];
							needUpdate = true;
						}
						else {
							done = true;
						}
					}
					else if (update) {
						needUpdate = true;
					}
					else {
						button->draw(false, true);
					}
				}
				else if (button->subButtons) {
					_menuStack[_menuStackIndex++] = currentMenu;
					currentMenu = button->subButtons;
					needUpdate = true;
					delay(200);
				}
				_display.textTransparent(RA8875_BLACK);
			}
		}

		// if (needUpdate || updateTime > 500) {
		if (needUpdate) {
			needUpdate = false;
			updateTime = 0;

			if ((*currentMenu)->isHeader()) {
				uint16_t height = (*currentMenu)->height();
				_display.fillRect(0, height, _display.width(), _display.height()-height, RA8875_BLACK);
			}
			else {
				_display.fillScreen(RA8875_BLACK);
			}
			drawButtons(currentMenu);
			_display.textTransparent(RA8875_BLACK);
		}

		packetCheck();
		_gps.update();
		_accel.update();

		Button** list = currentMenu;
		while (*list) {
			(*list)->refresh();
			list++;
		}

		if (_prefsDirty && _prefsDirtyTime > prefWriteTime) {
			_prefs.writePrefs();
			_prefsDirty = false;
		}

		static elapsedMillis showTime;
		if (showTime > 1000) {
			Serial.printf("%06d: Menu: free memory=%d\n", millis(), ESP.getFreeHeap());
			showTime = 0;
		}
	}
	if (_prefsDirty) {
		_prefs.writePrefs();
	}
}

Menu _menu;
