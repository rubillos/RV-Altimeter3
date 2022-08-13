#include <Arduino.h>

#define defaultMaxWait 250

#include "heltec.h"
#include "TimeLib.h"
#include <elapsedMillis.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include "Adafruit_RA8875.h"
#include "Buffer8.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Timezone.h>
#include "sunset.h"
#include <ZoneCalc.h>
#include <RTClib.h>
#include "RingBuff.h"
#include <PNGdec.h>
#include "packets.h"
#include "beep.h"
#include "touchscreen.h"
#include <Preferences.h>
#include "button.h"
#include "menu.h"

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "fonts/FreeSans44pt7b.h"
#include "fonts/FreeSans16pt7b.h"
#include "fonts/FreeSansBold30pt7b.h"
#include "fonts/ModFreeSansBold30pt.h"

#include "fonts/ModFreeSans44pt.h"

#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans12pt7b.h"

#include "fonts/IconFont.h"

#include "fonts/DejaVuSerifItalic12pt.h"
#include "fonts/DejaVuSerifBoldItalic24pt.h"
#include "fonts/DejaVuSerifBoldItalic30pt.h"
#include "fonts/DejaVuSerifBoldItalic56pt.h"

#include "graphics/Tire.png.h"

#ifdef DO_SERIAL
#define Debug_print(...) Serial.print(__VA_ARGS__)
#define Debug_println(...) Serial.println(__VA_ARGS__)
// #define Debug_println(str) { Heltec.display->clear(); Heltec.display->drawString(0, 0, str);	Heltec.display->display(); }
#define Debug_flush(...) Serial.flush()
#define Debug_begin(...) Serial.begin(__VA_ARGS__)
#define Debug_begin_wait(...) { Serial.begin(__VA_ARGS__); while (!Serial); Serial.println(); delay(250); }
#define Debug_delay(...) delay(__VA_ARGS__)
#define Debug_defined
#endif

#ifdef __GNUC__
#define NOT_USED __attribute__ ((unused))
#else
#define NOT_USED
#endif

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

#define Color8(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))
#define Color16(r, g, b) ((r & 0xE0) | ((g & 0xE0)>>3) | (b>>6))

#define BLACK8 Color8(0x00, 0x00, 0x00)
#define WHITE8 Color8(0xFF, 0xFF, 0xFF)
#define GREEN8 Color8(0x00, 0xFF, 0x00)
#define RED8 Color8(0xFF, 0x00, 0x00)
#define BLUE8 Color8(0x00, 0x00, 0xFF)
#define ORANGE8 Color8(0xFF, 0x80, 0x00)
#define YELLOW8 Color8(0xFF, 0xFF, 0x00)
#define CYAN8 Color8(0x00, 0xFF, 0xFF)
#define MAGENTA8 Color8(0xFF, 0x00, 0xFF)

#define DARK_GRAY8 0b01101101

#define WHITE16 0xFFFF

constexpr int16_t display_width = 800;
constexpr int16_t display_height = 480;

constexpr int16_t cellWidth = 340;
constexpr int16_t cellHeight = 70;

SPIClass LCD_SPI(HSPI);
Adafruit_RA8875 display = Adafruit_RA8875(RA8875_CS, RA8875_RESET);

SSD1306Wire displayOLED = SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);

SFE_UBLOX_GNSS gps;

ZoneCalc zoneCalc;
SunSet sun;

Buffer8 buffer(0, 0, cellWidth, cellHeight);

Menu menu;

#define PREFS_VERSION 1

#define NUM_TIRES 6
#define LOG_DEPTH 16

struct {
	uint32_t sensor_ids[NUM_TIRES];

	float alarm_pressure_min;
	float alarm_pressure_max;
	float alarm_temp_max;

	tsMatrix_t touch_calibration;
	bool touch_calibrated;
} pref_data;

TPMS_Packet sensor_packets[NUM_TIRES];

TPMS_Packet packet_log[LOG_DEPTH];
uint16_t packet_log_first = 0;
uint16_t packet_log_last = 0;

Preferences preferences;

const char* sensorKeys[] = { "id0", "id1", "id2", "id3", "id4", "id5" };

