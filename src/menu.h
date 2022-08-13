#ifndef DEF_MENU_H
#define DEF_MENU_H

#include "button.h"

#define ColorRA8875(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

constexpr uint16_t RA8875_GRAY_DK = 0b0101001010101010;
constexpr uint16_t RA8875_GRAY_LT = 0b1010010100010100;

// constexpr uint16_t RA8875_GRAY_LT = ColorRA8875(192, 192, 192);
// constexpr uint16_t RA8875_GRAY = ColorRA8875(127, 127, 127);
// constexpr uint16_t RA8875_GRAY_DK = ColorRA8875(44, 44, 44);

class Menu {
    public:
        void begin(Adafruit_RA8875* tft, float* minPressure, float* maxPressure, float* maxTemperature, tsMatrix_t* calibration);

        void run();

        bool _goBack = false;
        float* _minPressure;
        float* _maxPressure;
        float* _maxTemperature;
        tsMatrix_t* _calibration;

    private:
        Adafruit_RA8875* _tft;

        Button** _menuStack[10];
        uint16_t _menuStackIndex = 0;

};

#endif
