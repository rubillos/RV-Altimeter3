#include "touchscreen.h"

#include "elapsedMillis.h"
#include "beep.h"
#include "button.h"

extern ButtonScheme backScheme;
extern Button button_back;

#define TOUCH_MAX 512

Adafruit_RA8875* touch_tft;
uint8_t touch_int_pin;
uint8_t touch_wait_pin;
uint8_t touch_beep_pin = 255;

void wait_tft_done() {
    while (digitalRead(touch_wait_pin)==LOW) {}
}

bool computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix) {
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

tsPoint_t scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix) {
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

constexpr uint32_t defaultTouchDelay = 500;
constexpr uint32_t repeatTouchDelay = 300;
constexpr uint32_t beepDuration = 20;

volatile uint32_t interruptEnableTime;
volatile bool interrupted = false;
volatile bool touched = false;
volatile bool allowRepeat = false;

void allowNextRepeat() {
    allowRepeat = true;
}

void touchInterrupt() {
    static uint32_t lastTouchTime = 0;
    uint32_t time = millis();
    uint32_t interruptDelay = time - interruptEnableTime;
    interrupted = true;

    if ((interruptDelay > defaultTouchDelay) || (allowRepeat && (time-lastTouchTime)>repeatTouchDelay)) {
        lastTouchTime = time;
        touched = true;
        if (touch_beep_pin != 255) {
            beep(touch_beep_pin, LOW, beepDuration);
        }
    }
}

bool checkTouchEvent(tsPoint_t* touchPt) {
    bool result = false;

    if (interrupted) {
        bool wasTouched = touched;
        uint16_t x, y;

        interrupted = false;
        interruptEnableTime = millis();
        touch_tft->touchRead(&x, &y);

        if (wasTouched) {
            touched = false;

            Serial.printf("Touched: x=0x%3X, y=0x%03X\n", x, y);

            touchPt->x = x;
            touchPt->y = y;
            allowRepeat = false;
            result = true;
        }

    }
    return result;
}

bool checkScreenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix) {
    tsPoint_t touchPt;

    if (checkTouchEvent(&touchPt)) {
        *screenPt = scaleTouchPoint(touchPt, matrix);
        return true;
    }
    else {
        return false;
    }
}

#define CALIBRATION_DOT_SIZE 16

bool runCalibration(tsMatrix_t* matrix) {
    bool result = false;
    uint8_t dotBits = 0b111;
    tsPoint_t touchPts[3];
    tsPoint_t screenPts[3];

    screenPts[0] = { touch_tft->width() * 1 / 10, touch_tft->height() * 1 / 10 };
    screenPts[1] = { touch_tft->width() * 5 / 10, touch_tft->height() * 9 / 10 };
    screenPts[2] = { touch_tft->width() * 9 / 10, touch_tft->height() * 5 / 10 };

    touch_tft->textMode();
    touch_tft->fillScreen(RA8875_BLACK);
    wait_tft_done();
    touch_tft->textEnlarge(2);
    touch_tft->textSetCursor(170, 120);
    touch_tft->textColor(RA8875_YELLOW, RA8875_BLACK);
    touch_tft->textWrite("Touch calibration dots..");
    touch_tft->graphicsMode();
    wait_tft_done();

    for (uint8_t i=0; i<3; i++) {
        touch_tft->fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE + 4, RA8875_WHITE);
        wait_tft_done();
        touch_tft->fillCircle(screenPts[i].x, screenPts[i].y, CALIBRATION_DOT_SIZE, RA8875_BLACK);
        wait_tft_done();
    }

    button_back.draw(false, true);

    uint16_t one_third = TOUCH_MAX/3;
    uint16_t two_thirds = TOUCH_MAX*2/3;
    tsPoint_t pt;
    bool done = false;

    while (!done && dotBits) {
        while (!checkTouchEvent(&pt)) {}

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
                touch_tft->fillCircle(screenPts[ptIndex].x, screenPts[ptIndex].y, CALIBRATION_DOT_SIZE - 4, RA8875_GREEN);
                dotBits &= ~(1<<ptIndex);
                touchPts[ptIndex] = pt;
            }

            delay(100);
        }
    }

    if (!done && computeCalibrationMatrix(screenPts, touchPts, matrix)) {
        result = true;

        touch_tft->textMode();
        touch_tft->fillScreen(RA8875_BLACK);
        wait_tft_done();
        touch_tft->textEnlarge(2);
        touch_tft->textSetCursor(170, 120);
        touch_tft->textColor(RA8875_GREEN, RA8875_BLACK);
        touch_tft->textWrite("Touch to test..");
        touch_tft->graphicsMode();
        wait_tft_done();

        bool done = false;
        while(!done) {
            tsPoint_t screenPt;

            if (checkScreenTouch(&screenPt, matrix)) {
                done = button_back.hitTest(screenPt);
                if (done) {
                    button_back.draw(true, true);
                }
                else {
                    touch_tft->fillCircle(screenPt.x, screenPt.y, 16, RA8875_YELLOW);
                }
            }
        }
    }
    touch_tft->fillScreen(RA8875_BLACK);

    return result;
}

void startTouch(Adafruit_RA8875& tft, uint8_t touchIntPin, uint8_t touchWaitPin, uint8_t beeperPin) {
    touch_tft = &tft;
    touch_int_pin = touchIntPin;
    touch_wait_pin = touchWaitPin;

    pinMode(touch_int_pin, INPUT_PULLUP);
    pinMode(touch_wait_pin, INPUT_PULLUP);

    touch_beep_pin = beeperPin;
    interruptEnableTime = millis();
    attachInterrupt(touch_int_pin, touchInterrupt, FALLING);

    Serial.println("Starting touch");
    touch_tft->touchEnable(true);
}
