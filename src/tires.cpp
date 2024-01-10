#include "tires.h"

#include "defs.h"
#include "touchscreen.h"
#include "prefs.h"
#include "png.h"

// https://tchapi.github.io/Adafruit-GFX-Font-Customiser/
// ./fontconvert freefont-ttf/sfd/FreeSansBold.ttf 24 32 126 '~°' > Fonts/FreeSansBold24pt7bCustom.h

#include "fonts/FreeSansBold30pt7b.h"
#include "fonts/FreeSansBold24pt7bCustom.h"
#include "fonts/FreeSansBold14pt7b.h"
#include "graphics/Tire2.png.h"

constexpr uint32_t tireDataSaveInterval = 60 * 5;

constexpr uint16_t tireX[] = { 272, 416, 152, 272, 416, 536 };
constexpr uint16_t tireY[] = { tireTopY, tireTopY, tireTopY+80, tireTopY+80, tireTopY+80, tireTopY+80 };
constexpr uint16_t tireWidth = 110;
constexpr uint16_t tireHeight = 67;

const char* tirePressKeys[] = { "tirePress0", "tirePress1", "tirePress2", "tirePress3", "tirePress4", "tirePress5" };
const char* tireTempKeys[] = { "tireTemp0", "tireTemp1", "tireTemp2", "tireTemp3", "tireTemp4", "tireTemp5" };

const char* tireNames[] = {
	"Left Front",
	"Right Front",
	"Left Rear Outer",
	"Left Rear Inner",
	"Right Rear Inner",
	"Right Rear Outer"
};

const char* shortTireNames[] = {
	"Left Front",
	"Right Front",
	"Left Rear Outer",
	"Left Rear Inner",
	"Right Rear Inner",
	"Right Rear Outer"
};

void TireHandler::drawTires() {
	_display.fillRect(380, tireTopY+30, 40, 6, WHITE16);
	_display.fillRect(262, tireTopY+110, 280, 6, WHITE16);
	_display.fillRect(396, tireTopY+32, 6, 80, WHITE16);

	uint32_t time = millis();
	String str;

	for (uint16_t i=0; i<6; i++) {
		TPMSPacket* sensor = &_sensorPackets[i];
		int16_t xOffset = -1, yOffset = 0;

		float pressure = pressureForSensor(i);
		float temperature = temperatureForSensor(i);

		bool tempAlarm = temperatureAlarm(temperature);
		bool tempWarn = temperatureWarning(temperature);
		bool pressAlarm = pressureAlarm(pressure);
		bool pressWarn = pressureWarning(pressure);

		bool forceTemperature = tempWarn && ((time/1000) % 2)==0;

		_displayBuffer8.setOffset(tireX[i], tireY[i]);

		uint8_t tireColor;

		if (pressAlarm || tempAlarm) {
			tireColor = RED8;
		}
		else if (pressWarn || tempWarn) {
			tireColor = DARKORANGE8;
		}
		else {
			tireColor = WHITE8;
		}

		drawPNG83(Tire2_png, sizeof(Tire2_png), &_displayBuffer8, tireX[i], tireY[i], tireColor);

		if (pressure != sensorNotPaired) {
			_displayBuffer8.setTextColor(pressureColor(pressure));
			str = pressureString(pressure, false);

			if (pressure == sensorNotPaired) {
				_displayBuffer8.setFont(&FreeSansBold30pt7b);
			}
			else if (pressure == timedOutValue) {
				_displayBuffer8.setFont(&FreeSansBold30pt7b);
			}
			else if (pressure == noDataValue) {
				_displayBuffer8.setFont(&FreeSansBold30pt7b);
				yOffset = -6;
			}
			else if (forceTemperature || _tempTimer < temperatureTime) {
				_displayBuffer8.setFont(&FreeSansBold24pt7bCustom);
				_displayBuffer8.setTextColor(temperatureColor(temperature));
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
				yOffset -= 1;
				if (str.c_str()[0]=='1') {
					xOffset -= 4;
				}
			}

			_displayBuffer8.drawCenteredText(str, tireX[i] + tireWidth/2 + xOffset, tireY[i]+tireHeight-13+yOffset);
		}
		else {
			String line1 = "Not";
			String line2 = "Paired";

			_displayBuffer8.setFont(&FreeSansBold14pt7b);
			_displayBuffer8.setTextColor(RED8);

			_displayBuffer8.drawCenteredText(line1, tireX[i] + tireWidth/2 + xOffset, tireY[i]+tireHeight-36+yOffset);
			_displayBuffer8.drawCenteredText(line2, tireX[i] + tireWidth/2 + xOffset, tireY[i]+tireHeight-13+yOffset);
		}

		_displayBuffer8.draw(_display, tireWidth, tireHeight);
	}
}

