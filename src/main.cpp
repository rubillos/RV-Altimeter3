#include <Arduino.h>

#if defined(DO_SERIAL)
#define Debug_print(...) SEGGER_RTT_printf(0, __VA_ARGS__)
#define Debug_println(...) SEGGER_RTT_printf(__VA_ARGS__) ; SEGGER_RTT_printf("\n")
#define Debug_begin(...) SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL)
#define Debug_delay(...) delay(__VA_ARGS__)
##elif defined(RTT)
#define Debug_print(...) Serial.print(__VA_ARGS__)
#define Debug_println(...) Serial.println(__VA_ARGS__)
#define Debug_begin(...) Serial.begin(__VA_ARGS__)
#define Debug_delay(...) delay(__VA_ARGS__)
#else
#define Debug_print(...)
#define Debug_println(...)
#define Debug_begin(...)
#define Debug_delay(...)
#endif

#define defaultMaxWait 250

#include "SEGGER_RTT.h"
#include <elapsedMillis.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_SharpMem.h>
#include <Adafruit_DotStar.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <GPSTimeZoneLookup.h>
#include <sunset.h>
#include <Timezone.h>

#include "fonts/Rancho40.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/LatoBlack64.h"
#include "fonts/IconFont.h"

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

void startDisplay() {
  display.begin();
  display.clearDisplay();

  const GFXfont *f = &Lato_Black_64;
  String str1 = String("Digital");
  String str2 = String("Altimeter");
  uint16_t w, h;

  display.setFont(f);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextWrap(false);

  getStringDimensions(str1, &w, &h);
  display.setCursor((display.width()-w)/2, (display.height()-h*2-10)/2 + ascenderForFont(f));
  display.print(str1);

  getStringDimensions(str2, &w, &h);
  display.setCursor((display.width() - w) / 2, (display.height() - h*2-10) / 2 + ascenderForFont(f) + h + 10);
  display.print(str2);

  display.refresh();
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

  Debug_println("Begin Startup");

  Debug_println("Init DotStar...");
  dotStar.begin();
  dotStar.setBrightness(128);
  dotStar.show();

  Debug_println("Init display...");
  startDisplay();

  Debug_println("Init GPS...");
  Wire.begin();

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Debug_println("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing.");
    while (1)
      ;
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  if (!myGNSS.setDynamicModel(DYN_MODEL_AUTOMOTIVE)) {
    Debug_println("***!!! Warning: setDynamicModel failed !!!***");
  }

  delay(300);

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
  bool pm = hours >= 13 && hours <= 24;

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
  
  result = result + String((pm) ? "pm":"am");

  return result;
}

TimeChangeRule usPDT = {"PDT", Second, Sun, Mar, 2, -420};
TimeChangeRule usPST = {"PST", First, Sun, Nov, 2, -480};
Timezone myTZ(usPDT, usPST);

bool dateIsDST(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute)
{
  TimeElements timeParts = { 0, minute, hour, 0, day, month, (uint8_t)(year - 1970) };
  time_t utcTime = makeTime(timeParts);

  return myTZ.utcIsDST(utcTime);
}

