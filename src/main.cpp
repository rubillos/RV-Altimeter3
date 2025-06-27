#include <Arduino.h>

#include "defs.h"
#include "heltec.h"
#include "TimeLib.h"
#include <elapsedMillis.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include "Adafruit_RA8875.h"
#include "Buffer8.h"
#include <Timezone.h>
#include "sunset.h"
#include "RingBuff.h"
#include "dataDisplay.h"
#include "png.h"
#include "packets.h"
#include "touchscreen.h"
#include "menu.h"
#include "mainMenu.h"
#include "tires.h"
#include "gps.h"
#include "prefs.h"
#include "OLED_Print.h"
#include "pins.h"
#include "pairMenu.h"

#include "graphics/PX3_Flat.png.h"

SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SunSet sun;

constexpr float voltageScale = 19.8;
constexpr float runningVoltage = 13.0;

constexpr uint32_t screenDimDelay = 20 * 60 * 1000; // 20 minutes

bool have12v = false;
bool switchClosed = false;

#define LOG_DEPTH 16

constexpr uint16_t lightAverageCount = 16;
constexpr uint16_t analogResolution = 12;
constexpr float analogMax = (1 << analogResolution);

RingBuff<float> lightBuff(100);

float readLight() {
	uint32_t total = 0;
	
	for (uint16_t i=0; i<lightAverageCount; i++) {
		total += analogRead(LIGHT_SENSOR_PIN);
	}

	lightBuff.addSample((float)total / (lightAverageCount * analogMax));

	return lightBuff.average();
}

void scanBus(TwoWire &bus) {
	Serial.println("Scanning Bus...");
	for (uint16_t i=1; i<127; i++) {
		bus.beginTransmission(i);
		byte error = bus.endTransmission();
		if (error == 0) {
			Serial.print("Device found at addr 0x");
			Serial.println(String(i, HEX));
		}
	}
	Serial.println("Scan Complete.");
}

float sequenceInterp(float* values, int16_t count, float percent) {
	uint16_t startIndex = (float)count * percent;
	float frac = ((float)count * percent) - startIndex;
	float startValue = values[startIndex];
	float endValue = values[(startIndex + 1) % count];

	return startValue + (endValue - startValue) * frac;
}

void doCalibrate() {
	if (_touchScreen.runCalibration(&_prefData.touchCalibration)) {
		_touchScreen.setTouchMatrix(&_prefData.touchCalibration);
		Serial.println("Writing prefs.");
		_prefs.writePrefs();
	}
}

void packetCheck() {
	TPMSPacket packet;

	if (_packetMonitor.getPacket(&packet)) {
		Serial.printf("id=0x%X, pressure=%0.1f, temperature=%0.1f\n", packet.id, packet.pressure, packet.temperature);
		_tireHandler.recordPacket(packet);
	}
}

bool switchState() {
	pinMode(SWITCH_AND_12V_PIN, INPUT_PULLUP);
	pinMode(SWITCH_GND_PIN, OUTPUT);
	digitalWrite(SWITCH_GND_PIN, LOW);

	bool switchClosed = digitalRead(SWITCH_AND_12V_PIN) == LOW;

	pinMode(SWITCH_GND_PIN, INPUT);
	pinMode(SWITCH_AND_12V_PIN, INPUT);

	return switchClosed;
}

float voltageValue() {
	pinMode(SWITCH_GND_PIN, INPUT);
	pinMode(SWITCH_AND_12V_PIN, INPUT);

	uint16_t value = analogRead(SWITCH_AND_12V_PIN);

	if (value < 10) {
		value = 0;
	}

	// Serial.printf("voltageValue=%d\n", value);
	
	return (float)value / analogMax * voltageScale;
}

void soundTest() {
	constexpr uint32_t frequency = 100;
	constexpr uint32_t duration = 100;
	constexpr uint32_t interval = 1000000 / frequency;
	constexpr uint32_t offDuration = interval - duration;

	for (int i=0; i<frequency; i++) {
		digitalWrite(BUZZER_PIN, LOW);
		delayMicroseconds(duration);
		digitalWrite(BUZZER_PIN, HIGH);
		delayMicroseconds(offDuration);
	}
}

