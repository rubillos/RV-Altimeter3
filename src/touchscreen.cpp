#include "touchscreen.h"

#include "elapsedMillis.h"
#include "beep.h"
#include "button.h"
#include "SimplyAtomic.h"

extern ButtonScheme backScheme;
extern Button button_back;
extern Button button_done;

#define TOUCH_MAX 512

constexpr uint32_t defaultTouchDelay = 500;
constexpr uint32_t repeatTouchDelay = 300;
constexpr uint32_t beepDuration = 10;

volatile uint32_t interruptEnableTime;
volatile bool interrupted = false;
volatile bool touched = false;
volatile bool allowRepeat = false;

Beeper* touch_beeper = NULL;

void touchInterrupt() {
    static uint32_t lastTouchTime = 0;
    uint32_t time = millis();
    uint32_t interruptDelay = time - interruptEnableTime;

    interrupted = true;

    if (!touched && ((interruptDelay > defaultTouchDelay) || (allowRepeat && (time-lastTouchTime)>repeatTouchDelay))) {
        lastTouchTime = time;
        touched = true;
        if (touch_beeper) {
            touch_beeper->beep(beepDuration);
        }
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
        _tft->touchRead(&x, &y);

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

    screenPts[0] = { _tft->width() * 1 / 10, _tft->height() * 1 / 10 };
    screenPts[1] = { _tft->width() * 5 / 10, _tft->height() * 9 / 10 };
    screenPts[2] = { _tft->width() * 9 / 10, _tft->height() * 5 / 10 };

    _tft->fillScreen(RA8875_BLACK);
    _tft->textMode();
    wait_tft_done();
    _tft->textEnlarge(2);
    _tft->textSetCursor(120, 120);
    _tft->textColor(RA8875_YELLOW, RA8875_BLACK);
    _tft->textWrite("Touch calibration dots..");
    _tft->graphicsMode();
    wait_tft_done();

    for (uint8_t i=0; i<3; i++) {
        _tft->fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE + 4, RA8875_WHITE);
        wait_tft_done();
        _tft->fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE, RA8875_BLACK);
        wait_tft_done();
    }

    button_back.draw(false, true);

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
            button_back.draw(true, true);
            delay(200);
            done = true;
        }

        if (!done) {
            if (ptIndex != -1) {
                _tft->fillCircle(screenPts[ptIndex].x, screenPts[ptIndex].y, CALIBRATION_DOT_SIZE - 4, RA8875_GREEN);
                dotBits &= ~(1<<ptIndex);
                touchPts[ptIndex] = pt;
            }

            delay(100);
        }
    }

    if (!done && computeCalibrationMatrix(screenPts, touchPts, matrix)) {
        result = true;

        _tft->fillScreen(RA8875_BLACK);
        _tft->textMode();
        wait_tft_done();
        _tft->textEnlarge(2);
        _tft->textSetCursor(170, 120);
        _tft->textColor(RA8875_GREEN, RA8875_BLACK);
        _tft->textWrite("Touch to test...");
        _tft->graphicsMode();
        wait_tft_done();

        button_done.draw(false, true);

        bool done = false;
        while(!done) {
            tsPoint_t screenPt;

            if (screenTouch(&screenPt, matrix)) {
                done = button_done.hitTest(screenPt);
                if (done) {
                    button_done.draw(true, true);
                }
                else {
                    _tft->fillCircle(screenPt.x, screenPt.y, 16, RA8875_YELLOW);
                }
            }
        }
    }
    _tft->fillScreen(RA8875_BLACK);

    return result;
}

void TouchScreen::begin(Adafruit_RA8875& tft, uint8_t touchIntPin, Beeper* beeper) {
    _tft = &tft;
    _touchIntPin = touchIntPin;
    touch_beeper = beeper;

    pinMode(_touchIntPin, INPUT_PULLUP);

    interruptEnableTime = millis();
    attachInterrupt(_touchIntPin, touchInterrupt, FALLING);

    Serial.println("Starting touch");
    _tft->touchEnable(true);
}

void TouchScreen::setTouchMatrix(tsMatrix_t* matrix) {
    _matrix = *matrix;
}
