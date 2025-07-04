#pragma once

#include "Adafruit_RA8875.h"
#include "beep.h"
#include "RA8875_Buffer8.h"

class TouchScreen {
    public:
        void startDisplay(bool have12v);
        void beginTouch();

        void enableBacklight(bool enable);
        bool backlightEnabled() { return _backlightOn; };

        bool computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix);
        tsPoint_t scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix);
        bool touchEvent(tsPoint_t* touchPt);
        void touchRefresh();
        bool screenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix=NULL);
        bool touchReady();
        void allowNextRepeat();
        void clearTouch();
        bool runCalibration(tsMatrix_t* matrix);
        void setTouchMatrix(tsMatrix_t* matrix);

    private:
        tsMatrix_t _matrix;
        bool _have12v;
        bool _backlightOn;
};

extern TouchScreen _touchScreen;
extern Adafruit_RA8875 _display;
extern RA8875_Buffer8 _displayBuffer8;
