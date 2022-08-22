#include "menu.h"

#include "elapsedMillis.h"
#include "touchscreen.h"
#include "defs.h"

extern void doCalibrate();

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

Button buttonBack(0, 408, -20, 58, "Back", backScheme);
Button buttonDone(buttonRightSide, 408, -20, 58, "Done", backScheme);

//-------------------------------------
Header header_setAlarms(0,  0, 800, 60, "Set Tire Alarms", headerScheme);

Label label_minPressure(0,  120, 400, 50, "Minimum Pressure:", limitsLabelScheme);
Button buttonMinPressureMinus(440,  120, 50, 50, "-", minusButtonScheme);
Label value_minPressure(500,  120, 170, 50, "", limitsValueScheme);
Button buttonMinPressurePlus(680,  120, 50, 50, "+", plusButtonScheme);

Label label_maxPressure(0,  200, 400, 50, "Maximum Pressure:", limitsLabelScheme);
Button buttonMaxPressureMinus(440,  200, 50, 50, "-", minusButtonScheme);
Label value_maxPressure(500,  200, 170, 50, "", limitsValueScheme);
Button buttonMaxPressurePlus(680,  200, 50, 50, "+", plusButtonScheme);

Label label_maxTemperature(0,  280, 400, 50, "Maximum Temperature:", limitsLabelScheme);
Button buttonMaxTemperatureMinus(440,  280, 50, 50, "-", minusButtonScheme);
Label value_maxTemperature(500,  280, 170, 50, "", limitsValueScheme);
Button buttonMaxTemperaturePlus(680,  280, 50, 50, "+", plusButtonScheme);

Button* setAlarmsMenu[] = { &header_setAlarms, &buttonBack,
									&label_minPressure, &buttonMinPressureMinus, &value_minPressure, &buttonMinPressurePlus,
									&label_maxPressure, &buttonMaxPressureMinus, &value_maxPressure, &buttonMaxPressurePlus,
									&label_maxTemperature, &buttonMaxTemperatureMinus, &value_maxTemperature, &buttonMaxTemperaturePlus,
								NULL };

//-------------------------------------
constexpr uint16_t sensor_top = 80;
constexpr uint16_t sensor_offset = 50;
constexpr uint16_t sensor_inset = 20;

Header header_EditSensors(0,  0, 800, 60, "Tap Sensor to Pair", headerScheme);
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

Button* editSensorsMenu[] = { &header_EditSensors, &buttonDone, 
									&sensorLabel0, &sensorLabel1, &sensorLabel2, &sensorLabel3, &sensorLabel4, &sensorLabel5,
									&sensor0, &sensor1, &sensor2, &sensor3, &sensor4, &sensor5, NULL };

//-------------------------------------
Header header_packetMonitor(0,  0, 800, 60, "Sensor Log", headerScheme);
LogView logView_packetLog(0, 77, 800, 333, "LogView", logScheme);
Button* packetMonitorMenu[] = { &header_packetMonitor, &buttonBack, &logView_packetLog, NULL };

//-------------------------------------
Header header_SystemStatus(0,  0, 800, 60, "System Status", headerScheme);
Button* systemStatusMenu[] = { &header_SystemStatus, &buttonBack, NULL };

//-------------------------------------
constexpr uint16_t mb_y = 82;
constexpr uint16_t mb_off = 65;
constexpr uint16_t mb_w = 500;
constexpr uint16_t mb_h = 48;

Header header_main(0,  0, 800, 60, "Main Menu", headerScheme);
Button buttonSetAlarms(buttonHCenter,  mb_y, mb_w, mb_h, "Set Tire Alarms", mainButtonScheme);
Button buttonEditSensors(buttonHCenter, mb_y+1*mb_off, mb_w, mb_h, "Pair Sensors", mainButtonScheme);
Button buttonCalibrate(buttonHCenter, mb_y+2*mb_off, mb_w, mb_h, "Calibrate Screen", mainButtonScheme);
Button buttonMonitor(buttonHCenter, mb_y+3*mb_off, mb_w, mb_h, "Sensor Log", mainButtonScheme);
Button buttonStatus(buttonHCenter, mb_y+4*mb_off, mb_w, mb_h, "System Status", mainButtonScheme);
Button* mainMenu[] = { &header_main, &buttonDone, &buttonSetAlarms, &buttonEditSensors, &buttonCalibrate, &buttonMonitor, &buttonStatus, NULL };

void setMinPressureTitle(Menu* menu) {
	value_minPressure.setTitle(String(*menu->_minPressure, 0)+"psi");
}

void setMaxPressureTitle(Menu* menu) {
	value_maxPressure.setTitle(String(*menu->_maxPressure, 0)+"psi");
}

void setMaxTemperatureTitle(Menu* menu) {
	value_maxTemperature.setTitle(String(*menu->_maxTemperature, 0)+"\xBA");
}

