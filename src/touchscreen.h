#ifndef DEF_TOUCHSCREEN_CPP
#define DEF_TOUCHSCREEN_CPP

#include "Adafruit_RA8875.h"
#include "beep.h"

extern void wait_tft_done();

class TouchScreen {
    public:
        void begin(Adafruit_RA8875& tft, uint8_t touchIntPin, Beeper* beeper);

        bool computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix);
        tsPoint_t scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix);
        bool touchEvent(tsPoint_t* touchPt);
        bool screenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix=NULL);
        void allowNextRepeat();
        void clearTouch();
        bool runCalibration(tsMatrix_t* matrix);
        void setTouchMatrix(tsMatrix_t* matrix);

    private:
        Adafruit_RA8875* _tft;
        tsMatrix_t _matrix;

        uint8_t _touchIntPin;
        uint8_t _beeperPin;
};

#endif