int16_t TireHandler::indexOfTireAtPoint(tsPoint_t pt) {
	for (uint16_t i=0; i<numTires; i++) {
		if (pt.x>=tireX[i] && pt.x<=tireX[i]+tireWidth && pt.y>=tireY[i] && pt.y<=tireY[i]+tireHeight) {
			return i;
		}
	}
	return -1;
}

void TireHandler::pressTire(uint16_t index) {
	_display.fillRoundRect(tireX[index]+6, tireY[index]+11, tireWidth-13, tireHeight-23, 8, RA8875_WHITE);
}

void TireHandler::checkSensorData(bool moving) {
	if (_sensorCheckTime > 1000) {
		_sensorCheckTime = 0;

		uint32_t time = millis();

		for (uint16_t i=0; i<numTires; i++) {
			if (_prefData.sensorIDs[i]) {
				TPMSPacket* sensor = &_sensorPackets[i];

				if (sensor->timeStamp) {
					if (sensor->pressure >= 0) {
						if ((time-sensor->timeStamp)>sensorTimeout) {
							if (moving) {
								sensor->pressure = timedOutValue;
								sensor->temperature = timedOutValue;
							}
							else {
								sensor->pressure = -sensor->pressure;
								sensor->temperature = -sensor->temperature;
							}
						}
					}
				}
			}
		}
	}

	static elapsedSeconds tireSaveSeconds;
	if (moving && tireSaveSeconds > tireDataSaveInterval) {
		tireSaveSeconds = 0;

		Serial.print("Saving tire data.");

		for (uint16_t i=0; i<numTires; i++) {
			if (_prefData.sensorIDs[i]) {
				TPMSPacket* sensor = &_sensorPackets[i];

				float tirePressure = sensor->pressure;
				float tireTemperature = sensor->temperature;

				if (tirePressure == timedOutValue) {
					tirePressure = noDataValue;
				}
				if (tireTemperature == timedOutValue) {
					tireTemperature = noDataValue;
				}
				_preferences.putFloat(tirePressKeys[i], tirePressure);
				_preferences.putFloat(tireTempKeys[i], tireTemperature);
			}
		}
	}
}

void TireHandler::adjustForRadioReset() {
	uint32_t time = millis();

	for (uint16_t i=0; i<numTires; i++) {
		if (_prefData.sensorIDs[i]) {
			TPMSPacket* sensor = &_sensorPackets[i];

			if (sensor->timeStamp) {
				if (sensor->pressure >= 0) {
					sensor->timeStamp -= radioResetInterval;
				}
			}
		}
	}
}

