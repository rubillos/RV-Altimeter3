#include "menu.h"

#include "elapsedMillis.h"
#include "touchscreen.h"
#include "textManager.h"
#include "packets.h"
#include "dataDisplay.h"
#include "beep.h"
#include "defs.h"

constexpr uint32_t prefWriteTime = 2000;
constexpr uint32_t refreshRate = 200;

//-------------------------------------------------------
void Menu::run(Button** currentMenu) {
	bool done = false;
	bool needUpdate = true;
	bool refreshFast = false;
	static elapsedMillis refreshTime;

	setupButtons(currentMenu);

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
					setupButtons(currentMenu);
					needUpdate = true;
					delay(buttonFlashTime);
				}
				_display.textTransparent(RA8875_BLACK);
			}
		}

		if (needUpdate || _redrawAlt) {
			needUpdate = false;

			if (_redrawAlt) {
				_dataDisplay.drawInAltLayer();
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

		if (refreshFast || refreshTime > refreshRate) {
			refreshTime = 0;
			refreshFast = false;

			Button** list = currentMenu;
			while (*list) {
				uint8_t result = (*list)->refresh();
				if (result == buttonRefreshRedraw) {
					_redrawAlt = true;
				}
				else if (result == buttonRefreshFast) {
					refreshFast = true;
				}
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
	}
}
