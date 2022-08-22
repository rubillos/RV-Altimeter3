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
#include <RTClib.h>
#include "RingBuff.h"
#include "dataDisplay.h"
#include "png.h"
#include "packets.h"
#include "touchscreen.h"
#include <Preferences.h>
#include "button.h"
#include "menu.h"
#include "tires.h"
#include "gps.h"
#include "defs.h"

#include "OLED_print.h"

#include "graphics/PX3_Flat.png.h"

// GPS - yellow SCL, blue SDA

// #define GPS_SCL 15
// #define GPS_SDA 4

#define LIGHT_SENSOR_PIN 38

#define SWITCH_AND_12V_PIN 2
#define SWITCH_GND_PIN 13

SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SunSet sun;

Buffer8 buffer(0, 0, cellWidth, cellHeight);

Preferences preferences;

PacketMonitor packetMonitor;
TouchScreen touchScreen;
Menu menu;
TireHandler tireHandler;
GPS gps;

bool have12v = false;
bool switchClosed = false;

#define PREFS_VERSION 3
#define LOG_DEPTH 16
#define NUM_TIRES 6

struct {
	uint32_t sensorIDs[NUM_TIRES];

	float alarmPressureMin;
	float alarmPressureMax;
	float alarmTempMax;

	tsMatrix_t touchCalibration;
} prefData;

TPMSPacket sensorPackets[NUM_TIRES];

const char* sensorKeys[] = { "id0", "id1", "id2", "id3", "id4", "id5" };

void readPrefs() {
	preferences.begin("altometer", false);
	if (PREFS_VERSION != preferences.getUInt("version")) {
		Serial.println("Prefs version does not match, clearing.");
		preferences.clear();
	}

	for (uint8_t i=0; i<NUM_TIRES; i++) {
		prefData.sensorIDs[i] = preferences.getULong(sensorKeys[i], 0);
	}
	prefData.alarmPressureMin = preferences.getFloat("pressMin", 80.0);
	prefData.alarmPressureMax = preferences.getFloat("pressMax", 115.0);
	prefData.alarmTempMax = preferences.getFloat("tempMax", 158.0);
	if (preferences.getBytes("touchMatrix", &prefData.touchCalibration, sizeof(prefData.touchCalibration))) {
		touchScreen.setTouchMatrix(&prefData.touchCalibration);
	}
}

void writePrefs() {
	preferences.putUInt("version", PREFS_VERSION);
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		preferences.putULong(sensorKeys[i], prefData.sensorIDs[i]);
	}
	preferences.putFloat("pressMin", prefData.alarmPressureMin);
	preferences.putFloat("pressMax", prefData.alarmPressureMax);
	preferences.putFloat("tempMax", prefData.alarmTempMax);
	preferences.putBytes("touchMatrix", &prefData.touchCalibration, sizeof(prefData.touchCalibration));
}

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
	if (touchScreen.runCalibration(&prefData.touchCalibration)) {
		touchScreen.setTouchMatrix(&prefData.touchCalibration);
		Serial.println("Writing prefs.");
		writePrefs();
	}
}

void packetCheck() {
	TPMSPacket packet;

	if (packetMonitor.getPacket(&packet)) {
		Serial.printf("id=0x%X, pressure=%0.1f, temperature=%0.1f\n", packet.id, packet.pressure, packet.temperature);
		tireHandler.recordPacket(packet);
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
	readPrefs();

	OLEDprintln("Init TFT display...");
	touchScreen.startDisplay(have12v);

	drawPNG8(PX3_Flat_png, sizeof(PX3_Flat_png), &_tft, 0, 0, true);
	delay(2000);

	OLEDprintln("Init Touchscreen...");
	touchScreen.beginTouch();

	OLEDprintln("Init PacketRadio...");
	packetMonitor.begin();

	OLEDprintln("Init GPS...");
	gps.begin();
	
	OLEDprintln("Init Menu System...");
	menu.begin(&touchScreen, &packetMonitor, &tireHandler, &prefData.alarmPressureMin, &prefData.alarmPressureMax, &prefData.alarmTempMax);

	tireHandler.begin(_tft, buffer, 300, prefData.sensorIDs,
						&prefData.alarmPressureMin, &prefData.alarmPressureMax, &prefData.alarmTempMax);

	OLEDprintln("Check touchscreen...");
	if (!prefData.touchCalibration.Divider) {
		Serial.println("Calibrating...");
		doCalibrate();
	}

	OLEDprintln("Init Done!");
}

const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };

constexpr float movingThreshold = 5.0;

void loop() {
	static bool haveHadFix = false;
	static GPS_Data gpsData;
	static elapsedMillis gpsDataTime;
	static uint32_t drawTime;
	static elapsedSeconds movingSeconds;
	static elapsedSeconds stoppedSeconds;

	static elapsedMillis gpsTime = 800;
	if (gpsTime >= 1000) {
		gpsTime = 0;
		gps.getData(gpsData);
		if (gpsData.haveFix) {
			haveHadFix = true;
			gpsDataTime = 0;
		}
		if (gpsData.speed >= movingThreshold) {
			stoppedSeconds = 0;
		}
		else {
			movingSeconds = 0;
		}
	}

	static uint16_t drawIndex = 0;
	static elapsedMillis statusTime = 500;
	if (statusTime >= 300) {
		statusTime = 0;

		float light = readLight();

		sun.setPosition(gpsData.latitude, gpsData.longitude, gpsData.zoneOffset);
		sun.setCurrentDate(gpsData.gpsTimeDate.year(), gpsData.gpsTimeDate.month(), gpsData.gpsTimeDate.day());

		uint32_t sunriseTime = sun.calcSunrise();
		uint32_t sunsetTime = sun.calcSunset();
		String sunUp = timeFromDayMinutes(sunriseTime, false);
		String sunDown = timeFromDayMinutes(sunsetTime, false);

		uint32_t curTime = gpsData.gpsTimeDate.hour() * 60 + gpsData.gpsTimeDate.minute();

		Serial.printf("GPS Data: Alt=%0.1f, Heading=%0.2f, Sunrise=%s, Sunset=%s, GMTOffset=%d, fix=%d, sat#=%d, lat=%0.3f, lon=%0.3f, speed=%0.1f, light=%0.2f\n",
								gpsData.altitude, gpsData.heading, sunUp, sunDown, gpsData.zoneOffset, gpsData.haveFix, gpsData.satellites,
								gpsData.latitude,  gpsData.longitude, gpsData.speed, light);

		elapsedMillis drawStart;
		while (!touchScreen.touchReady() && !showData(&drawIndex, curTime, gpsData.altitude, gpsData.heading, gpsData.speed, sunriseTime, sunsetTime, gpsData.satellites, haveHadFix, fixNames[gpsData.fixType])) {
			packetCheck();
		}
		drawTime = drawStart;
	}

	packetCheck();

	tsPoint_t touchPt;
	if (touchScreen.screenTouch(&touchPt)) {
		if (touchPt.y < 300) {
			drawIndex = 0;
			menu.run();
		}
		else {
			tireHandler.showTemperature();
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