const char* directionNames[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
const char *directionGlyphs = "KLMFGHIJ";
const char *speedGlyphs = "NOPQRSTUV";
const char* qualityNames[] = { "No Fix", "No Fix", "2D", "3D" };
char timeGlyph = 'A';
char altitudeGlyph = 'B';
char speedGlyph = 'C';
char sunriseGlyph = 'D';
char sunsetGlyph = 'E';
char satelliteGlyph = 'W';

const GFXfont *textFont = &FreeSans18pt7b;
const GFXfont *glyphFont = &iconFont;

#define KNOTS_TO_MILES 1.151
#define METERS_TO_FEET 3.281

void showCell(int16_t x, int16_t y, char glyph, String str, const GFXfont* strFont = textFont)
{
  uint16_t glyphAscender = ascenderForFont(glyphFont, glyph);
  uint16_t textAscender = ascenderForFont(strFont);

  display.setFont(glyphFont);
  display.setCursor(x, y + glyphAscender);
  display.print(glyph);
  display.setFont(strFont);
  display.setCursor(display.getCursorX() - 2, y + textAscender + (glyphAscender - textAscender) / 2);
  display.print(str);
}

void showData(uint32_t time, int16_t altitude, uint8_t heading, float speed, String sunrise, String sunset, uint16_t satCount, bool haveFix) {
  int16_t xOffset = 0;
  int16_t cellWidth = 190;
  int16_t yStart = 5;
  int16_t cellHeight = 60;

  display.fillScreen(WHITE);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextWrap(false);

  if (haveFix) {
    showCell(xOffset, yStart, timeGlyph, timeFromDayMinutes(time, false));

    showCell(xOffset + cellWidth, yStart, altitudeGlyph, String(altitude) + String("ft"));

    showCell(xOffset, yStart + cellHeight, directionGlyphs[heading], String(directionNames[heading]));
    showCell(xOffset + cellWidth, yStart + cellHeight, speedGlyphs[(uint8_t)((speed+5.0)/10.0)], String(speed, 1) + String("mph"));

    showCell(xOffset, yStart + cellHeight * 2, sunriseGlyph, sunrise);
    showCell(xOffset + cellWidth, yStart + cellHeight * 2, sunsetGlyph, sunset);
  }
  else {
    showCell(xOffset, yStart, timeGlyph, String("--:--"));
    showCell(xOffset + cellWidth, yStart, altitudeGlyph, String("---"));

    showCell(xOffset, yStart + cellHeight, directionGlyphs[0], String("--"));
    showCell(xOffset + cellWidth, yStart + cellHeight, speedGlyph, String("---"));

    showCell(xOffset, yStart + cellHeight * 2, sunriseGlyph, String("--:--"));
    showCell(xOffset + cellWidth, yStart + cellHeight * 2, sunsetGlyph, String("--:--"));

    display.setFont(&FreeSans18pt7b);
    display.setCursor(200, 230);
    display.print("Acquiring");

    static uint8_t dotCount = 1;

    for (uint8_t i=0; i<dotCount; i++) {
      display.print('.');
    }
    dotCount = (dotCount % 6) + 1;
  }

  showCell(xOffset, 200, satelliteGlyph, String(satCount), &FreeSans12pt7b);

  display.refresh();
}

#define DEGREES_TO_FLOAT (1.0 / 10e-7)
#define HEADING_TO_FLOAT (1.0 / 10e-5)
#define SPEED_TO_FLOAT (6.2137e-7)

void loop() {
  static elapsedMillis statusTime;
  static elapsedMillis upTime;
  static bool haveHadFix = false;
  static uint8_t direction = 0;

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
    float latitude = (float)myGNSS.getLatitude() * DEGREES_TO_FLOAT;
    float longitude = (float)myGNSS.getLongitude() * DEGREES_TO_FLOAT;
    float altitude = myGNSS.getAltitudeMSL();
    int16_t heading = (float)myGNSS.getHeading() * HEADING_TO_FLOAT;
    float speed = (float)myGNSS.getGroundSpeed() * SPEED_TO_FLOAT;
    uint8_t satellites = myGNSS.getSIV();

#if 0
    if (!haveFix) {
      latitude = 37.7775;
      longitude = -122.416389;
      year = 2021;
      month = 4;
      day = 25;
      uint32_t milliTime = upTime;
      uint32_t minutes = milliTime / 1000 / 60;
      hour = 12 + (minutes / 60);
      minute = minutes % 60;
      seconds = (milliTime / 1000) % 60;
      static uint8_t headingNum = 0;
      heading = headingNum * 45 + 22;
      headingNum = (headingNum + 1) % 8;
      speed = seconds * 80.0 / 60.0;
      altitude = 12005 / METERS_TO_FEET;
      haveHadFix = (upTime > 2000);
    }
#endif

    GPSTimeZoneLookup tz1(latitude, longitude);
    int16_t offset = tz1.GMTOffset;
  
    if (tz1.implementsDST && dateIsDST(year, month, day, hour, minute)) {
      offset++;
    }

    sun.setPosition(latitude, longitude, offset);
    sun.setCurrentDate(year, month, day);

    String sunUp = timeFromDayMinutes(sun.calcSunrise(), false);
    String sunDown = timeFromDayMinutes(sun.calcSunset(), false);

    int16_t adjustedHours = hour + offset;

    if (adjustedHours < 0) {
      adjustedHours += 24;
    }
    else if (adjustedHours > 23) {
      adjustedHours -= 24;
    }

    uint32_t curTime = adjustedHours * 60 + minute;

    if (speed > 3) {
      float tweakedHeading = heading + 22.5;

      if (tweakedHeading < 0)
      {
        tweakedHeading += 360;
      }
      else if (tweakedHeading >= 360)
      {
        tweakedHeading -= 360;
      }

      direction = tweakedHeading / 45.0;
    }

    Debug_print("GPS Data: Altitude= ");
    Debug_print(altitude);
    Debug_print(", Heading= ");
    Debug_print(heading);
    Debug_print(", sunrise=");
    Debug_print(sunUp);
    Debug_print(", sunset=");
    Debug_print(sunDown);
    Debug_print(", usesDST=");
    Debug_print(tz1.implementsDST);
    Debug_print(", GMTOffset=");
    Debug_print(offset);
    Debug_print(", fix=");
    Debug_print(haveFix);
    Debug_print(", ");
    Debug_print(satellites);
    Debug_print(" satellites");
    Debug_println();

    showData(curTime, altitude, direction, speed, sunUp, sunDown, satellites, haveHadFix);
  }
}