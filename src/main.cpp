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
#include "beep.h"
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

#define RA8875_SCK 17
#define RA8875_MISO 36
#define RA8875_MOSI 23
#define RA8875_CS 22
#define RA8875_RESET 21
#define RA8875_INT 37
#define RA8875_WAIT 39

#define BUZZER_PIN 25
#define LIGHT_SENSOR_PIN 38

#define SWITCH_AND_12V_PIN 2
#define SWITCH_GND_PIN 13

SPIClass LCD_SPI(HSPI);
Adafruit_RA8875 display = Adafruit_RA8875(RA8875_CS, RA8875_RESET);
SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SunSet sun;

Buffer8 buffer(0, 0, cellWidth, cellHeight);

Preferences preferences;

PacketMonitor packetMonitor;
TouchScreen touchScreen;
Menu menu;
TireHandler tireHandler;
GPS gps;
Beeper beeper(BUZZER_PIN, LOW);

bool have12v = false;
bool switchClosed = false;

#define PREFS_VERSION 2
#define LOG_DEPTH 16
#define NUM_TIRES 6

struct {
	uint32_t sensor_ids[NUM_TIRES];

	float alarm_pressure_min;
	float alarm_pressure_max;
	float alarm_temp_max;

	tsMatrix_t touch_calibration;
} pref_data;

TPMS_Packet sensor_packets[NUM_TIRES];

const char* sensorKeys[] = { "id0", "id1", "id2", "id3", "id4", "id5" };

void readPrefs() {
	preferences.begin("altometer", false);
	if (PREFS_VERSION != preferences.getUInt("version")) {
		Serial.println("Prefs version deos not match, clearing.");
		preferences.clear();
	}

	for (uint8_t i=0; i<NUM_TIRES; i++) {
		pref_data.sensor_ids[i] = preferences.getULong(sensorKeys[i], 0);
	}
	pref_data.alarm_pressure_min = preferences.getFloat("press_min", 80.0);
	pref_data.alarm_pressure_max = preferences.getFloat("press_max", 115.0);
	pref_data.alarm_temp_max = preferences.getFloat("temp_max", 158.0);
	if (preferences.getBytes("touch_matrix", &pref_data.touch_calibration, sizeof(pref_data.touch_calibration))) {
		touchScreen.setTouchMatrix(&pref_data.touch_calibration);
	}
}

void writePrefs() {
	preferences.putUInt("version", PREFS_VERSION);
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		preferences.putULong(sensorKeys[i], pref_data.sensor_ids[i]);
	}
	preferences.putFloat("press_min", pref_data.alarm_pressure_min);
	preferences.putFloat("press_max", pref_data.alarm_pressure_max);
	preferences.putFloat("temp_max", pref_data.alarm_temp_max);
	preferences.putBytes("touch_matrix", &pref_data.touch_calibration, sizeof(pref_data.touch_calibration));
}

constexpr uint16_t light_average_count = 16;
constexpr uint16_t analog_resolution = 12;
constexpr float light_max = 4096.0;

RingBuff<float> lightBuff(100);

float readLight() {
	uint32_t total = 0;
	
	for (uint16_t i=0; i<light_average_count; i++) {
		total += analogRead(LIGHT_SENSOR_PIN);
	}

	lightBuff.addSample((float)total / (light_average_count * light_max));

	return lightBuff.average();
}

void startDisplay() {
	LCD_SPI.begin(RA8875_SCK, RA8875_MISO, RA8875_MOSI, RA8875_CS);

	delay(1000);
	if (!display.begin(RA8875_800x480, &LCD_SPI)) {
		Serial.println("RA8875 Not Found!");
		while (1);
	}

	display.setLayerMode(true);
	display.graphicsMode();                 // go back to graphics mode
	display.fillScreen(BLACK8);
	delay(1);

	display.displayOn(true);
	delay(1);

	display.GPIOX(true);      // Enable display - display enable tied to GPIOX
	delay(100);

	display.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	delay(100);

	display.PWM1out((have12v) ? 255 : 127);	// set backlight
	display.setWaitPin(RA8875_WAIT);
}

