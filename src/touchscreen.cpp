#include "touchscreen.h"

#include "elapsedMillis.h"
#include "beep.h"
#include "defs.h"
#include "button.h"
#include "SimplyAtomic.h"
#include "pins.h"
#include "menu.h"

#define TOUCH_MAX 512

constexpr uint32_t defaultTouchDelay = 500;
constexpr uint32_t repeatTouchDelay = 300;
constexpr uint32_t beepDuration = 10;

volatile uint32_t interruptEnableTime;
volatile bool interrupted = false;
volatile bool touched = false;
volatile bool allowRepeat = false;

SPIClass LCDSPI(HSPI);
Adafruit_RA8875 _display = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
Buffer8 _displayBuffer(0, 0, cellWidth, cellHeight);

void touchInterrupt() {
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

void TouchScreen::allowNextRepeat() {
    allowRepeat = true;
}

bool TouchScreen::touchReady() {
    return touched;
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

#define CALIBRATION_DOT_SIZE 16

bool TouchScreen::runCalibration(tsMatrix_t* matrix) {
    bool result = false;
    uint8_t dotBits = 0b111;
    tsPoint_t touchPts[3];
    tsPoint_t screenPts[3];

    screenPts[0] = { _display.width() * 1 / 10, _display.height() * 1 / 10 };
    screenPts[1] = { _display.width() * 5 / 10, _display.height() * 9 / 10 };
    screenPts[2] = { _display.width() * 9 / 10, _display.height() * 5 / 10 };

    _display.fillScreen(RA8875_BLACK);
    _display.textMode();
    _display.textEnlarge(2);
    _display.textSetCursor(120, 120);
    _display.textColor(RA8875_YELLOW, RA8875_BLACK);
    _display.textWrite("Touch calibration dots..");
    _display.graphicsMode();

    for (uint8_t i=0; i<3; i++) {
        _display.fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE + 4, RA8875_WHITE);
        _display.fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE, RA8875_BLACK);
    }

    buttonBack.draw(false, true);

    uint16_t one_third = TOUCH_MAX/3;
    uint16_t two_thirds = TOUCH_MAX*2/3;
    tsPoint_t pt;
    bool done = false;

    while (!done && dotBits) {
        while (!touchEvent(&pt)) {}

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
                _display.fillCircle(screenPts[ptIndex].x, screenPts[ptIndex].y, CALIBRATION_DOT_SIZE - 4, RA8875_GREEN);
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
        }
    }
    _display.fillScreen(RA8875_BLACK);

    return result;
}

void TouchScreen::startDisplay(bool have12v) {
	LCDSPI.begin(RA8875_SCK, RA8875_MISO, RA8875_MOSI, RA8875_CS);

	delay(1000);
	if (!_display.begin(RA8875_800x480, &LCDSPI)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	pinMode(RA8875_WAIT, INPUT_PULLUP);

	_display.setLayerMode(true);
	_display.graphicsMode();                 // go back to graphics mode
	_display.fillScreen(BLACK8);
	delay(1);

	_display.displayOn(true);
	delay(1);

	_display.GPIOX(true);      // Enable display - display enable tied to GPIOX
	delay(100);

	_display.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	delay(100);

	_display.PWM1out((have12v) ? 255 : 127);	// set backlight
	_display.setWaitPin(RA8875_WAIT);
}

void TouchScreen::beginTouch() {
    pinMode(RA8875_INT, INPUT_PULLUP);

    interruptEnableTime = millis();
    attachInterrupt(RA8875_INT, touchInterrupt, FALLING);
    _display.touchEnable(true);
}

void TouchScreen::setTouchMatrix(tsMatrix_t* matrix) {
    _matrix = *matrix;
}

TouchScreen _touchScreen;
