#ifndef DEF_TOUCHSCREEN_CPP
#define DEF_TOUCHSCREEN_CPP

#include "Adafruit_RA8875.h"

extern bool computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix);
extern tsPoint_t scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix);
extern bool checkTouchEvent(tsPoint_t* touchPt);
extern bool checkScreenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix);
extern void allowNextRepeat();
extern void clearTouch();
extern bool runCalibration(tsMatrix_t* matrix);
extern void startTouch(Adafruit_RA8875& tft, uint8_t touchIntPin, uint8_t touchWaitPin, uint8_t beeperPin = 255);

#endif
