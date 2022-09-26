#include "touchscreen.h"

#include "elapsedMillis.h"
#include "beep.h"
#include "defs.h"
#include "button.h"
#include "SimplyAtomic.h"
#include "pins.h"
#include "mainMenu.h"

constexpr uint16_t touchMax = 512;

constexpr uint32_t defaultTouchDelay = 500;
constexpr uint32_t repeatTouchDelay = 300;
constexpr uint32_t beepDuration = 10;

volatile uint32_t interruptEnableTime;
volatile bool interrupted = false;
volatile bool touched = false;
volatile bool allowRepeat = false;

SPIClass LCDSPI(HSPI);
Adafruit_RA8875 _display = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
RA8875_Buffer8 _displayBuffer8(0, 0, cellWidth, cellHeight);

void IRAM_ATTR touchInterrupt() {
    static uint32_t lastTouchTime = 0;
    uint32_t time = millis();
    uint32_t interruptDelay = time - interruptEnableTime;

    interrupted = true;

    if (!touched && ((interruptDelay > defaultTouchDelay) || (allowRepeat && (time-lastTouchTime)>repeatTouchDelay))) {
        lastTouchTime = time;
        touched = true;
        _beeper.beep(beepDuration);
    }
}

void TouchScreen::enableBacklight(bool enable) {
    if (enable) {
        _display.displayOn(true);
        delay(1);
        _display.GPIOX(true);
        delay(200);
        _display.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
        delay(100);
        _display.PWM1out((_have12v) ? 255 : 127);	// set backlight
    }
    else {
        _display.PWM1out(0);	// set backlight
        _display.PWM1config(false, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
        _display.GPIOX(false);
        _display.displayOn(false);
    }
}

void TouchScreen::startDisplay(bool have12v) {
	LCDSPI.begin(RA8875_SCK, RA8875_MISO, RA8875_MOSI, RA8875_CS);

	if (!_display.begin(RA8875_800x480, &LCDSPI)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

    _have12v = have12v;

	pinMode(RA8875_WAIT, INPUT_PULLUP);
	_display.setWaitPin(RA8875_WAIT);

	_display.setLayerMode(true);
	_display.graphicsMode();                 // go back to graphics mode
	_display.fillScreen(BLACK8);

	enableBacklight(true);

    _displayBuffer8.setTextSize(1);
    _displayBuffer8.setTextWrap(false);
}

void TouchScreen::beginTouch() {
    pinMode(RA8875_INT, INPUT_PULLUP);

    interruptEnableTime = millis();
    attachInterrupt(RA8875_INT, touchInterrupt, FALLING);
    _display.touchEnable(true);
}

void TouchScreen::allowNextRepeat() {
    allowRepeat = true;
}

bool TouchScreen::touchReady() {
    return touched;
}

void TouchScreen::touchRefresh() {
    bool result = false;

    if (interrupted && !touched) {
        uint32_t time = millis();

        ATOMIC() {
            interrupted = false;
            interruptEnableTime = time;
        }

        // Serial.printf("%d: Was interrupted\n", millis());

        uint16_t x, y;
        _display.touchRead(&x, &y);
    }
}

bool TouchScreen::touchEvent(tsPoint_t* touchPt) {
    bool result = false;

    if (interrupted) {
        uint32_t time = millis();
        bool wasTouched;

        ATOMIC() {
            wasTouched = touched;
            touched = false;
            interrupted = false;
            interruptEnableTime = time;
        }

        // Serial.printf("%d: Was interrupted\n", millis());

        uint16_t x, y;
        _display.touchRead(&x, &y);

        if (wasTouched) {
            // Serial.printf("Touched: x=0x%3X, y=0x%03X\n", x, y);

            touchPt->x = x;
            touchPt->y = y;
            allowRepeat = false;
            result = true;
        }

    }
    return result;
}

bool TouchScreen::screenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix) {
    tsPoint_t touchPt;

    if (matrix == NULL) {
        matrix = &_matrix;
    }

    if (matrix->Divider && touchEvent(&touchPt)) {
        *screenPt = scaleTouchPoint(touchPt, matrix);
        return true;
    }
    else {
        return false;
    }
}

bool TouchScreen::computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix) {
	matrix->Divider = ((touchPts[0].x - touchPts[2].x) * (touchPts[1].y - touchPts[2].y)) -
						((touchPts[1].x - touchPts[2].x) * (touchPts[0].y - touchPts[2].y)) ;

	if( matrix->Divider == 0 ) {
		return false;
	}
	else {
		matrix->An = ((displayPts[0].x - displayPts[2].x) * (touchPts[1].y - touchPts[2].y)) -
						((displayPts[1].x - displayPts[2].x) * (touchPts[0].y - touchPts[2].y)) ;

		matrix->Bn = ((touchPts[0].x - touchPts[2].x) * (displayPts[1].x - displayPts[2].x)) -
						((displayPts[0].x - displayPts[2].x) * (touchPts[1].x - touchPts[2].x)) ;

		matrix->Cn = (touchPts[2].x * displayPts[1].x - touchPts[1].x * displayPts[2].x) * touchPts[0].y +
						(touchPts[0].x * displayPts[2].x - touchPts[2].x * displayPts[0].x) * touchPts[1].y +
						(touchPts[1].x * displayPts[0].x - touchPts[0].x * displayPts[1].x) * touchPts[2].y ;

		matrix->Dn = ((displayPts[0].y - displayPts[2].y) * (touchPts[1].y - touchPts[2].y)) -
						((displayPts[1].y - displayPts[2].y) * (touchPts[0].y - touchPts[2].y)) ;

		matrix->En = ((touchPts[0].x - touchPts[2].x) * (displayPts[1].y - displayPts[2].y)) -
						((displayPts[0].y - displayPts[2].y) * (touchPts[1].x - touchPts[2].x)) ;

		matrix->Fn = (touchPts[2].x * displayPts[1].y - touchPts[1].x * displayPts[2].y) * touchPts[0].y +
						(touchPts[0].x * displayPts[2].y - touchPts[2].x * displayPts[0].y) * touchPts[1].y +
						(touchPts[1].x * displayPts[0].y - touchPts[0].x * displayPts[1].y) * touchPts[2].y ;

		return true;
	}
}

