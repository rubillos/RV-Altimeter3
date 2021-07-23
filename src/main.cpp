#include <Arduino.h>

#ifdef DO_SERIAL
#define Debug_print(...) Serial.print(__VA_ARGS__)
#define Debug_println(...) Serial.println(__VA_ARGS__)
#define Debug_begin(...) Serial.begin(__VA_ARGS__)
#define Debug_delay(...) delay(__VA_ARGS__)
#endif
// #ifdef DO_RTT
// #define Debug_print(...) SEGGER_RTT_printf(0, __VA_ARGS__)
// #define Debug_println(...)  SEGGER_RTT_printf(0, __VA_ARGS__); SEGGER_RTT_printf(0, "\n")
// #define Debug_begin(...) SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL)
// #define Debug_delay(...) delay(__VA_ARGS__)
// #endif
#ifndef DO_SERIAL
  // #ifndef DO_RTT
    #define Debug_print(...)
    #define Debug_println(...)
    #define Debug_begin(...)
    #define Debug_delay(...)
  // #endif
#endif

#define defaultMaxWait 250

// #include "SEGGER_RTT.h"
#include <elapsedMillis.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_SharpMem.h>
#include <Adafruit_DotStar.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <GPSTimeZoneLookup.h>
#include <sunset.h>
#include <Timezone.h>
#include <ZoneCalc.h>

#include "fonts/FreeSans24pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/IconFont.h"
#include "fonts/DejaVuSerifItalic12pt.h"
#include "fonts/DejaVuSerifBoldItalic24pt.h"
#include "fonts/DejaVuSerifBoldItalic30pt.h"
#include "fonts/DejaVuSerifBoldItalic56pt.h"
#include "fonts/ModFreeSans24pt.h"

#define SHARP_SCK 10
#define SHARP_DI 9
#define SHARP_SS 7
#define DOTSTAR_DATAPIN 8
#define DOTSTAR_CLOCKPIN 6

#define BLACK 0
#define WHITE 1

Adafruit_SharpMem display(SHARP_SCK, SHARP_DI, SHARP_SS, 400, 240);
Adafruit_DotStar dotStar(1, DOTSTAR_DATAPIN, DOTSTAR_CLOCKPIN, DOTSTAR_BRG);
SFE_UBLOX_GNSS myGNSS;
SunSet sun;

uint8_t ascenderForFont(const GFXfont *f, char character = 'A')
{
  uint16_t index = character - f->first;
  int8_t offset = f->glyph[index].yOffset;

  return -offset;
}

void getStringDimensions(String str, uint16_t* width, uint16_t* height) {
  int16_t x, y;

  display.getTextBounds(str, 0, 0, &x, &y, width, height);
}

uint16_t getStringWidth(String str)
{
  int16_t x, y;
  uint16_t width, height;

  display.getTextBounds(str, 0, 0, &x, &y, &width, &height);
  return width;
}

void startDisplay() {
  display.begin();
  display.clearDisplay();

  const GFXfont *f1 = &DejaVuSerifBoldItalic56;
  const GFXfont *f2 = &DejaVuSerifBoldItalic24;
  String str1 = String("GPS");
  String str2 = String("Altometer");
  String str3 = String("by Randy Ubillos");
  uint16_t w, h;

  display.setFont(f1);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextWrap(false);

  getStringDimensions(str1, &w, &h);
  display.setCursor((display.width()-w)/2, 80);
  display.print(str1);

  getStringDimensions(str2, &w, &h);
  display.setCursor((display.width() - w) / 2, display.getCursorY() + 56);
  display.print(str2);

  display.setFont(f2);
  getStringDimensions(str3, &w, &h);
  display.setCursor((display.width() - w) / 2, display.getCursorY() + 80);
  display.print(str3);

  display.refresh();
}

typedef struct {
  const char* name;
  float lat;
  float lon;
} CityRec;