void readPrefs() {
	preferences.begin("altometer", false);
	if (PREFS_VERSION != preferences.getUInt("version")) {
		Debug_println("Prefs version deos not match, clearing.");
		preferences.clear();
	}

	for (uint8_t i=0; i<NUM_TIRES; i++) {
		pref_data.sensor_ids[i] = preferences.getULong(sensorKeys[i], 0);
	}
	pref_data.alarm_pressure_min = preferences.getFloat("press_min", 80.0);
	pref_data.alarm_pressure_max = preferences.getFloat("press_max", 115.0);
	pref_data.alarm_temp_max = preferences.getFloat("temp_max", 158.0);
	pref_data.touch_calibrated = preferences.getBool("touch_cal", false);
	preferences.getBytes("touch_matrix", &pref_data.touch_calibration, sizeof(pref_data.touch_calibration));
}

void writePrefs() {
	preferences.putUInt("version", PREFS_VERSION);
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		preferences.putULong(sensorKeys[i], pref_data.sensor_ids[i]);
	}
	preferences.putFloat("press_min", pref_data.alarm_pressure_min);
	preferences.putFloat("press_max", pref_data.alarm_pressure_max);
	preferences.putFloat("temp_max", pref_data.alarm_temp_max);
	preferences.putBool("touch_cal", pref_data.touch_calibrated);
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

uint8_t ascenderForFont(const GFXfont *f, char character = 'A')
{
	uint16_t index = character - f->first;
	int8_t offset = f->glyph[index].yOffset;

	return -offset;
}

void getStringDimensions(Adafruit_GFX& dest, String str, uint16_t* width, uint16_t* height) {
	int16_t x, y;

	dest.getTextBounds(str, 0, 0, &x, &y, width, height);
}

uint16_t getStringWidth(Adafruit_GFX& dest, String str)
{
	int16_t x, y;
	uint16_t width, height;

	dest.getTextBounds(str, 0, 0, &x, &y, &width, &height);
	return width;
}

PNG png;
int16_t pngX, pngY;
Adafruit_GFX* pngDest;

#define rBits 0b1110000000000000
#define gBits 0b0000011100000000
#define bBits 0b0000000000011000

#define rShift 8
#define gShift 6
#define bShift 3

#define RGB16to8(v) ((v&rBits)>>rShift | (v&gBits)>>gShift | (v&bBits)>>bShift)

