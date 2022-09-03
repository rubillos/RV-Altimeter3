#include "tires.h"
#include "defs.h"
#include "touchscreen.h"
#include "prefs.h"
#include "png.h"

// https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// ./fontconvert freefont-ttf/sfd/FreeSansBold.ttf 24 32 126 '~°' > Fonts/FreeSansBold24pt7bCustom.h

#include "fonts/FreeSansBold30pt7b.h"
#include "fonts/FreeSansBold24pt7bCustom.h"
#include "graphics/Tire2.png.h"

float pressureList[] = { 90, 95, 90, 100, 105, 76, 124 };
constexpr uint16_t pressureCount = sizeof(pressureList) / sizeof(float);

float tempList[] = { 65, 70, 80, 85, 80, 150, 80, 30 };
constexpr uint16_t tempCount = sizeof(tempList) / sizeof(float);

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

    static float tireP[] = { 0.0, 0.1, 0.2, 0.3, 0.4, 0.5 };

	uint16_t tirePressure[] = { 87, 87, 90, 102, 92, 90 };
	uint8_t tireColor[] = { GREEN8, GREEN8, GREEN8, GREEN8, GREEN8, GREEN8 };

    uint32_t time = millis();
    String str;

	for (uint16_t i=0; i<6; i++) {
        TPMSPacket* sensor = &_sensorPackets[i];
        int16_t xOffset = -1, yOffset = 0;

        // uint16_t temperature = _sensorPackets[i].temperature;
        // uint16_t pressure = _sensorPackets[i].pressure;
        // uint16_t temperature = temperatures[i];
        // uint16_t pressure = pressures[i];
        uint16_t temperature = sequenceInterp(tempList, tempCount, tireP[i]);
        uint16_t pressure = sequenceInterp(pressureList, pressureCount, tireP[i]);
        tireP[i] += 0.001;
        if (tireP[i]>=1.0) {
            tireP[i] -= 1.0;
        }
        _sensorPackets[i].pressure = pressure;
        _sensorPackets[i].temperature = temperature;

        bool tempAlarm = temperature >= _prefData.alarmTempMax;
        bool tempWarn = temperature >= _prefData.alarmTempMax-5;
        bool pressureAlarm = (pressure <= _prefData.alarmPressureMin) || (pressure >= _prefData.alarmPressureMax);
        bool pressureWarn = (pressure <= _prefData.alarmPressureMin+5) || (pressure >= _prefData.alarmPressureMax-5);

        bool forceTemperature = tempWarn && ((millis()/1000) % 2)==0;

		_displayBuffer8.setOffset(tireX[i], tireY[i]);

        uint8_t tireColor;

        if (pressureAlarm || tempAlarm) {
            tireColor = RED8;
        }
        else if (pressureWarn || tempWarn) {
            tireColor = DARKORANGE8;
        }
        else {
            tireColor = WHITE8;
        }

		drawPNG83(Tire2_png, sizeof(Tire2_png), &_displayBuffer8, tireX[i], tireY[i], tireColor);

        if (sensor->timeStamp && !sensor->stale && (time-sensor->timeStamp)>sensorTimeout) {
            sensor->stale = true;
        }

        if (0 && _prefData.sensorIDs[i] == 0) {
        	_displayBuffer8.setFont(&FreeSansBold30pt7b);
            _displayBuffer8.setTextColor(RED8);
            str = timedOutString();
        }
        else if (0 && (sensor->stale || sensor->timeStamp == 0)) {
        	_displayBuffer8.setFont(&FreeSansBold30pt7b);
            _displayBuffer8.setTextColor(ORANGE8);
            str = noDataString();
            yOffset = -6;
        }
        else if (forceTemperature || _tempTimer < temperatureTime) {
        	_displayBuffer8.setFont(&FreeSansBold24pt7bCustom);
            if (tempAlarm) {
                _displayBuffer8.setTextColor(DARKORANGE8);
            }
            else if (tempWarn) {
                _displayBuffer8.setTextColor(YELLOW8);
            }
            else if (temperature < 33) {
                _displayBuffer8.setTextColor(LIGHTBLUE8);
            }
            else {
                _displayBuffer8.setTextColor(CYAN8);
            }
            str = temperatureString(temperature, true);
            yOffset -= 5;
            if (str.c_str()[0]=='1') {
                xOffset -= 2;
            }
            else {
                xOffset += 2;
            }
        }
        else {
        	_displayBuffer8.setFont(&FreeSansBold30pt7b);
            if (pressureAlarm) {
                _displayBuffer8.setTextColor(RED8);
            }
            else if (pressureWarn) {
                _displayBuffer8.setTextColor(ORANGE8);
            }
            else {
                _displayBuffer8.setTextColor(GREEN8);
            }
            str = pressureString(pressure, false);
            yOffset -= 1;
            if (str.c_str()[0]=='1') {
                xOffset -= 4;
            }
        }

        int16_t x, y;
        uint16_t width, height;

        _displayBuffer8.getTextBounds(str, 0, 0, &x, &y, &width, &height);
        _displayBuffer8.setCursor(tireX[i] + (tireWidth-width)/2 + xOffset, tireY[i]+tireHeight-13+yOffset);
        _displayBuffer8.print(str);
		_displayBuffer8.draw(_display, tireWidth, tireHeight);
	}
}