tsPoint_t TouchScreen::scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix) {
  tsPoint_t scaled;

  if(matrix->Divider != 0) {
    scaled.x = ( (matrix->An * touchPt.x) + (matrix->Bn * touchPt.y) + matrix->Cn ) / matrix->Divider ;
    scaled.y = ( (matrix->Dn * touchPt.x) + (matrix->En * touchPt.y) + matrix->Fn ) / matrix->Divider ;
  }
  else {
    scaled.x = scaled.y = 0;
  }

  return scaled;
}

constexpr uint16_t calibrationDotSize = 16;

ButtonScheme calibrateScheme = { RA8875_YELLOW, RA8875_BLACK, RA8875_BLACK, 3, 3 };
Label labelTouchCalibrate(0,  120, 800, 32, "Touch calibration dots...", calibrateScheme);

bool TouchScreen::runCalibration(tsMatrix_t* matrix) {
    bool result = false;
    uint8_t dotBits = 0b111;
    tsPoint_t touchPts[3];
    tsPoint_t screenPts[3];

    screenPts[0] = { _display.width() * 1 / 10, _display.height() * 1 / 10 };
    screenPts[1] = { _display.width() * 5 / 10, _display.height() * 9 / 10 };
    screenPts[2] = { _display.width() * 9 / 10, _display.height() * 5 / 10 };

    _display.fillScreen(RA8875_BLACK);

    labelTouchCalibrate.draw();

    for (uint8_t i=0; i<3; i++) {
        _display.fillCircle(screenPts[i].x, screenPts[i].y, calibrationDotSize + 4, RA8875_WHITE);
        _display.fillCircle(screenPts[i].x, screenPts[i].y, calibrationDotSize, RA8875_BLACK);
    }

    buttonBack.draw(false, true);

    uint16_t one_third = touchMax/3;
    uint16_t two_thirds = touchMax*2/3;
    tsPoint_t pt;
    bool done = false;

    while (!done && dotBits) {
        while (!touchEvent(&pt)) {
            systemUpdate();
        }

        Serial.printf("Touch pt: x=0x%3X, y=0x%3X\n", pt.x, pt.y);

        int16_t ptIndex = -1;

        if (pt.x < one_third && pt.y < one_third) {
            ptIndex = 0;
        }
        else if (pt.x > one_third && pt.x < two_thirds && pt.y > two_thirds) {
            ptIndex = 1;
        }
        else if (pt.x > two_thirds && pt.y > one_third && pt.y < two_thirds) {
            ptIndex = 2;
        }
        else if (pt.x < one_third && pt.y > two_thirds) {
            buttonBack.draw(true, true);
            delay(200);
            done = true;
        }

        if (!done) {
            if (ptIndex != -1) {
                _display.fillCircle(screenPts[ptIndex].x, screenPts[ptIndex].y, calibrationDotSize - 4, RA8875_GREEN);
                dotBits &= ~(1<<ptIndex);
                touchPts[ptIndex] = pt;
            }

            delay(100);
        }
    }

    if (!done && computeCalibrationMatrix(screenPts, touchPts, matrix)) {
        result = true;

        _display.fillScreen(RA8875_BLACK);
        _display.textMode();
        _display.textEnlarge(2);
        _display.textSetCursor(170, 120);
        _display.textColor(RA8875_GREEN, RA8875_BLACK);
        _display.textWrite("Touch to test...");
        _display.graphicsMode();
        _display.textTransparent(RA8875_BLACK);

        buttonDone.draw(false, true);

        bool done = false;
        while(!done) {
            tsPoint_t screenPt;

            if (screenTouch(&screenPt, matrix)) {
                done = buttonDone.hitTest(screenPt);
                if (done) {
                    buttonDone.draw(true, true);
                }
                else {
                    _display.fillCircle(screenPt.x, screenPt.y, 16, RA8875_YELLOW);
                }
            }
            systemUpdate();
        }
    }
    _display.fillScreen(RA8875_BLACK);

    return result;
}

void TouchScreen::setTouchMatrix(tsMatrix_t* matrix) {
    _matrix = *matrix;
}

TouchScreen _touchScreen;
