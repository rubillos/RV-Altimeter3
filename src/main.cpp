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
#include "accel.h"
#include "OLED_print.h"
#include "pins.h"

#include <esp_sleep.h>

#include "graphics/PX3_Flat.png.h"

SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SunSet sun;

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
	
	return (float)value / analogMax * 32.0;
}

void setup() {
	delay(3000);
	Serial.begin(115200);
	Serial.flush();
	delay(50);

	Serial.println("Begin Startup");

	analogReadResolution(analogResolution);

	Serial.printf("Voltage: %0.2f\n", voltageValue());

	have12v = voltageValue() > 15.0;
	switchClosed = switchState();

	Serial.printf("12v Power: %s, Switch Closed: %s\n", have12v ? "yes":"no", switchClosed ? "yes":"no");

	// Serial.println("Scan Bus");
	// displayOLED.connect();
	// scanBus(Wire);

	Serial.println("Init OLED");
	displayOLED.init();
	displayOLED.flipScreenVertically();
	displayOLED.setFont(ArialMT_Plain_10);
	displayOLED.setLogBuffer(6, 40);
	OLEDinited = true;

	OLEDprintln("GPS Altometer");

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
	_packetMonitor.setFakePackets(true);

	OLEDprintln("Init GPS...");
	_gps.begin();
	
	OLEDprintln("Init Accelerometer...");
	_accel.begin();
	_accel.setShakeLimits(0.4, 0.4, 0.4);
	
	OLEDprintln("Init Tire Drawing...");
	_tireHandler.begin(300);

	OLEDprintln("Check touchscreen...");
	if (!_prefData.touchCalibration.Divider) {
		Serial.println("Calibrating...");
		doCalibrate();
	}

	// _prefData.sensorIDs[0] = 0xAA2365;
	// _prefData.sensorIDs[1] = 0xAA6721;
	// _prefData.sensorIDs[2] = 0x437812;
	// _prefData.sensorIDs[3] = 0x327812;
	// _prefData.sensorIDs[4] = 0xAA7712;
	// _prefData.sensorIDs[5] = 0x213876;

	OLEDprintln("Init Done!");
}

void sleepUntilTouch() {
	bool done = false;
	tsPoint_t touchPt;

	_gps.setPowerSaveMode(true);
	_touchScreen.enableBacklight(false);

	while (!done) {
		esp_sleep_enable_timer_wakeup(50 * 1000); // 50ms
		esp_light_sleep_start();

		if (_touchScreen.screenTouch(&touchPt) || _accel.didShake()) {
			done = true;
		}
	}

	_gps.setPowerSaveMode(false);
	_touchScreen.enableBacklight(true);
}

const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };

bool systemUpdate() {
	bool haveHadFix;

	packetCheck();
	_accel.update();
	haveHadFix = _gps.update();
	_tireHandler.checkSensorData(_gpsData.movingSeconds>0);

	return haveHadFix;
}

void loop() {
	static uint32_t drawTime;

	bool haveHadFix = systemUpdate();

	static uint16_t drawIndex = 0;
	static elapsedMillis statusTime = 500;
	if (statusTime >= 300) {
		statusTime = 0;

		float light = readLight();

		sun.setPosition(_gpsData.latitude, _gpsData.longitude, _gpsData.zoneOffset);
		sun.setCurrentDate(_gpsData.gpsTimeDate.year(), _gpsData.gpsTimeDate.month(), _gpsData.gpsTimeDate.day());

		uint32_t sunriseTime = sun.calcSunrise();
		uint32_t sunsetTime = sun.calcSunset();

		String sunUp = _dataDisplay.timeFromDayMinutes(sunriseTime, false);
		String sunDown = _dataDisplay.timeFromDayMinutes(sunsetTime, false);

		uint32_t curTime = _gpsData.gpsTimeDate.hour() * 60 + _gpsData.gpsTimeDate.minute();

		elapsedMillis drawStart;
		while (!_touchScreen.touchReady() && !_dataDisplay.showData(&drawIndex, curTime, _gpsData.altitude, _gpsData.heading, _gpsData.speed, sunriseTime, sunsetTime, _gpsData.satellites, haveHadFix, fixNames[_gpsData.fixType])) {
			packetCheck();
			_accel.update();
		}
		drawTime = drawStart;
	}

	tsPoint_t touchPt;
	if (_touchScreen.screenTouch(&touchPt)) {
		int16_t tireIndex = _tireHandler.indexOfTireAtPoint(touchPt);

		if (tireIndex != -1 && _tireHandler.pressureForSensor(tireIndex)==sensorNotPaired) {
			_tireHandler.pressTire(tireIndex);
			delay(200);
		}
		else if (touchPt.y < 300) {
			drawIndex = 0;
			runMainMenu();
		}
		else {
			_tireHandler.showTemperature();
			statusTime = 500;
		}
	}

	static elapsedMillis showTime;
	if (showTime > 500) {
		Serial.printf("%06d: Draw time=%dms, free memory=%d\n", millis(), drawTime, ESP.getFreeHeap());

		// sensors_event_t movement;
		// _accel.getEvent(movement);
		// Serial.printf("Accel: x=%f, y=%f, z=%f\n", movement.acceleration.x, movement.acceleration.y, movement.acceleration.z);

		if (_accel.didShake()) {
			Serial.println("Shake!");
		}

		showTime = 0;
	}

	static bool oledCleared = false;
	static elapsedMillis oledClearTime;
	if (!oledCleared && oledClearTime>12000) {
		OLEDclear();
		oledCleared = true;
	}
}