void TireHandler::restoreSavedTireData() {
	uint32_t time = millis();

	for (uint16_t i=0; i<numTires; i++) {
		if (_prefData.sensorIDs[i]) {
			TPMSPacket* sensor = &_sensorPackets[i];

			sensor->timeStamp = time;
			sensor->pressure = _preferences.getFloat(tirePressKeys[i], noDataValue);
			sensor->temperature = _preferences.getFloat(tireTempKeys[i], noDataValue);

			if (sensor->pressure > 0) {
				sensor->pressure = -sensor->pressure;
			}
			if (sensor->temperature > 0) {
				sensor->temperature = -sensor->temperature;
			}
		}
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

void TireHandler::setSensorID(uint16_t sensorIndex, uint32_t id) {
	_prefData.sensorIDs[sensorIndex] = id;
	_sensorPackets[sensorIndex].id = id;
	_sensorPackets[sensorIndex].timeStamp = 0;
	_prefs.writePrefs();
}

bool TireHandler::temperatureWarning(float temperature) {
	return temperature>0 && temperature >= _prefData.alarmTempMax-5;

}

bool TireHandler::temperatureAlarm(float temperature) {
	return temperature>0 && temperature >= _prefData.alarmTempMax;

}

bool TireHandler::pressureWarning(float pressure) {
	return pressure>0 && ((pressure <= _prefData.alarmPressureMin+5) || (pressure >= _prefData.alarmPressureMax-5));

}

bool TireHandler::pressureAlarm(float pressure) {
	return pressure>0 && ((pressure <= _prefData.alarmPressureMin) || (pressure >= _prefData.alarmPressureMax));
}

const char* TireHandler::tireName(uint16_t index, bool shortName) {
	if (shortName) {
		return shortTireNames[index];
	}
	else {
		return tireNames[index];
	}
}

int32_t TireHandler::codedColor(float value) {
	if (value == sensorNotPaired) {
		return RED8;
	}
	else if (value == timedOutValue) {
		return RED8;
	}
	else if (value == noDataValue) {
		return ORANGE8;
	}
	else if (value < 0) {
		return DARK_GRAY8;
	}
	else {
		return -1;
	}
}

uint16_t TireHandler::pressureColor(float pressure) {
	int32_t color = codedColor(pressure);

	if (color == -1) {
		if (pressureAlarm(pressure)) {
			color = RED8;
		}
		else if (pressureWarning(pressure)) {
			color = ORANGE8;
		}
		else {
			color = GREEN8;
		}
	}
	return color;
}

uint16_t TireHandler::pressureColorForSensor(uint16_t index) {
	return pressureColor(pressureForSensor(index));
}

uint16_t TireHandler::temperatureColor(float temperature) {
	int32_t color = codedColor(temperature);

	if (color == -1) {
		if (temperatureAlarm(temperature)) {
			color = DARKORANGE8;
		}
		else if (temperatureWarning(temperature)) {
			color = YELLOW8;
		}
		else if (temperature < 33) {
			color = LIGHTBLUE8;
		}
		else {
			color = CYAN8;
		}
	}
	return color;
}

uint16_t TireHandler::temperatureColorForSensor(uint16_t index) {
	return temperatureColor(temperatureForSensor(index));
}

const char* TireHandler::noDataString() {
	return "--";
}

const char* TireHandler::timedOutString() {
	return "??";
}

String TireHandler::codedString(float value) {
	if (value == sensorNotPaired) {
		return "XX";
	}
	else if (value == noDataValue) {
		return noDataString();
	}
	else if (value == timedOutValue) {
		return timedOutString();
	}
	else {
		return "";
	}
}

String TireHandler::pressureString(float pressure, bool addPSI) {
	String str = codedString(pressure);

	if (str.length()==0) {
		str = String(abs(pressure), 0);

		if (addPSI) {
			str = str + "psi";
		}
	}
	return str;
}

String TireHandler::pressureStringForSensor(uint16_t index, bool addPSI) {
	return pressureString(pressureForSensor(index), addPSI);
}

String TireHandler::temperatureString(float temperature, uint8_t addDegrees) {
	String str = codedString(temperature);

	if (str.length()==0) {
		str = String(abs(temperature), 0);

		if (addDegrees == 1) {
			str = str + "~";
		}
		else if (addDegrees == 2) {
			str = str + "\xBA";
		}
	}
	return str;
}

String TireHandler::temperatureStringForSensor(uint16_t index, uint8_t addDegrees) {
	return temperatureString(temperatureForSensor(index), addDegrees);
}

uint32_t TireHandler::idForSensor(uint16_t index) {
	return _prefData.sensorIDs[index];
}

float stateOfSensor(TPMSPacket* sensor, uint16_t index, float value) {
	if (_prefData.sensorIDs[index]==0) {
		return sensorNotPaired;
	}
	else if (sensor->timeStamp==0) {
		return noDataValue;
	}
	else {
		return value;
	}
}

float TireHandler::pressureForSensor(uint16_t index) {
	TPMSPacket* sensor = &_sensorPackets[index];
	
	return stateOfSensor(sensor, index, sensor->pressure);
}

float TireHandler::temperatureForSensor(uint16_t index) {
	TPMSPacket* sensor = &_sensorPackets[index];

	return stateOfSensor(sensor, index, sensor->temperature);
}

TireHandler _tireHandler;
