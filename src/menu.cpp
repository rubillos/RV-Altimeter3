#include "menu.h"

#include "elapsedMillis.h"
#include "touchscreen.h"
#include "textManager.h"
#include "packets.h"
#include "dataDisplay.h"
#include "beep.h"
#include "defs.h"

extern void doCalibrate();

constexpr uint32_t prefWriteTime = 2000;
constexpr uint32_t refreshRate = 200;

//-------------------------------------------------------
void Menu::allowNextRepeat() {
	_touchScreen.allowNextRepeat();
}

void Menu::run(Button** currentMenu) {
	bool done = false;
	bool needUpdate = true;
	static elapsedMillis refreshTime;

	while (!done) {
		tsPoint_t touchPt;
		
		if (!needUpdate && _touchScreen.screenTouch(&touchPt)) {
			Button* button = hitButton(currentMenu, touchPt, true);
			
			if (button) {
				// Serial.printf("Button: %s\n", button->title().c_str());
				
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
					delay(buttonFlashTime);
				}
				_display.textTransparent(RA8875_BLACK);
			}
		}

		if (needUpdate || _redrawAlt) {
			needUpdate = false;

			if (_redrawAlt) {
				_dataDisplay.drawAltLayer();
			}

			if ((*currentMenu)->isHeader()) {
				uint16_t height = (*currentMenu)->height();
				_display.fillRect(0, height, _display.width(), _display.height()-height, RA8875_BLACK);
			}
			else {
				_display.fillScreen(RA8875_BLACK);
			}

			drawButtons(currentMenu);
			_display.textTransparent(RA8875_BLACK);

			if (_redrawAlt) {
				_dataDisplay.switchToAltLayer();
				_redrawAlt = false;
			}
		}

		systemUpdate();

		if (refreshTime > refreshRate) {
			refreshTime = 0;

			Button** list = currentMenu;
			while (*list) {
				_redrawAlt |= (*list)->refresh();
				list++;
			}
			if (_redrawAlt) {
				needUpdate = true;
			}
		}

		if (_prefsDirty && (_prefsDirtyTime > prefWriteTime || done)) {
			_prefs.writePrefs();
			_prefsDirty = false;
		}

	// 	static elapsedMillis showTime;
	// 	if (showTime > 1000) {
	// 		Serial.printf("%06d: Menu: free memory=%d\n", millis(), ESP.getFreeHeap());
	// 		showTime = 0;
	// 	}
	}
}
