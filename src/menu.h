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

constexpr uint32_t buttonFlashTime = 200;

#define ColorRA8875(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

constexpr uint16_t RA8875_GRAY_DK = 0b0101001010101010;
constexpr uint16_t RA8875_GRAY_LT = 0b1010010100010100;

class Menu {
	public:
		void run(Button** currentMenu=NULL);
		bool isAtTopLevel() { return _menuStackIndex == 0; };
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

#endif