void PNGDraw(PNGDRAW *pDraw) {
	uint16_t pixels[cellWidth];

	png.getLineAsRGB565(pDraw, pixels, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
	for (uint16_t i = 0; i<pDraw->iWidth; i++) {
		uint8_t pixel8 = RGB16to8(pixels[i]);
		if (pixel8) {
			pngDest->drawPixel(pngX+i, pngY+pDraw->y, pixel8);
		}
	}
}

void drawPNG(const unsigned char* data, uint16_t length, Adafruit_GFX* dest, int16_t x, int16_t y) {
	if (png.openRAM((uint8_t*)data, length, PNGDraw) == PNG_SUCCESS) {
		pngX = x;
		pngY = y;
		pngDest = dest;
		if (png.decode(NULL, 0) != PNG_SUCCESS) {
			Debug_println("PNG Decode Error!");
		}
	}
}

void startDisplay() {
	LCD_SPI.begin(RA8875_SCK, RA8875_MISO, RA8875_MOSI, RA8875_CS);

	if (!display.begin(RA8875_800x480, &LCD_SPI)) {
		Debug_println("RA8875 Not Found!");
		while (1);
	}

	display.displayOn(true);
	display.GPIOX(true);      // Enable display - display enable tied to GPIOX
	display.PWM1config(true, RA8875_PWM_CLK_DIV1024); // PWM output for backlight
	display.PWM1out(255);
	display.setLayerMode(true);
	// display.setWait(RA8875_WAIT);
	display.graphicsMode();                 // go back to graphics mode
	display.fillScreen(BLACK8);

	const GFXfont *f1 = &DejaVuSerifBoldItalic56;
	const GFXfont *f2 = &DejaVuSerifBoldItalic24;
	String str1 = String("GPS");
	String str2 = String("Altometer");
	String str3 = String("by Randy Ubillos");
	uint16_t w, h;

	display.setFont(f1);
	display.setTextSize(1);
	display.setTextColor(WHITE8);
	display.setTextWrap(false);

	getStringDimensions(display, str1, &w, &h);
	display.setCursor((display.width()-w)/2, 140);
	display.print(str1);

	getStringDimensions(display, str2, &w, &h);
	display.setCursor((display.width() - w) / 2, display.getCursorY() + 56);
	display.print(str2);

	display.setFont(f2);
	getStringDimensions(display, str3, &w, &h);
	display.setCursor((display.width() - w) / 2, display_height - 40);
	display.print(str3);
}

typedef struct {
	const char* name;
	float lat;
	float lon;
} CityRec;

// void test_zones() {
// #if defined(DO_SERIAL)
//   Debug_println("Zone Test");

//   CityRec cities[] = {
//     { "Boston", 42.360081, -71.058884 },
//     { "Chicago", 41.878113, -87.629799 },
//     { "Tampa", 27.950575, -82.457176 },
//     { "Denver", 39.739235, -104.990250 },
//     { "Houston", 29.760427, -95.369804 },
//     { "Alaska", 63.391522, -155.537651 },
//     { "San Francisco", 37.744657, -122.438970 },
//     { "Trout Creek", 47.836042, -115.593490 },
//     { "Heron MT", 48.093633, -116.033489 },
//     { "Claire Fork ID", 48.090157, -116.057808 },
//   };

//   int cityCount = sizeof(cities) / sizeof(CityRec);

//   for (int i=0; i<cityCount; i++) {
//     int sumZone = zoneOffsetForGPSCoord(cities[i].lat, cities[i].lon, true);
//     int wintZone = zoneOffsetForGPSCoord(cities[i].lat, cities[i].lon, false);
//     Debug_print(cities[i].name);
//     Debug_print(": summer=");
//     Debug_print(sumZone);
//     Debug_print(": winter=");
//     Debug_print(wintZone);
//     Debug_println();
//   }

//   while (1);
// #endif
// }

void scan_bus(TwoWire &bus) {
	Debug_println("Scanning Bus...");
	for (uint16_t i=1; i<127; i++) {
		bus.beginTransmission(i);
    	byte error = bus.endTransmission();
		if (error == 0) {
			Debug_print("Device found at addr 0x");
			Debug_println(String(i, HEX));
		}
	}
	Debug_println("Scan Complete.");
}

constexpr uint16_t oled_line_count = 6;
constexpr uint16_t oled_char_count = 32;
constexpr uint16_t oled_line_height = 10;

typedef struct {
	char line[oled_char_count];
} OLED_Line;

OLED_Line oled_lines[6];
uint16_t oled_line_index = 0;
uint16_t oled_char_index = 0;
bool oled_need_scroll = false;

void OLED_show() {
	displayOLED.clear();
	for (uint16_t i=0; i<oled_line_count; i++) {
		if (oled_lines[i].line[0]) {
			displayOLED.drawString(0, i*oled_line_height, oled_lines[i].line);
		}
	}
	displayOLED.display();
}

void OLED_scrollIfNeeded() {
	if (oled_need_scroll) {
		memmove(&oled_lines[0], &oled_lines[1], (oled_line_count-1) * oled_char_count);
		memset(&oled_lines[oled_line_count-1], 0, oled_char_count);
		oled_need_scroll = false;
	}
}

void OLED_println(const char* str) {
	Debug_println(str);

	OLED_scrollIfNeeded();
	if (oled_char_index < oled_char_count-1) {
		memcpy(&oled_lines[oled_line_index].line[oled_char_index], str, min((uint16_t)strlen(str), (uint16_t)(oled_char_count-1-oled_char_index)) );
	}
	oled_char_index = 0;
	if (oled_line_index < (oled_line_count-1)) {
		oled_line_index++;
	}
	else {
		oled_need_scroll = true;
	}
	OLED_show();
}

void OLED_println(int32_t val) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLED_println(valueBuff);
}

void OLED_println(float val) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLED_println(valueBuff);
}

