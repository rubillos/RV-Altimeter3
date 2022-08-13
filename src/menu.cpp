#include "menu.h"

#include "elapsedMillis.h"
#include "touchscreen.h"
#include <Preferences.h>

extern void doCalibrate();
extern void wait_tft_done();

#define MIN_PRESSURE 60.0
#define MAX_PRESSURE 125.0
#define MIN_TEMPERATURE 100.0
#define MAX_TEMPERATURE 140.0

ButtonScheme backScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_GRAY_LT, 3 };
ButtonScheme headerScheme = { RA8875_WHITE, RA8875_BLUE, RA8875_BLUE, 3, buttonsNoClick | buttonIsHeader };
ButtonScheme mainButtonScheme = { RA8875_BLACK, RA8875_GRAY_DK, RA8875_GRAY_LT, 2 };
ButtonScheme limitsLabelScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 2, buttonsNoClick | buttonAlignRight };
ButtonScheme limitsValueScheme = { RA8875_WHITE, RA8875_BLACK, RA8875_BLACK, 3, buttonsNoClick };
ButtonScheme minusButtonScheme = { RA8875_BLACK, RA8875_RED, RA8875_RED, 3 };
ButtonScheme plusButtonScheme = { RA8875_BLACK, RA8875_GREEN, RA8875_GREEN, 3 };

Button button_back(0, 406, -30, 60, "Back", backScheme);
Button button_done(buttonRightSide, 406, -30, 60, "Done", backScheme);

//-------------------------------------
Button header_setAlarms(0,  0, 800, 60, "Set Tire Alarms", headerScheme);

Button label_minPressure(0,  120, 400, 50, "Minimum Pressure:", limitsLabelScheme);
Button button_minPressureMinus(440,  120, 50, 50, "-", minusButtonScheme);
Button value_minPressure(500,  120, 170, 50, "", limitsValueScheme);
Button button_minPressurePlus(680,  120, 50, 50, "+", plusButtonScheme);

Button label_maxPressure(0,  200, 400, 50, "Maximum Pressure:", limitsLabelScheme);
Button button_maxPressureMinus(440,  200, 50, 50, "-", minusButtonScheme);
Button value_maxPressure(500,  200, 170, 50, "", limitsValueScheme);
Button button_maxPressurePlus(680,  200, 50, 50, "+", plusButtonScheme);

Button label_maxTemperature(0,  280, 400, 50, "Maximum Temperature:", limitsLabelScheme);
Button button_maxTemperatureMinus(440,  280, 50, 50, "-", minusButtonScheme);
Button value_maxTemperature(500,  280, 170, 50, "", limitsValueScheme);
Button button_maxTemperaturePlus(680,  280, 50, 50, "+", plusButtonScheme);

Button* setAlarmsMenu[] = { &header_setAlarms, &button_back,
									&label_minPressure, &button_minPressureMinus, &value_minPressure, &button_minPressurePlus,
									&label_maxPressure, &button_maxPressureMinus, &value_maxPressure, &button_maxPressurePlus,
									&label_maxTemperature, &button_maxTemperatureMinus, &value_maxTemperature, &button_maxTemperaturePlus,
								NULL };

//-------------------------------------
Button header_EditSensors(0,  0, 800, 60, "Edit Sensors", headerScheme);
Button* editSensorsMenu[] = { &header_EditSensors, &button_back, NULL };

//-------------------------------------
Button header_packetMonitor(0,  0, 800, 60, "PacketMonitor", headerScheme);
Button* packetMonitorMenu[] = { &header_packetMonitor, &button_back, NULL };

//-------------------------------------
Button header_SystemStatus(0,  0, 800, 60, "PacketMonitor", headerScheme);
Button* systemStatusMenu[] = { &header_SystemStatus, &button_back, NULL };

//-------------------------------------
constexpr uint16_t mb_y = 82;
constexpr uint16_t mb_off = 65;
constexpr uint16_t mb_w = 400;
constexpr uint16_t mb_h = 48;

Button header_main(0,  0, 800, 60, "Main Menu", headerScheme);
Button button_setAlarms(buttonHCenter,  mb_y, mb_w, mb_h, "Set Tire Alarms", mainButtonScheme);
Button button_editSensors(buttonHCenter, mb_y+1*mb_off, mb_w, mb_h, "Edit Sensors", mainButtonScheme);
Button button_calibrate(buttonHCenter, mb_y+2*mb_off, mb_w, mb_h, "Calibrate Screen", mainButtonScheme);
Button button_monitor(buttonHCenter, mb_y+3*mb_off, mb_w, mb_h, "Packet Monitor", mainButtonScheme);
Button button_status(buttonHCenter, mb_y+4*mb_off, mb_w, mb_h, "System Status", mainButtonScheme);
Button* mainMenu[] = { &header_main, &button_done, &button_setAlarms, &button_editSensors, &button_calibrate, &button_monitor, &button_status, NULL };