void setup() {
	delay(3000);
	Serial.begin(115200);
	Serial.flush();
	delay(50);

	Serial.println("Begin Startup");

	// soundTest();

	analogReadResolution(analogResolution);

	Serial.printf("Voltage: %0.2f\n", voltageValue());

	have12v = voltageValue() > 10.0;
	switchClosed = switchState();

	Serial.printf("12v Power: %s, Switch Closed: %s\n", have12v ? "yes":"no", switchClosed ? "yes":"no");

	Serial.println("Init OLED");
	pinMode(VEXT_PIN, OUTPUT);			// turn on VExt
	digitalWrite(VEXT_PIN, LOW);

	displayOLED.init();
	displayOLED.flipScreenVertically();
	displayOLED.setFont(ArialMT_Plain_10);
	displayOLED.setLogBuffer(6, 40);
	OLEDinited = true;

	// test_zones();
	OLEDprintln("Init Prefs...");
	_prefs.readPrefs();

	OLEDprintln("Init TFT display...");
	_touchScreen.startDisplay(have12v);

	OLEDprintln("Show splash screen...");
	drawPNG8(PX3_Flat_png, sizeof(PX3_Flat_png), &_display, 0, 0, true);
	delay(2000);

	OLEDprintln("Init Touchscreen...");
	_touchScreen.beginTouch();

	OLEDprintln("Init PacketRadio...");
	_packetMonitor.begin();
	// _packetMonitor.setFakePackets(true);

	OLEDprintln("Init GPS...");
	_gps.begin();
	
	OLEDprintln("Init Modules...");
	_dataDisplay.begin();
	menuInit();

	OLEDprintln("Restore Tire Pressures...");
	_tireHandler.restoreSavedTireData();

	OLEDprintln("Check touchscreen...");
	if (!_prefData.touchCalibration.Divider) {
		Serial.println("Calibrating...");
		doCalibrate();
	}

	// _prefData.sensorIDs[0] = 0;
	// _prefData.sensorIDs[1] = 0;
	// _prefData.sensorIDs[2] = 0;
	// _prefData.sensorIDs[3] = 0;
	// _prefData.sensorIDs[4] = 0;
	// _prefData.sensorIDs[5] = 0;

	OLEDprintln("Init Done!");
}

bool allowScreenDim = false;

bool screenCanDim() {
	return allowScreenDim;
}

void setScreenCanDim(bool canDim) {
	allowScreenDim = canDim;
}

void systemUpdate() {
	packetCheck();
	_gps.update();
	_tireHandler.checkSensorData(_gpsData.movingSeconds>(2*60) || _gpsData.stoppedSeconds>(60*60)); // moving>2mins || stopped>60mins
}

void loop() {
	static uint32_t drawTime;
	static elapsedMillis engineStoppedTime;

	systemUpdate();

	static uint16_t drawIndex = 0;
	static elapsedMillis statusTime = 500;

	if (statusTime >= 300) {
		statusTime = 0;

		float light = readLight();

		if (_gpsData.haveFix) {
			sun.setPosition(_gpsData.latitude, _gpsData.longitude, _gpsData.zoneOffset);
		}

		DateTime sunriseDate = _gpsData.localTimeDate;
		DateTime sunsetDate = _gpsData.localTimeDate;
		uint32_t curMinutes = sunriseDate.hour()*60+sunriseDate.minute();

		sun.setCurrentDate(sunriseDate.year(), sunriseDate.month(), sunriseDate.day());
		uint32_t sunriseTime = sun.calcSunrise();

		if (curMinutes > sunriseTime) {
			sunriseDate = sunriseDate + TimeSpan(1, 0, 0, 0);
			sun.setCurrentDate(sunriseDate.year(), sunriseDate.month(), sunriseDate.day());
			sunriseTime = sun.calcSunrise();
		}

		sun.setCurrentDate(sunsetDate.year(), sunsetDate.month(), sunsetDate.day());
		uint32_t sunsetTime = sun.calcSunset();

		if (curMinutes > sunsetTime) {
			sunsetDate = sunsetDate + TimeSpan(1, 0, 0, 0);
			sun.setCurrentDate(sunsetDate.year(), sunsetDate.month(), sunsetDate.day());
			sunsetTime = sun.calcSunset();
		}

		String sunUp = _dataDisplay.timeFromDayMinutes(sunriseTime, false);
		String sunDown = _dataDisplay.timeFromDayMinutes(sunsetTime, false);

		uint32_t curTime = _gpsData.localTimeDate.hour() * 60 + _gpsData.localTimeDate.minute();

		elapsedMillis drawStart;
		while (!_touchScreen.touchReady() && !_dataDisplay.showData(&drawIndex, curTime, &_gpsData, sunriseTime, sunsetTime)) {
			packetCheck();
		}
		drawTime = drawStart;
	}

	tsPoint_t touchPt;
	if (_touchScreen.screenTouch(&touchPt)) {
		if (_touchScreen.backlightEnabled()) {
			int16_t tireIndex = _tireHandler.indexOfTireAtPoint(touchPt);

			if (tireIndex != -1 && _tireHandler.pressureForSensor(tireIndex)==sensorNotPaired) {
				drawIndex = 0;
				_tireHandler.pressTire(tireIndex);
				delay(200);
				runPairMenu(tireIndex);
			}
			else if (touchPt.y < topRowY) {
				_dataDisplay.toggleDrawGraphs();
				drawIndex = 0;
			}
			else if (touchPt.y < tireTopY) {
				drawIndex = 0;
				runMainMenu();
			}
			else {
				_tireHandler.showTemperature();
				statusTime = 500;
			}
		}
		engineStoppedTime = screenDimDelay / 2;				// set half sleep wait time
	}

	if (voltageValue() > runningVoltage) {
		engineStoppedTime = 0;
	}

	_touchScreen.enableBacklight(engineStoppedTime < screenDimDelay || !allowScreenDim);

	static elapsedMillis showTime;
	if (showTime > 500) {
		Serial.printf("%06d: Draw time=%dms, free memory=%d\n", millis(), drawTime, ESP.getFreeHeap());
		showTime = 0;
	}

	static bool oledCleared = false;
	static elapsedMillis oledClearTime;
	if (!oledCleared && oledClearTime>12000) {
		OLEDclear();
		oledCleared = true;
	}
}
