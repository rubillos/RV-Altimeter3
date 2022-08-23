#include <Arduino.h>

#define defaultMaxWait 250

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
#include "tires.h"
#include "gps.h"
#include "prefs.h"
#include "defs.h"
#include "OLED_print.h"
#include "pins.h"

#include "graphics/PX3_Flat.png.h"

SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SunSet sun;

bool have12v = false;
bool switchClosed = false;

#define LOG_DEPTH 16

constexpr uint16_t lightAverageCount = 16;
constexpr uint16_t analogResolution = 12;
constexpr float lightMax = 4096.0;

RingBuff<float> lightBuff(100);

float readLight() {
	uint32_t total = 0;
	
	for (uint16_t i=0; i<lightAverageCount; i++) {
		total += analogRead(LIGHT_SENSOR_PIN);
	}

	lightBuff.addSample((float)total / (lightAverageCount * lightMax));

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

void setup() {
	delay(3000);
	Serial.begin(115200);
	Serial.flush();
	delay(50);

	Serial.println("Begin Startup");

	analogReadResolution(analogResolution);

	pinMode(SWITCH_GND_PIN, INPUT);
	pinMode(SWITCH_AND_12V_PIN, INPUT);
	
	have12v = analogRead(SWITCH_AND_12V_PIN) > 1500;

	pinMode(SWITCH_AND_12V_PIN, INPUT_PULLUP);
	pinMode(SWITCH_GND_PIN, OUTPUT);
	digitalWrite(SWITCH_GND_PIN, LOW);

	switchClosed = digitalRead(SWITCH_AND_12V_PIN) == LOW;

	pinMode(SWITCH_GND_PIN, INPUT);

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
	_prefs.readPrefs(_touchScreen);

	OLEDprintln("Init TFT display...");
	_touchScreen.startDisplay(have12v);

	drawPNG8(PX3_Flat_png, sizeof(PX3_Flat_png), &_display, 0, 0, true);
	delay(2000);

	OLEDprintln("Init Touchscreen...");
	_touchScreen.beginTouch();

	OLEDprintln("Init PacketRadio...");
	_packetMonitor.begin();

	OLEDprintln("Init GPS...");
	_gps.begin();
	
	OLEDprintln("Init Menu System...");
	_menu.begin();

	OLEDprintln("Init Tire Drawing...");
	_tireHandler.begin(300);

	OLEDprintln("Check touchscreen...");
	if (!_prefData.touchCalibration.Divider) {
		Serial.println("Calibrating...");
		doCalibrate();
	}

	OLEDprintln("Init Done!");
}

const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };

void loop() {
	static uint32_t drawTime;

	bool haveHadFix = _gps.update();

	static uint16_t drawIndex = 0;
	static elapsedMillis statusTime = 500;
	if (statusTime >= 300) {
		statusTime = 0;

		float light = readLight();

		sun.setPosition(_gpsData.latitude, _gpsData.longitude, _gpsData.zoneOffset);
		sun.setCurrentDate(_gpsData.gpsTimeDate.year(), _gpsData.gpsTimeDate.month(), _gpsData.gpsTimeDate.day());

		uint32_t sunriseTime = sun.calcSunrise();
		uint32_t sunsetTime = sun.calcSunset();
		String sunUp = timeFromDayMinutes(sunriseTime, false);
		String sunDown = timeFromDayMinutes(sunsetTime, false);

		uint32_t curTime = _gpsData.gpsTimeDate.hour() * 60 + _gpsData.gpsTimeDate.minute();

		Serial.printf("GPS Data: Alt=%0.1f, Heading=%0.2f, Sunrise=%s, Sunset=%s, GMTOffset=%d, fix=%d, sat#=%d, lat=%0.3f, lon=%0.3f, speed=%0.1f, light=%0.2f\n",
								_gpsData.altitude, _gpsData.heading, sunUp, sunDown, _gpsData.zoneOffset, _gpsData.haveFix, _gpsData.satellites,
								_gpsData.latitude,  _gpsData.longitude, _gpsData.speed, light);

		elapsedMillis drawStart;
		while (!_touchScreen.touchReady() && !showData(&drawIndex, curTime, _gpsData.altitude, _gpsData.heading, _gpsData.speed, sunriseTime, sunsetTime, _gpsData.satellites, haveHadFix, fixNames[_gpsData.fixType])) {
			packetCheck();
		}
		drawTime = drawStart;
	}

	packetCheck();

	tsPoint_t touchPt;
	if (_touchScreen.screenTouch(&touchPt)) {
		if (touchPt.y < 300) {
			drawIndex = 0;
			_menu.run();
		}
		else {
			_tireHandler.showTemperature();
			statusTime = 500;
		}
	}

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