void setMinPressureTitle(Menu* menu) {
	value_minPressure.setTitle(String(*menu->_minPressure, 0)+"psi");
}

void setMaxPressureTitle(Menu* menu) {
	value_maxPressure.setTitle(String(*menu->_maxPressure, 0)+"psi");
}

void setMaxTemperatureTitle(Menu* menu) {
	value_maxTemperature.setTitle(String(*menu->_maxTemperature, 0)+"\xBA");
}

bool minPressurePlus(Menu* menu) {
	*menu->_minPressure = min(min(MAX_PRESSURE, *menu->_maxPressure-10.0), *menu->_minPressure + 1.0);
	setMinPressureTitle(menu);
	value_minPressure.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool minPressureMinus(Menu* menu) {
	*menu->_minPressure = _max(MIN_PRESSURE, *menu->_minPressure - 1.0);
	setMinPressureTitle(menu);
	value_minPressure.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool maxPressurePlus(Menu* menu) {
	*menu->_maxPressure = min(MAX_PRESSURE, *menu->_maxPressure + 1.0);
	setMaxPressureTitle(menu);
	value_maxPressure.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool maxPressureMinus(Menu* menu) {
	*menu->_maxPressure = max(max(MIN_PRESSURE, *menu->_minPressure+10.0), *menu->_maxPressure - 1.0);
	setMaxPressureTitle(menu);
	value_maxPressure.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool maxTemperaturePlus(Menu* menu) {
	*menu->_maxTemperature = min(MAX_TEMPERATURE, *menu->_maxTemperature + 1.0);
	setMaxTemperatureTitle(menu);
	value_maxTemperature.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool maxTemperatureMinus(Menu* menu) {
	*menu->_maxTemperature = max(MIN_TEMPERATURE, *menu->_maxTemperature - 1.0);
	setMaxTemperatureTitle(menu);
	value_maxTemperature.draw(false, true);
	allowNextRepeat();
	delay(200);
	return false;
}

bool doScreenCalibrate(Menu* menu) {
	delay(200);
	doCalibrate();
	return true;
}

bool menuBack(Menu* menu) {
	menu->_goBack = true;
	delay(200);
	return true;
}

bool menuDone(Menu* menu) {
	menu->_goBack = true;
	return true;
}

void Menu::begin(Adafruit_RA8875* tft, float* minPressure, float* maxPressure, float* maxTemperature, tsMatrix_t* calibration) {
	_tft = tft;
	_minPressure = minPressure;
	_maxPressure = maxPressure;
	_maxTemperature = maxTemperature;
	_calibration = calibration;

	button_back.touchFunc = (bool(*)(void*))&menuBack;
	button_back.setTFT(tft);

	button_done.touchFunc = (bool(*)(void*))&menuDone;

	button_setAlarms.subButtons = setAlarmsMenu;
	button_editSensors.subButtons = editSensorsMenu;
	button_calibrate.touchFunc = (bool(*)(void*))&doScreenCalibrate;
	button_monitor.subButtons = packetMonitorMenu;
	button_status.subButtons = systemStatusMenu;

	button_minPressureMinus.touchFunc = (bool(*)(void*))&minPressureMinus;
	button_minPressurePlus.touchFunc = (bool(*)(void*))&minPressurePlus;

	button_maxPressureMinus.touchFunc = (bool(*)(void*))&maxPressureMinus;
	button_maxPressurePlus.touchFunc = (bool(*)(void*))&maxPressurePlus;

	button_maxTemperatureMinus.touchFunc = (bool(*)(void*))&maxTemperatureMinus;
	button_maxTemperaturePlus.touchFunc = (bool(*)(void*))&maxTemperaturePlus;

	setMinPressureTitle(this);
	setMaxPressureTitle(this);
	setMaxTemperatureTitle(this);
}

void Menu::run() {
	Button** currentMenu = mainMenu;
	bool done = false;
	bool needUpdate = true;
	static elapsedMillis updateTime;

	while (!done) {
		tsPoint_t touchPt;

		if (!needUpdate && checkScreenTouch(&touchPt, _calibration)) {
			Serial.println("Menu: screen touched.");
			Button* button = hitButton(currentMenu, touchPt, true);

			if (button) {
				Serial.println("Menu: found button.");
				if (button->touchFunc) {
					_goBack = false;
					bool update = button->touchFunc(this);
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
			}
		}

		// if (needUpdate || updateTime > 500) {
		if (needUpdate) {
			needUpdate = false;
			updateTime = 0;

			if ((*currentMenu)->isHeader()) {
				uint16_t height = (*currentMenu)->height();
				_tft->fillRect(0, height, _tft->width(), _tft->height()-height, RA8875_BLACK);
			}
			else {
				_tft->fillScreen(RA8875_BLACK);
			}
			wait_tft_done();
			drawButtons(currentMenu);
		}
	}
}
