#include "graph.h"

#include "touchscreen.h"

#include "fonts/FreeSansBold9pt7b.h"

void DataGraph::draw() {
	uint16_t left, right, top, bottom;

	_display.drawFastHLine(_x, _y, _w, _edgeColor);
	_display.drawFastHLine(_x, _y+_h-1, _w, _edgeColor);
	if (_rightSide) {
		_display.drawFastVLine(_x, _y, _h, _edgeColor);
		left = _x+1;
	}
	else {
		_display.drawFastVLine(_x+_w-1, _y, _h, _edgeColor);
		left = _x;
	}

	top = _y+1;
	bottom = _y+_h-1;
	right = _x+_w-1;

	uint16_t drawHeight = bottom-top;
	uint16_t drawWidth = _w-1;
	uint16_t drawCount = min(_data->sampleCount(), drawWidth);
	bool coloredRamps = _upThreshold != 0 || _downThreshold != 0;

	if (drawWidth<drawCount) {
		drawCount = drawWidth;
	}

	if (_autoScale) {
		if (drawCount) {
			uint16_t vMin = _data->minimum(32767) * 9 / 10;
			uint16_t vMax = _data->maximum(-32767) * 11 / 10;
			uint16_t m1, m2;

			if (vMax - vMin > 1000) {
				m1 = 1000;
				m2 = 2000;
			}
			else {
				m1 = 100;
				m2 = 400;
			}

			_scaleMin = (uint16_t)(vMin / m1) * m1;
			uint16_t max = (uint16_t)((vMax+(m1-1))/ m1) * m1;
			uint16_t diff = max - _scaleMin;
			_scaleMax = _scaleMin + (uint16_t)((diff+(m2-1))/m2) * m2;
			_scaleStep = (_scaleMax-_scaleMin)/4;
		}
		else {
			_scaleMin = 0;
			_scaleMax = 400;
			_scaleStep = 100;
		}
	}

	uint16_t range = max(1, _scaleMax - _scaleMin);

	int16_t value = _data->lookup(0);

	for (uint16_t i=0; i<drawCount; i++) {
		int16_t nextValue = _data->lookup(i+1);
		uint16_t scaled = (value - _scaleMin) * drawHeight / range;

		if (scaled) {
			uint16_t color = _dataColor;

			if (coloredRamps) {
				if (nextValue - value > _downThreshold) {
					color = _downColor;
				}
				else if (value - nextValue > _upThreshold) {
					color = _upColor;
				}
			}
			
			_display.drawFastVLine(_x+_w-2-i, _y+_h-2-scaled, scaled, color);
		}

		value = nextValue;
	}

	_display.setFont(&FreeSansBold9pt7b);

	for (uint16_t v=_scaleMin; v<_scaleMax; v+=_scaleStep) {
		uint16_t scaled = (v - _scaleMin) * drawHeight / range;
		uint16_t vOffset = _y+_h-1-scaled;

		if (v>_scaleMin) {
			_display.drawFastHLine(left, vOffset, right-left, _edgeColor);
		}

		_display.setCursor(left+6, vOffset-6);
		_display.print(v);
	}
}