void TireHandler::showTemperature() {
    _tempTimer = 0;
}

uint16_t sIndex = 0;

int16_t TireHandler::indexOfSensor(uint32_t sensorID) {
	for (uint8_t i=0; i<numTires; i++) {
		if (sensorID == _prefData.sensorIDs[i]) {
			return i;
		}
	}
	return -1;
}

void TireHandler::recordPacket(TPMSPacket& packet) {
    int16_t index = indexOfSensor(packet.id);

    if (index != -1) {
        Serial.printf("Recording packet: #%d\n", index);
        _sensorPackets[index] = packet;
    }
}

void TireHandler::sensorIDChanged(uint16_t sensorIndex) {
    _sensorPackets[sensorIndex].timeStamp = 0;
}

const char* tireNames[] = {
    "Left Front",
    "Right Front",
    "Left Rear Inner",
    "Right Rear Inner",
    "Left Rear Outer",
    "Right Rear Outer"
};

const char* TireHandler::tireName(uint16_t index) {
    return tireNames[index];
}

const char* TireHandler::noDataString() {
    return "--";
}

const char* TireHandler::timedOutString() {
    return "??";
}

String TireHandler::pressureString(float pressure, bool addPSI) {
    if (pressure == noDataValue) {
        return noDataString();
    }
    else if (pressure == timedOutValue) {
        return timedOutString();
    }
    else {
        String str = String(pressure, 0);

        if (addPSI) {
            str = str + "psi";
        }

        return str;
    }
}

String TireHandler::temperatureString(float temperature, uint8_t addDegrees) {
    if (temperature == noDataValue) {
        return noDataString();
    }
    else if (temperature == timedOutValue) {
        return timedOutString();
    }
    else {
        String str = String(temperature, 0);

        if (addDegrees == 1) {
            str = str + "~";
        }
        else if (addDegrees == 2) {
            str = str + "\xBA";
        }

        return str;
    }
}

bool TireHandler::sensorTimedOut(uint16_t index) {
    bool result = false;
    TPMSPacket* sensor = &_sensorPackets[index];

    if (sensor->id != 0) {
        if (sensor->pressure != timedOutValue) {
            uint32_t timeStamp = sensor->timeStamp;

            if (timeStamp) {
                uint32_t stampAge = millis() - timeStamp;

                if (stampAge > sensorTimeout) {
                    sensor->pressure = timedOutValue;
                    sensor->temperature = timedOutValue;
                }
            }
        }
        result = sensor->pressure == timedOutValue;
    }
    return result;
}

uint32_t TireHandler::idForSensor(uint16_t index) {
    return _prefData.sensorIDs[index];
}

float TireHandler::pressureForSensor(uint16_t index) {
    TPMSPacket* sensor = &_sensorPackets[index];

    if (sensorTimedOut(index)) {
        return timedOutValue;
    }
    else if (sensor->id==0) {
        return noDataValue;
    }
    else {
        return sensor->pressure;
    }
}

float TireHandler::temperatureForSensor(uint16_t index) {
    TPMSPacket* sensor = &_sensorPackets[index];

    if (sensorTimedOut(index)) {
        return timedOutValue;
    }
    else if (sensor->id==0) {
        return noDataValue;
    }
    else {
        return sensor->temperature;
    }
}

TireHandler _tireHandler;
