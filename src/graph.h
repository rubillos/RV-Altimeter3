#pragma once

#include "Arduino.h"
#include "RingBuff.h"
#include "Buffer8.h"

class DataGraph {
    public:
        DataGraph(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t textColor, uint16_t dataColor, uint16_t edgeColor, bool rightSide);
        void setRingBuff(RingBuff<int16_t>* data) { _data = data; };
        void setAutoScale(bool scale) { _autoScale=scale; };
        void setScale(uint16_t min, uint16_t max, uint16_t step) { _scaleMin=min; _scaleMax=max; _scaleStep=step; _autoScale=false; };
        void setRampColors(uint16_t upThreshold, uint16_t upColor, uint16_t downThreshold, uint16_t downColor) {
            _upThreshold = upThreshold; _upColor = upColor; _downThreshold = downThreshold; _downColor = downColor;
        };
        void draw();

    private:
        int16_t _x, _y;
        uint16_t _h, _w;
        bool _rightSide;
        uint16_t _edgeColor;
        uint16_t _dataColor;
        uint16_t _textColor;

        bool _autoScale = false;
        int16_t _scaleMin;
        int16_t _scaleMax;
        int16_t _scaleStep;

        RingBuff<int16_t>* _data;

        uint16_t _upThreshold = 0;
        uint16_t _upColor;
        uint16_t _downThreshold = 0;
        uint16_t _downColor;

        OutlineBuff* _outlineBuff;
};