void test_zones() {
#if defined(DO_SERIAL)
  Debug_println("Zone Test");

  CityRec cities[] = {
    { "Boston", 42.360081, -71.058884 },
    { "Chicago", 41.878113, -87.629799 },
    { "Tampa", 27.950575, -82.457176 },
    { "Denver", 39.739235, -104.990250 },
    { "Houston", 29.760427, -95.369804 },
    { "Alaska", 63.391522, -155.537651 },
    { "San Francisco", 37.744657, -122.438970 },
    { "Trout Creek", 47.836042, -115.593490 },
    { "Heron MT", 48.093633, -116.033489 },
    { "Claire Fork ID", 48.090157, -116.057808 },
  };

  int cityCount = sizeof(cities) / sizeof(CityRec);

  for (int i=0; i<cityCount; i++) {
    int sumZone = zoneOffsetForGPSCoord(cities[i].lat, cities[i].lon, true);
    int wintZone = zoneOffsetForGPSCoord(cities[i].lat, cities[i].lon, false);
    Debug_print(cities[i].name);
    Debug_print(": summer=");
    Debug_print(sumZone);
    Debug_print(": winter=");
    Debug_print(wintZone);
    Debug_println();
  }

  while (1);
#endif
}

void setup()
{
  Debug_begin(115200);

#if defined(DO_SERIAL)
  while (!Serial)
    yield();
  Debug_println();
#endif

  // settling time
  delay(250);

  // test_zones();

  Debug_println("Begin Startup");

  Debug_println("Init DotStar...");
  dotStar.begin();
  dotStar.setBrightness(128);
  dotStar.show();

  Debug_println("Init display...");
  startDisplay();

  Debug_println("Init GPS...");
  Wire.begin();

  if (!myGNSS.begin()) //Connect to the u-blox module using Wire port
  {
    Debug_println("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing.");
    while (1)
      ;
  }

  // Wire.setClock(400000);                             //Increase I2C clock speed to 400kHz

  myGNSS.setI2COutput(COM_TYPE_UBX);                 //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  if (!myGNSS.setDynamicModel(DYN_MODEL_AUTOMOTIVE))
  {
    Debug_println("***!!! Warning: setDynamicModel failed !!!***");
  }

  // myGNSS.enableDebugging();

  delay(300);

  
  Debug_print("GNSS is ");
  Debug_println(myGNSS.isConnected() ? "CONNECTED" : "not connected");
  Debug_print("GPS is ");
  Debug_println(myGNSS.isGNSSenabled(SFE_UBLOX_GNSS_ID_GPS) ? "ENABLED" : "disabled");
  Debug_print("GLONASS is ");
  Debug_println(myGNSS.isGNSSenabled(SFE_UBLOX_GNSS_ID_GLONASS) ? "ENABLED" : "disabled");
  Debug_print("GALILEO is ");
  Debug_println(myGNSS.isGNSSenabled(SFE_UBLOX_GNSS_ID_GALILEO) ? "ENABLED" : "disabled");
  Debug_print("SBAS is ");
  Debug_println(myGNSS.isGNSSenabled(SFE_UBLOX_GNSS_ID_SBAS) ? "ENABLED" : "disabled");

  Debug_println("Init Done");
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

TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone myTZ(usPDT, usPST);

bool dateIsDST(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute) {
  TimeElements timeParts = { 0, minute, hour, 0, day, month, (uint8_t)(year - 1970) };
  time_t utcTime = makeTime(timeParts);

  return myTZ.utcIsDST(utcTime);
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

String title = String("GPS Altometer");
String tm = String("tm");

const GFXfont *textFont = &MySans24pt7b;
const GFXfont *suffixFont = &FreeSans9pt7b;
const GFXfont *glyphFont = &iconFont;

void offsetCursor(int16_t xOffset, int16_t yOffset) {
  display.setCursor(display.getCursorX() + xOffset, display.getCursorY() + yOffset);
}

void showCell(int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, const GFXfont* strFont, String suffix, const GFXfont* sufFont) {
  uint16_t glyphAscender = ascenderForFont(glyphFont, glyph);
  uint16_t xOffset = -2;

  if (str.startsWith(String("1"))) {
    xOffset += oneOffset;
  }

  display.setFont(glyphFont);
  display.setCursor(x, y + glyphAscender);
  display.print(glyph);
  display.setFont((strFont != NULL) ? strFont : textFont);
  offsetCursor(xOffset, -15);
  display.print(str);
  if (suffix) {
    display.setFont((sufFont != NULL) ? sufFont : suffixFont);
    display.print(suffix);
  }
}

#define swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }

void writePixel(int16_t x, int16_t y, uint16_t color, uint16_t thickness) {
  if (thickness >= 5) {
    display.writeFastHLine(x - 1, y - 2, 3, color);
    display.writeFastHLine(x - 2, y - 1, 5, color);
    display.writeFastHLine(x - 2, y, 5, color);
    display.writeFastHLine(x - 2, y + 1, 5, color);
    display.writeFastHLine(x - 1, y + 2, 3, color);
  }
  else if (thickness >= 3) {
    display.writePixel(x, y - 1, color);
    display.writeFastHLine(x - 1, y, 3, color);
    display.writePixel(x, y + 1, color);
  }
  else {
    display.writePixel(x, y, color);
  }
}

void drawThickLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t thickness, uint16_t color)
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
      writePixel(y0, x0, color, thickness);
    }
    else {
      writePixel(x0, y0, color, thickness);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void centerRotLenToPoint(int16_t centerX, int16_t centerY, float angle, float length, int16_t* outX, int16_t* outY) {
  float radAngle = angle * (2.0 * M_PI) / 360.0;

  *outX = round(centerX + sin(radAngle) * length);
  *outY = round(centerY - cos(radAngle) * length);
}

uint16_t hoursLen = 9;
uint16_t minutesLen = 16;

void drawPolarLine(int16_t x, int16_t y, float angle, uint16_t length, int16_t thickness) {
  int16_t destX, destY;

  centerRotLenToPoint(x, y, angle, length, &destX, &destY);
  
  if (thickness > 1) {
    drawThickLine(x, y, destX, destY, thickness, BLACK);
  }
  else {
    display.drawLine(x, y, destX, destY, BLACK);
  }
}

void drawPointer(int16_t x, int16_t y, float angle, uint16_t majorLen, uint16_t minorLen, uint16_t cornerAngle) {
  int16_t x1, x2, x3, y1, y2, y3;

  centerRotLenToPoint(x, y, angle, majorLen, &x1, &y1);
  centerRotLenToPoint(x, y, angle - cornerAngle, minorLen, &x2, &y2);
  centerRotLenToPoint(x, y, angle + cornerAngle, minorLen, &x3, &y3);
  display.fillTriangle(x1, y1, x2, y2, x3, y3, BLACK);
}

void drawTime(uint16_t x, uint16_t y, uint16_t hours, uint16_t minutes) {
  drawPolarLine(x, y, hours * 360.0 / 12.0, hoursLen, 5);
  drawPolarLine(x, y, minutes * 360.0 / 60.0, minutesLen, 5);
}

void showData(uint32_t time, int16_t altitude, float heading, float speed, uint32_t sunriseTime, uint32_t sunsetTime, uint16_t satCount, bool haveFix, String status) {
  int16_t xOffset = 0;
  int16_t cellWidth = 200;
  int16_t yStart = 5;
  int16_t cellHeight = 60;
  static bool colon = true;

  display.fillScreen(WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextWrap(false);

  if (haveFix) {
    String timeString = timeFromDayMinutes(time, false);
    if (!colon) {
      timeString.replace(String(":"), String(" "));
    }
    colon = !colon;
    showCell(xOffset, yStart, emptyTimeGlyph, timeString, -2, NULL, suffixFromDayMinutes(time), NULL);
    drawTime(xOffset + 32, yStart + 32, time / 60, time % 60);

    showCell(xOffset + cellWidth, yStart, altitudeGlyph, String(altitude), -2, NULL, String("ft"), NULL);

    int16_t direction = ((int16_t)((heading + 11.25) / 22.5)) % 16; 

    showCell(xOffset, yStart + cellHeight, emptyDirectionGlyph, String(directionNames[direction]), 0, NULL, String(""), NULL);
    drawPointer(xOffset + 32, yStart + cellHeight + 32, heading, 15, 8, 140);

    showCell(xOffset + cellWidth, yStart + cellHeight, emptySpeedlyph, String(speed, 1), 0, NULL, String("mph"), NULL);
    drawPolarLine(xOffset + cellWidth + 32, yStart + cellHeight + 40, -100.0 + (speed / 75.0) * 200.0, 14, 5);

    showCell(xOffset, yStart + cellHeight * 2, sunriseGlyph, timeFromDayMinutes(sunriseTime, false), -9, NULL, suffixFromDayMinutes(sunriseTime), NULL);
    showCell(xOffset + cellWidth, yStart + cellHeight * 2, sunsetGlyph, timeFromDayMinutes(sunsetTime, false), -9, NULL, suffixFromDayMinutes(sunsetTime), NULL);

    display.setFont(&DejaVuSerifBoldItalic30);
    if (status == String("3D")) {
      int16_t titleWidth = getStringWidth(title);
      display.setFont(&DejaVuSerifItalic12);
      int16_t tmWidth = getStringWidth(tm);
      display.setFont(&DejaVuSerifBoldItalic30);
      display.setCursor(390 - (titleWidth + tmWidth + 3), 230);
      display.print(title);
      display.setFont(&DejaVuSerifItalic12);
      offsetCursor(3, -12);
      display.print(tm);
    }
    else {
      display.setCursor(390 - getStringWidth(status), 230);
      display.print(status);
    }
  }
  else {
    showCell(xOffset, yStart, timeGlyph, String("--:--"), 0, NULL, String(""), NULL);
    showCell(xOffset + cellWidth, yStart, altitudeGlyph, String("---"), 0, NULL, String(""), NULL);

    showCell(xOffset, yStart + cellHeight, emptyDirectionGlyph, String("--"), 0, NULL, String(""), NULL);
    showCell(xOffset + cellWidth, yStart + cellHeight, speedGlyph, String("---"), 0, NULL, String(""), NULL);

    showCell(xOffset, yStart + cellHeight * 2, sunriseGlyph, String("--:--"), 0, NULL, String(""), NULL);
    showCell(xOffset + cellWidth, yStart + cellHeight * 2, sunsetGlyph, String("--:--"), 0, NULL, String(""), NULL);

    static uint8_t dotCount = 1;
    static String acquiring = String("Acquiring");
    static String dots = String("......");

    status = acquiring + dots.substring(0, dotCount);
    dotCount = (dotCount % 6) + 1;

    display.setFont(&FreeSans18pt7b);

    display.setCursor(200, 230);
    display.print(status);
  }

  showCell(xOffset + 4, 200, satelliteGlyph, String(satCount), 0, &FreeSans12pt7b, String(""), NULL);

  display.refresh();
}

#define DEGREES_TO_FLOAT (10000000)
#define HEADING_TO_FLOAT (100000)
#define SPEED_TO_FLOAT_MPH (447.04)
#define DISTANCE_TO_FLOAT_FLOAT (304.8)

void loop() {
  static elapsedMillis statusTime;
  static bool haveHadFix = false;

  if (statusTime > 1000) {
    statusTime -= 1000;

    bool haveFix = myGNSS.getGnssFixOk();

    haveHadFix = haveHadFix || haveFix;

    uint16_t year = myGNSS.getYear();
    uint16_t month = myGNSS.getMonth();
    uint16_t day = myGNSS.getDay();
    uint16_t hour = myGNSS.getHour();
    uint16_t minute = myGNSS.getMinute();
    // uint16_t second = myGNSS.getSecond();
    float latitude = (float)myGNSS.getLatitude() / DEGREES_TO_FLOAT;
    float longitude = (float)myGNSS.getLongitude() / DEGREES_TO_FLOAT;
    float altitude = myGNSS.getAltitudeMSL() / DISTANCE_TO_FLOAT_FLOAT;
    float heading = (float)myGNSS.getHeading() / HEADING_TO_FLOAT;
    float speed = (float)myGNSS.getGroundSpeed() / SPEED_TO_FLOAT_MPH;
    uint8_t satellites = myGNSS.getSIV();
    uint8_t fixType = myGNSS.getFixType();
    String status = String(fixNames[fixType]);

#if 0
    if (1 || !haveFix) {
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

    int16_t offset = zoneOffsetForGPSCoord(latitude, longitude, dateIsDST(year, month, day, hour, minute));

    sun.setPosition(latitude, longitude, offset);
    sun.setCurrentDate(year, month, day);

    uint32_t sunriseTime = sun.calcSunrise();
    uint32_t sunsetTime = sun.calcSunset();
    String sunUp = timeFromDayMinutes(sunriseTime, false);
    String sunDown = timeFromDayMinutes(sunsetTime, false);

    int16_t adjustedHours = hour + offset;

    if (adjustedHours < 0) {
      adjustedHours += 24;
    }
    else if (adjustedHours > 23) {
      adjustedHours -= 24;
    }

    uint32_t curTime = adjustedHours * 60 + minute;

    Debug_print("GPS Data: Altitude= ");
    Debug_print(altitude);
    Debug_print(", Heading= ");
    Debug_print(heading);
    Debug_print(", sunrise=");
    Debug_print(sunUp);
    Debug_print(", sunset=");
    Debug_print(sunDown);
    Debug_print(", GMTOffset=");
    Debug_print(offset);
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
    Debug_println();

    showData(curTime, altitude, heading, speed, sunriseTime, sunsetTime, satellites, haveHadFix, status);
  }
}