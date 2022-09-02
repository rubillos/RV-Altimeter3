#ifndef DEF_TOUCHSCREEN_CPP
#define DEF_TOUCHSCREEN_CPP

#include "Adafruit_RA8875.h"
#include "beep.h"
#include "Buffer8.h"

class TouchScreen {
    public:
        void startDisplay(bool have12v);
        void beginTouch();

        void enableBacklight(bool enable);

        bool computeCalibrationMatrix(tsPoint_t* displayPts, tsPoint_t* touchPts, tsMatrix_t* matrix);
        tsPoint_t scaleTouchPoint(tsPoint_t touchPt, tsMatrix_t* matrix);
        bool touchEvent(tsPoint_t* touchPt);
        bool screenTouch(tsPoint_t* screenPt, tsMatrix_t* matrix=NULL);
        bool touchReady();
        void allowNextRepeat();
        void clearTouch();
        bool runCalibration(tsMatrix_t* matrix);
        void setTouchMatrix(tsMatrix_t* matrix);

    private:
        tsMatrix_t _matrix;
        bool _have12v;
};

extern TouchScreen _touchScreen;
extern Adafruit_RA8875 _display;
extern Buffer8 _displayBuffer8;
extern Buffer1 _displayBuffer1;

#endif