void OLED_print(const char* str) {
	Debug_print(str);

	OLED_scrollIfNeeded();
	if (oled_char_index < oled_char_count-1) {
		uint16_t charsToCopy = min((uint16_t)strlen(str), (uint16_t)(oled_char_count-1-oled_char_index));
		memcpy(&oled_lines[oled_line_index].line[oled_char_index], str, charsToCopy );
		oled_char_index += charsToCopy;
	}
}

void OLED_print(int32_t val) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLED_print(valueBuff);
}

void OLED_print(float val) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLED_print(valueBuff);
}

void doCalibrate() {
	if (runCalibration(&pref_data.touch_calibration)) {
		pref_data.touch_calibrated = true;
		OLED_println("Writing prefs.");
		writePrefs();
	}
}

void setup() {
	pinMode(BUZZER_PIN, OUTPUT);
	digitalWrite(BUZZER_PIN, HIGH);

	Debug_delay(3000);
	Debug_begin(115200);
	Debug_flush();
	Debug_delay(50);

	Debug_println("Begin Startup");

	analogReadResolution(analog_resolution);

	Debug_println("Scan Bus");
	displayOLED.connect();
	scan_bus(Wire);

	Debug_println("Init OLED");
	displayOLED.init();
	// displayOLED.flipScreenVertically();
	displayOLED.setFont(ArialMT_Plain_10);
	displayOLED.setLogBuffer(6, 40);

	OLED_println("GPS Altometer");

	// test_zones();

	OLED_println("Init Prefs...");
	readPrefs();

	OLED_println("Init TFT display...");
	startDisplay();

	OLED_println("Init Touchscreen...");
	startTouch(display, RA8875_INT, RA8875_WAIT, BUZZER_PIN);

	OLED_println("Init PacketRadio...");
	packetsBegin();

	OLED_println("Init GPS...");
	if (!gps.begin(Wire)) { //Connect to the u-blox module using Wire port
		OLED_println("GPS not found!");
		while (1);
	}

	gps.setI2COutput(COM_TYPE_UBX);                 //Set the I2C port to output UBX only (turn off NMEA noise)
	gps.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
	if (!gps.setDynamicModel(DYN_MODEL_AUTOMOTIVE)) {
		OLED_println("* setDynamicModel failed!");
	}

	// gps.enableDebugging();

	delay(300);
	
	// Debug_print("GNSS is ");
	// Debug_println(gps.isConnected() ? "CONNECTED" : "not connected");
	// Debug_print("GPS is ");
	// Debug_println(gps.isGNSSenabled(SFE_UBLOX_GNSS_ID_GPS) ? "ENABLED" : "disabled");
	// Debug_print("GLONASS is ");
	// Debug_println(gps.isGNSSenabled(SFE_UBLOX_GNSS_ID_GLONASS) ? "ENABLED" : "disabled");
	// Debug_print("GALILEO is ");
	// Debug_println(gps.isGNSSenabled(SFE_UBLOX_GNSS_ID_GALILEO) ? "ENABLED" : "disabled");
	// Debug_print("SBAS is ");
	// Debug_println(gps.isGNSSenabled(SFE_UBLOX_GNSS_ID_SBAS) ? "ENABLED" : "disabled");

	OLED_println("Init Menu System...");
	menu.begin(&display, &pref_data.alarm_pressure_min, &pref_data.alarm_pressure_max, &pref_data.alarm_temp_max, &pref_data.touch_calibration);

	OLED_println("Check touchscreen...");
	if (!pref_data.touch_calibrated) {
		OLED_println("Calibrating...");
		doCalibrate();
	}

	OLED_println("Init Done!");
}

String timeFromDayMinutes(double dayMinutes, bool includeSeconds) {
	uint16_t hours = dayMinutes / 60;
	uint16_t minutes = ((uint32_t)dayMinutes) % 60;
	uint16_t seconds = (dayMinutes - (uint32_t)dayMinutes) * 60.0;

	// hours = 12;

	if (hours >= 13) {
		hours -= 12;
	}
	else if (hours < 1) {
		hours += 12;
	}

	String result = String(hours) + ":";
	
	if (minutes < 10) {
		result = result + String("0");
	}

	result = result + String(minutes);
	
	if (includeSeconds) {
		result = result + String(":");
		if (seconds < 10) {
			result = result + String("0");
		}
		result = result + String(seconds);
	}

	return result;
}