void scan_bus(TwoWire &bus) {
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
	if (touchScreen.runCalibration(&pref_data.touch_calibration)) {
		touchScreen.setTouchMatrix(&pref_data.touch_calibration);
		Serial.println("Writing prefs.");
		writePrefs();
	}
}

void packetCheck() {
	TPMS_Packet packet;

	if (packetMonitor.getPacket(&packet)) {
		// Serial.println("Got radio packet.");
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

	analogReadResolution(analog_resolution);

	pinMode(SWITCH_GND_PIN, INPUT);
	pinMode(SWITCH_AND_12V_PIN, INPUT);
	
	have12v = analogRead(SWITCH_AND_12V_PIN) > 1500;

	pinMode(SWITCH_AND_12V_PIN, INPUT_PULLUP);
	pinMode(SWITCH_GND_PIN, OUTPUT);
	digitalWrite(SWITCH_GND_PIN, LOW);

	switchClosed = digitalRead(SWITCH_AND_12V_PIN) == LOW;

	Serial.printf("12v Power: %s, Switch Closed: %s\n", have12v ? "yes":"no", switchClosed ? "yes":"no");

	// Serial.println("Scan Bus");
	// displayOLED.connect();
	// scan_bus(Wire);

	Serial.println("Init OLED");
	displayOLED.init();
	displayOLED.flipScreenVertically();
	displayOLED.setFont(ArialMT_Plain_10);
	displayOLED.setLogBuffer(6, 40);
	OLED_inited = true;

	Serial.println("GPS Altometer");

	// test_zones();
	OLED_println("Init Prefs...");
	readPrefs();

	OLED_println("Init TFT display...");
	startDisplay();

	drawPNG8(PX3_Flat_png, sizeof(PX3_Flat_png), &display, 0, 0, true);
	delay(2000);

	OLED_println("Init Touchscreen...");
	pinMode(RA8875_WAIT, INPUT_PULLUP);
	touchScreen.begin(display, RA8875_INT, &beeper);

	Serial.println("Init PacketRadio...");
	packetMonitor.begin();

	Serial.println("Init GPS...");
	gps.begin();
	
	Serial.println("Init Menu System...");
	menu.begin(&display, &touchScreen, &packetMonitor, &tireHandler, &pref_data.alarm_pressure_min, &pref_data.alarm_pressure_max, &pref_data.alarm_temp_max);

	tireHandler.begin(display, buffer, beeper, 300, pref_data.sensor_ids,
						&pref_data.alarm_pressure_min, &pref_data.alarm_pressure_max, &pref_data.alarm_temp_max);

	Serial.println("Check touchscreen...");
	if (!pref_data.touch_calibration.Divider) {
		Serial.println("Calibrating...");
		doCalibrate();
	}

	Serial.println("Init Done!");
}

const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };

void loop() {
	static bool haveHadFix = false;
	static GPS_Data gpsData;
	static elapsedMillis gpsDataTime;
	static uint32_t drawTime;

	static elapsedMillis gpsTime = 800;
	if (gpsTime >= 1000) {
		gpsTime = 0;
		gps.getData(gpsData);
		if (gpsData.haveFix) {
			haveHadFix = true;
			gpsDataTime = 0;
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

		// Serial.printf("GPS Data: Alt=%0.1f, Heading=%0.2f, Sunrise=%s, Sunset=%s, GMTOffset=%d, fix=%d, sat#=%d, lat=%0.3f, lon=%0.3f, speed=%0.1f, light=%0.2f\n",
		// 						gpsData.altitude, gpsData.heading, sunUp, sunDown, gpsData.zoneOffset, gpsData.haveFix, gpsData.satellites,
		// 						gpsData.latitude,  gpsData.longitude, gpsData.speed, light);

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
		OLED_Clear();
		oledCleared = true;
	}
}
