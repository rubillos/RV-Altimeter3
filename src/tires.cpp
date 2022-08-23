#include "tires.h"
#include "defs.h"
#include "touchscreen.h"
#include "prefs.h"

// https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// ./fontconvert freefont-ttf/sfd/FreeSansBold.ttf 24 32 126 '~°' > Fonts/FreeSansBold24pt7bCustom.h

#include "fonts/FreeSansBold30pt7b.h"
#include "fonts/FreeSansBold24pt7bCustom.h"
#include "graphics/Tire2.png.h"

void TireHandler::drawTires() {
	constexpr uint16_t tireTopY = 300;
    constexpr uint16_t tireX[] = { 272, 416, 152, 272, 416, 536 };
    constexpr uint16_t tireY[] = { tireTopY, tireTopY, tireTopY+80, tireTopY+80, tireTopY+80, tireTopY+80 };
	constexpr uint16_t tireWidth = 110;
	constexpr uint16_t tireHeight = 67;

    constexpr uint16_t pressures[] = { 23, 86, 91, 102, 112, 108 };
    constexpr uint16_t temperatures[] = { 23, 40, 61, 102, 112, 161 };

	_display.fillRect(380, tireTopY+30, 40, 6, WHITE16);
	_display.fillRect(262, tireTopY+110, 280, 6, WHITE16);
	_display.fillRect(396, tireTopY+32, 6, 80, WHITE16);

	uint16_t tirePressure[] = { 87, 87, 90, 102, 92, 90 };
	uint8_t tireColor[] = { GREEN8, GREEN8, GREEN8, GREEN8, GREEN8, GREEN8 };

    uint32_t time = millis();
    char stringBuff[10];
    const char* str;

	for (uint16_t i=0; i<6; i++) {
        TPMSPacket* sensor = &_sensorPackets[i];
        int16_t xOffset = -1, yOffset = 0;

        // uint16_t temperature = _sensorPackets[i].temperature;
        // uint16_t pressure = _sensorPackets[i].pressure;
        uint16_t temperature = temperatures[i];
        uint16_t pressure = pressures[i];

        bool tempAlarm = temperature >= _prefData.alarmTempMax;
        bool tempWarn = temperature >= _prefData.alarmTempMax-5;
        bool pressureAlarm = (pressure <= _prefData.alarmPressureMin) || (pressure >= _prefData.alarmPressureMax);
        bool pressureWarn = (pressure <= _prefData.alarmPressureMin+5) || (pressure >= _prefData.alarmPressureMax-5);

        bool forceTemperature = tempWarn && ((millis()/1000) % 2)==0;

		_displayBuffer.setOffset(tireX[i], tireY[i]);
		drawPNG(Tire2_png, sizeof(Tire2_png),&_displayBuffer, tireX[i], tireY[i]);

        if (sensor->timeStamp && !sensor->stale && (time-sensor->timeStamp)>sensorTimeout) {
            sensor->stale = true;
        }

        if (0 && sensorIDs[i] == 0) {
        	_displayBuffer.setFont(&FreeSansBold30pt7b);
            _displayBuffer.setTextColor(RED8);
            str = "??";
        }
        else if (0 && (sensor->stale || sensor->timeStamp == 0)) {
        	_displayBuffer.setFont(&FreeSansBold30pt7b);
            _displayBuffer.setTextColor(ORANGE8);
            str = "--";
            yOffset = -6;
        }
        else if (forceTemperature || _tempTimer < temperatureTime) {
        	_displayBuffer.setFont(&FreeSansBold24pt7bCustom);
            if (tempAlarm) {
                _displayBuffer.setTextColor(DARKORANGE8);
            }
            else if (tempWarn) {
                _displayBuffer.setTextColor(YELLOW8);
            }
            else if (temperature < 33) {
                _displayBuffer.setTextColor(LIGHTBLUE8);
            }
            else {
                _displayBuffer.setTextColor(CYAN8);
            }
            snprintf(stringBuff, sizeof(stringBuff), "%d~", temperature);
            str = stringBuff;
            yOffset -= 5;
            if (str[0]=='1') {
                xOffset -= 2;
            }
            else {
                xOffset += 2;
            }
        }
        else {
        	_displayBuffer.setFont(&FreeSansBold30pt7b);
            if (pressureAlarm) {
                _displayBuffer.setTextColor(RED8);
            }
            else if (pressureWarn) {
                _displayBuffer.setTextColor(ORANGE8);
            }
            else {
                _displayBuffer.setTextColor(GREEN8);
            }
            snprintf(stringBuff, sizeof(stringBuff), "%d", pressure);
            str = stringBuff;
            yOffset -= 1;
            if (str[0]=='1') {
                xOffset -= 4;
            }
        }

        int16_t x, y;
        uint16_t width, height;

        _displayBuffer.getTextBounds(str, 0, 0, &x, &y, &width, &height);
        _displayBuffer.setCursor(tireX[i] + (tireWidth-width)/2 + xOffset, tireY[i]+tireHeight-13+yOffset);
        _displayBuffer.print(str);
		_displayBuffer.draw(_display, tireWidth, tireHeight);
	}
}

void TireHandler::showTemperature() {
    _tempTimer = 0;
}

uint16_t sIndex = 0;

int16_t TireHandler::indexOfSensor(uint32_t sensorID) {
    sIndex = (sIndex+1) % 6;
    return sIndex;

	for (uint8_t i=0; i<numTires; i++) {
		if (sensorID == sensorIDs[i]) {
			return i;
		}
	}
	return -1;
}

void TireHandler::recordPacket(TPMSPacket& packet) {
    int16_t index = indexOfSensor(packet.id);

    if (index != -1) {
        Serial.println("Recording packet");
        _sensorPackets[index] = packet;
    }
}

void TireHandler::sensorIDChanged(uint16_t sensorIndex) {
    _sensorPackets[sensorIndex].timeStamp = 0;
}

TireHandler _tireHandler;