bool minPressurePlus(Menu* menu, Button* button) {
	*menu->_minPressure = min(min(MAX_PRESSURE, *menu->_maxPressure-10.0), *menu->_minPressure + 1.0);
	setMinPressureTitle(menu);
	value_minPressure.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool minPressureMinus(Menu* menu, Button* button) {
	*menu->_minPressure = _max(MIN_PRESSURE, *menu->_minPressure - 1.0);
	setMinPressureTitle(menu);
	value_minPressure.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool maxPressurePlus(Menu* menu, Button* button) {
	*menu->_maxPressure = min(MAX_PRESSURE, *menu->_maxPressure + 1.0);
	setMaxPressureTitle(menu);
	value_maxPressure.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool maxPressureMinus(Menu* menu, Button* button) {
	*menu->_maxPressure = max(max(MIN_PRESSURE, *menu->_minPressure+10.0), *menu->_maxPressure - 1.0);
	setMaxPressureTitle(menu);
	value_maxPressure.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool maxTemperaturePlus(Menu* menu, Button* button) {
	*menu->_maxTemperature = min(MAX_TEMPERATURE, *menu->_maxTemperature + 1.0);
	setMaxTemperatureTitle(menu);
	value_maxTemperature.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool maxTemperatureMinus(Menu* menu, Button* button) {
	*menu->_maxTemperature = max(MIN_TEMPERATURE, *menu->_maxTemperature - 1.0);
	setMaxTemperatureTitle(menu);
	value_maxTemperature.draw(false, true);
	menu->allowNextRepeat();
	menu->_prefsDirty = true;
	delay(200);
	return false;
}

bool doScreenCalibrate(Menu* menu, Button* button) {
	delay(200);
	doCalibrate();
	return true;
}

bool menuBack(Menu* menu, Button* button) {
	menu->_goBack = true;
	delay(200);
	return true;
}

bool menuDone(Menu* menu, Button* button) {
	menu->_goBack = true;
	return true;
}

//-------------------------------------------------------
bool doPairSensor(Menu* menu, SensorButton* button) {

	return true;
}

//-------------------------------------------------------

void LogView::draw(bool pressed, bool forceBackground) {
	_drawTime = 0;

	computeScreenRect();

	uint16_t count = _buffer->length();

	_tft.textMode();
	_tft.textEnlarge(_scheme.sizeX-1, _scheme.sizeY-1);
	_tft.textTransparent(RA8875_GREEN);

	_tft.textSetCursor(_rect.x, _rect.y);
	_tft.textWrite("         ID       Press    Temp       Age");

	if (count) {
		int16_t firstLine = _pageNumber * logLines;
		int16_t lastLine = min((uint32_t)(firstLine+logLines), (uint32_t)count);
		uint16_t lineHeight = logLineHeight * _scheme.sizeY;

		// Serial.printf("Draw logView: draw from %d to %d\n", firstLine, lastLine);;

		uint16_t line = 1;
		for (uint16_t i=firstLine; i<lastLine; i++) {
			TPMSPacket packet = _buffer->lookup(i);
			uint16_t y = _rect.y + line++ * lineHeight + 3;

			_tft.drawFastHLine(_rect.x, y, _rect.w, _scheme.borderColor);
			drawPacket(packet, y+1);
		}

	}
	_tft.textTransparent(RA8875_BLACK);
    _tft.graphicsMode();
}

void LogView::drawPacket(TPMSPacket& packet, uint16_t y) {
	char lineBuff[51];

	// Serial.printf("drawPacket: line=%d, id=0x%X06\n", lineNumber, packet.id);

    _tft.textSetCursor(_rect.x, y);
	_tft.textColor(_scheme.textColor, _scheme.backColor);

	uint32_t time = (millis() - packet.timeStamp) / 1000;
	uint16_t minutes = time / 60;
	uint16_t seconds = time % 60;

	snprintf(lineBuff, sizeof(lineBuff), "       %06X    %3.0fpsi    %3.0f\xBA     %3d:%02d        ", packet.id, packet.pressure, packet.temperature, minutes, seconds);

    _tft.textWrite(lineBuff);
}

void LogView::refresh() {
	if (_drawTime>1000 || _bufferHash!=_buffer->hash()) {
		_bufferHash = _buffer->hash();
		draw(false, true);
	}
};

//-------------------------------------------------------
void Menu::allowNextRepeat() {
	_touchScreen->allowNextRepeat();
}

void Menu::begin(TouchScreen *touchScreen, PacketMonitor* packetMonitor, TireHandler* tireHandler, float* minPressure, float* maxPressure, float* maxTemperature) {
	_packetMonitor = packetMonitor;
	_touchScreen = touchScreen;
	_tireHandler = tireHandler;

	_minPressure = minPressure;
	_maxPressure = maxPressure;
	_maxTemperature = maxTemperature;

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

	logView_packetLog.setPacketLog(packetMonitor->packetLog());

	sensor0.setTireHandler(tireHandler);
	sensor1.setTireHandler(tireHandler);
	sensor2.setTireHandler(tireHandler);
	sensor3.setTireHandler(tireHandler);
	sensor4.setTireHandler(tireHandler);
	sensor5.setTireHandler(tireHandler);

	sensor0.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor1.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor2.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor3.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor4.touchFunc = (bool(*)(void*, void*))&doPairSensor;
	sensor5.touchFunc = (bool(*)(void*, void*))&doPairSensor;

	setMinPressureTitle(this);
	setMaxPressureTitle(this);
	setMaxTemperatureTitle(this);
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
		
		if (!needUpdate && _touchScreen->screenTouch(&touchPt)) {
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
				_tft.textTransparent(RA8875_BLACK);
			}
		}

		// if (needUpdate || updateTime > 500) {
		if (needUpdate) {
			needUpdate = false;
			updateTime = 0;

			if ((*currentMenu)->isHeader()) {
				uint16_t height = (*currentMenu)->height();
				_tft.fillRect(0, height, _tft.width(), _tft.height()-height, RA8875_BLACK);
			}
			else {
				_tft.fillScreen(RA8875_BLACK);
			}
			drawButtons(currentMenu);
			_tft.textTransparent(RA8875_BLACK);
		}

		packetCheck();
		Button** list = currentMenu;
		while (*list) {
			(*list)->refresh();
			list++;
		}

		static elapsedMillis showTime;
		if (showTime > 1000) {
			Serial.printf("%06d: Menu: free memory=%d\n", millis(), ESP.getFreeHeap());
			showTime = 0;
		}
	}
	if (_prefsDirty) {
		writePrefs();
	}
}