String suffixFromDayMinutes(double dayMinutes) {
	uint16_t hours = dayMinutes / 60;
	bool pm = hours >= 12 && hours <= 24;

	return String((pm) ? "pm" : "am");
}

const char* directionNames[] = { "N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW" };
const char *directionGlyphs = "KLMFGHIJ";
const char *speedGlyphs = "NOPQRSTUV";
const char *fixNames[] = {"No Fix", "Dead Reckoning", "2D", "3D", "GNSS + Dead reckoning", "Time only" };
char timeGlyph = 'A';
char altitudeGlyph = 'B';
char speedGlyph = 'C';
char sunriseGlyph = 'D';
char sunsetGlyph = 'E';
char satelliteGlyph = 'Z';
char emptySpeedlyph = 'W';
char emptyTimeGlyph = 'X';
char emptyDirectionGlyph = 'Y';

const GFXfont *textFont = &ModFreeSans44pt7b;
const GFXfont *suffixFont = &FreeSans16pt7b;
const GFXfont *glyphFont = &iconFont;

void offsetCursor(Adafruit_GFX& dest, int16_t xOffset, int16_t yOffset) {
	dest.setCursor(dest.getCursorX() + xOffset, dest.getCursorY() + yOffset);
}

void showCell(Adafruit_GFX& dest, int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, String suffix, const GFXfont* strFont = textFont, const GFXfont* sufFont = suffixFont) {
	uint16_t ascender = ascenderForFont(strFont, glyph);
	uint16_t xOffset = -2;

	if (str.startsWith(String("1"))) {
		xOffset += oneOffset;
	}

	dest.setFont(glyphFont);
	dest.setCursor(x, y + ascender + 6);
	dest.print(glyph);
	dest.setFont(strFont);
	offsetCursor(dest, xOffset + 4, -5);
	dest.print(str);
	if (suffix) {
		dest.setFont(sufFont);
		dest.print(suffix);
	}
}

#define swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void writePixel(Adafruit_GFX& dest, int16_t x, int16_t y, uint16_t color, uint16_t thickness) {
	if (thickness >= 5) {
		dest.writeFastHLine(x - 1, y - 2, 3, color);
		dest.writeFastHLine(x - 2, y - 1, 5, color);
		dest.writeFastHLine(x - 2, y, 5, color);
		dest.writeFastHLine(x - 2, y + 1, 5, color);
		dest.writeFastHLine(x - 1, y + 2, 3, color);
	}
	else if (thickness >= 3) {
		dest.writePixel(x, y - 1, color);
		dest.writeFastHLine(x - 1, y, 3, color);
		dest.writePixel(x, y + 1, color);
	}
	else {
		dest.writePixel(x, y, color);
	}
}

void drawThickLine(Adafruit_GFX& dest, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap_int16_t(x0, y0);
		swap_int16_t(x1, y1);
	}

	if (x0 > x1) {
		swap_int16_t(x0, x1);
		swap_int16_t(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			writePixel(dest, y0, x0, color, thickness);
		}
		else {
			writePixel(dest, x0, y0, color, thickness);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

#ifndef M_PI
		#define M_PI 3.14159265358979323846
#endif

void centerRotLenToPoint(int16_t centerX, int16_t centerY, float angle, float length, int16_t* outX, int16_t* outY) {
	float radAngle = angle * (2.0 * M_PI) / 360.0;

	*outX = round(centerX + sin(radAngle) * length);
	*outY = round(centerY - cos(radAngle) * length);
}

uint16_t hoursLen = 9;
uint16_t minutesLen = 16;

void drawPolarLine(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t length, int16_t thickness) {
	int16_t destX, destY;

	centerRotLenToPoint(x, y, angle, length, &destX, &destY);
	
	if (thickness > 1) {
		drawThickLine(dest, x, y, destX, destY, thickness, WHITE8);
	}
	else {
		dest.drawLine(x, y, destX, destY, WHITE8);
	}
}

void drawPointer(Adafruit_GFX& dest, int16_t x, int16_t y, float angle, uint16_t majorLen, uint16_t minorLen, uint16_t cornerAngle) {
	int16_t x1, x2, x3, y1, y2, y3;

	centerRotLenToPoint(x, y, angle, majorLen, &x1, &y1);
	centerRotLenToPoint(x, y, angle - cornerAngle, minorLen, &x2, &y2);
	centerRotLenToPoint(x, y, angle + cornerAngle, minorLen, &x3, &y3);
	dest.fillTriangle(x1, y1, x2, y2, x3, y3, WHITE8);
}

void drawTime(Adafruit_GFX& dest, uint16_t x, uint16_t y, uint16_t hours, uint16_t minutes) {
	drawPolarLine(dest, x, y, hours * 360.0 / 12.0, hoursLen, 5);
	drawPolarLine(dest, x, y, minutes * 360.0 / 60.0, minutesLen, 5);
}

int8_t indexOfSensor(uint32_t sensor_id) {
	for (uint8_t i=0; i<NUM_TIRES; i++) {
		if (sensor_id == pref_data.sensor_ids[i]) {
			return i;
		}
	}
	return -1;
}

bool showData(uint16_t* drawIndex, uint32_t time, int16_t altitude, float heading, float speed, uint32_t sunriseTime, uint32_t sunsetTime, uint16_t satCount, bool haveFix, String status) {
	constexpr int16_t xOffset = 50;
	constexpr int16_t xGap = 45;
	constexpr int16_t yStart = 20;
	constexpr int16_t yGap = 22;
	static bool colon = true;
	static bool layer = false;

	layer = !layer;
	display.setDrawLayer(layer);
	display.fillScreen(BLACK8);

	buffer.setTextSize(1);
	buffer.setTextColor(WHITE8);
	buffer.setTextWrap(false);

	if (haveFix) {
		String timeString = timeFromDayMinutes(time, false);
		if (!colon) {
			timeString.replace(String(":"), String(" "));
		}
		colon = !colon;

		buffer.setOffset(xOffset, yStart);
		showCell(buffer, xOffset, yStart, emptyTimeGlyph, timeString, -2, suffixFromDayMinutes(time));
		drawTime(buffer, xOffset+32, yStart+36, time / 60, time % 60);
		buffer.draw(display);

		buffer.setOffset(xOffset+cellWidth+xGap, yStart);
		// buffer.fillRect(xOffset+cellWidth+xGap, yStart, cellWidth, cellHeight, GREEN8);
		showCell(buffer, xOffset+cellWidth+xGap, yStart, altitudeGlyph, String(altitude), -2, String("ft"));
		buffer.draw(display);

		int16_t direction = ((int16_t)((heading+11.25) / 22.5)) % 16; 

		buffer.setOffset(xOffset, yStart+cellHeight+yGap);
		showCell(buffer, xOffset, yStart+cellHeight+yGap, emptyDirectionGlyph, String(directionNames[direction]), 0, String(""));
		drawPointer(buffer, xOffset+32, yStart+cellHeight+yGap+36, heading, 15, 8, 140);
		buffer.draw(display);

		buffer.setOffset(xOffset+cellWidth+xGap, yStart+cellHeight+yGap);
		showCell(buffer, xOffset+cellWidth+xGap, yStart+cellHeight+yGap, emptySpeedlyph, String(speed, 1), 0, String("mph"));
		drawPolarLine(buffer, xOffset+cellWidth+xGap+32, yStart+cellHeight+yGap+44, -100.0+(speed / 75.0) * 200.0, 14, 5);
		buffer.draw(display);

		buffer.setOffset(xOffset, yStart+(cellHeight+yGap)*2);
		showCell(buffer, xOffset, yStart+(cellHeight+yGap)*2, sunriseGlyph, timeFromDayMinutes(sunriseTime, false), -9, suffixFromDayMinutes(sunriseTime));
		buffer.draw(display);

		buffer.setOffset(xOffset+cellWidth+xGap, yStart+(cellHeight+yGap)*2);
		showCell(buffer, xOffset+cellWidth+xGap, yStart+(cellHeight+yGap)*2, sunsetGlyph, timeFromDayMinutes(sunsetTime, false), -9, suffixFromDayMinutes(sunsetTime));
		buffer.draw(display);

		// buffer.setOffset(display_width - 130, display_height - 36);
		// buffer.setFont(&DejaVuSerifBoldItalic30);
		// buffer.setCursor(display_width - 10 - getStringWidth(buffer, status), display_height - 10);
		// buffer.print(status);
		// buffer.draw(display, 130, 26);
	}
	else {
		buffer.setOffset(display_width/2 - 100, 100);
		static uint8_t dotCount = 1;
		static String acquiring = String("Acquiring");
		static String dots = String(".....");

		status = acquiring+dots.substring(0, dotCount);
		dotCount = (dotCount % 5)+1;

		buffer.setFont(&FreeSans18pt7b);

		buffer.setCursor(display_width/2 - 100, 130);
		buffer.print(status);
		buffer.draw(display);
	}

	// display.drawFastHLine(0, 300, display_width, DARK_GRAY8);

	constexpr uint16_t tire_top_y = 300;

	display.fillRect(380, tire_top_y+30, 40, 6, WHITE16);
	display.fillRect(262, tire_top_y+110, 300, 6, WHITE16);
	display.fillRect(396, tire_top_y+32, 6, 80, WHITE16);

	uint16_t tireX[] = { 272, 416, 152, 272, 416, 536 };
	uint16_t tireY[] = { tire_top_y, tire_top_y, tire_top_y+80, tire_top_y+80, tire_top_y+80, tire_top_y+80 };
	uint16_t tirePressure[] = { 87, 87, 90, 102, 92, 90 };
	uint8_t tireColor[] = { GREEN8, GREEN8, GREEN8, GREEN8, GREEN8, GREEN8 };
	constexpr uint16_t tireWidth = 110;
	constexpr uint16_t tireHeight = 67;

	buffer.setFont(&ModFreeSansBold30pt7b);
	for (uint16_t i=0; i<6; i++) {
		buffer.setOffset(tireX[i], tireY[i]);
		drawPNG(Tire_png, sizeof(Tire_png), &buffer, tireX[i], tireY[i]);
		buffer.setTextColor(tireColor[i]);
		if (tirePressure[i]>99) {
			buffer.setCursor(tireX[i]+ 10, tireY[i]+tireHeight-13);
		}
		else {
			buffer.setCursor(tireX[i]+ 25, tireY[i]+tireHeight-13);
		}
		buffer.print(tirePressure[i]);

		buffer.draw(display, tireWidth, tireHeight);
	}

	// buffer.setTextColor(WHITE8);
	// buffer.setOffset(4, display_height - 40);
	// showCell(buffer, 4, display_height - 26, satelliteGlyph, String(satCount), 0, String(""), &FreeSans12pt7b);
	// buffer.draw(display, 60, 40);

	display.showLayer(layer);

	return true;
}

#define DEGREES_TO_FLOAT (10000000)
#define HEADING_TO_FLOAT (100000)
#define SPEED_TO_FLOAT_MPH (447.04)
#define DISTANCE_TO_FLOAT_FLOAT (304.8)

void loop() {
	static elapsedMillis statusTime;
	static bool haveHadFix = false;

	if (true || statusTime > 500) {
		statusTime -= 500;
		// statusTime = 0;

		float light = readLight();

		bool haveFix = gps.getGnssFixOk();
		// bool haveFix = false;

		haveHadFix = haveHadFix || haveFix;

		uint16_t year = gps.getYear();
		uint16_t month = gps.getMonth();
		uint16_t day = gps.getDay();
		uint16_t hour = gps.getHour();
		uint16_t minute = gps.getMinute();
		uint16_t second = gps.getSecond();
		float latitude = (float)gps.getLatitude() / DEGREES_TO_FLOAT;
		float longitude = (float)gps.getLongitude() / DEGREES_TO_FLOAT;
		float altitude = gps.getAltitudeMSL() / DISTANCE_TO_FLOAT_FLOAT;
		float heading = (float)gps.getHeading() / HEADING_TO_FLOAT;
		float speed = (float)gps.getGroundSpeed() / SPEED_TO_FLOAT_MPH;
		uint8_t satellites = gps.getSIV();
		uint8_t fixType = gps.getFixType();
		String status = String(fixNames[fixType]);
		// uint16_t year;
		// uint16_t month;
		// uint16_t day;
		// uint16_t hour;
		// uint16_t minute;
		// uint16_t second;
		// float latitude;
		// float longitude;
		// float altitude;
		// float heading;
		// float speed;
		// uint8_t satellites;
		// uint8_t fixType;
		// String status;

#if 1
		if (!haveFix) {
			static elapsedMillis upTime;

			latitude = 37.7775;
			longitude = -122.416389;
			year = 2021;
			month = 4;
			day = 25;
			uint32_t milliTime = upTime;
			uint32_t minutes = milliTime / 1000 / 60;
			hour = 12 + (minutes / 60);
			minute = minutes % 60;
			uint32_t seconds = (milliTime / 1000) % 60;
			static int16_t headingNum = 0;
			heading = headingNum;
			headingNum = (headingNum + 10) % 360;
			speed = seconds * 80.0 / 60.0;
			altitude = 12005;
			haveHadFix = (upTime > 2000);
		}
#endif

		static int16_t lastZoneOffset = -7;
		bool isDST = zoneCalc.dateIsDST(year, month, day, hour, minute, lastZoneOffset);
		float zoneOffset = zoneCalc.zoneOffsetForGPSCoord(latitude, longitude, isDST);
		int8_t zoneHour = zoneOffset;
		int8_t zoneMinute = (zoneOffset * 60) - (zoneHour * 60);

		lastZoneOffset = zoneOffset + (isDST ? 1 : 0);

		DateTime gpsTimeDate = DateTime(year, month, day, hour, minute, second) + TimeSpan(0, zoneHour, zoneMinute, 0);

		sun.setPosition(latitude, longitude, zoneOffset);
		sun.setCurrentDate(gpsTimeDate.year(), gpsTimeDate.month(), gpsTimeDate.day());

		uint32_t sunriseTime = sun.calcSunrise();
		uint32_t sunsetTime = sun.calcSunset();
		String sunUp = timeFromDayMinutes(sunriseTime, false);
		String sunDown = timeFromDayMinutes(sunsetTime, false);

		uint32_t curTime = gpsTimeDate.hour() * 60 + gpsTimeDate.minute();

		Debug_print("GPS Data: Altitude= ");
		Debug_print(altitude);
		Debug_print(", Heading= ");
		Debug_print(heading);
		Debug_print(", sunrise=");
		Debug_print(sunUp);
		Debug_print(", sunset=");
		Debug_print(sunDown);
		Debug_print(", GMTOffset=");
		Debug_print(lastZoneOffset);
		Debug_print(", fix=");
		Debug_print(haveFix);
		Debug_print(", ");
		Debug_print(satellites);
		Debug_print(" sats");
		Debug_print(", lat=");
		Debug_print(latitude);
		Debug_print(", lon=");
		Debug_print(longitude);
		Debug_print(", spd=");
		Debug_print(speed);
		Debug_print(", light=");
		Debug_print(light);
		Debug_println("");

		uint16_t drawIndex = 0;
		uint32_t startTime = millis();

		while (!showData(&drawIndex, curTime, altitude, heading, speed, sunriseTime, sunsetTime, satellites, haveHadFix, status)) {
			TPMS_Packet packet;

			if (readPacket(&packet)) {
				Debug_println("Got radio packet.");
				int8_t index = indexOfSensor(packet.id);

				if (index != -1) {
					Debug_println("Packet is valid");
					sensor_packets[index] = packet;
				}
			}
		}

		uint32_t totalTime = millis() - startTime;
		Debug_print("Display time: ");
		Debug_print(totalTime);
		Debug_println("ms");
	}

	tsPoint_t touchPt;
	if (pref_data.touch_calibrated && checkScreenTouch(&touchPt, &pref_data.touch_calibration)) {
		if (touchPt.y < 300) {
			menu.run();
		}
	}

	// static elapsedMillis showTime;
	// if (showTime > 300) {
	//   showTime = 0;
	//   Debug_print("Interval = ");
	//   Debug_println(elapsed);
	// }
}