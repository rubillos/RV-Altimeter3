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

#include "fonts/Rancho40.h"
#include "fonts/FreeSans24pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans9pt7b.h"
#include "fonts/LatoBlack72.h"
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

const GFXglyph MySans24pt7bGlyphs[] PROGMEM = {
    {0, 0, 0, 12, 0, 1},        // 0x20 ' '
    {0, 4, 34, 16, 6, -33},     // 0x21 '!'
    {17, 11, 12, 16, 2, -32},   // 0x22 '"'
    {34, 24, 33, 26, 1, -31},   // 0x23 '#'
    {133, 23, 41, 26, 1, -34},  // 0x24 '$'
    {251, 39, 34, 42, 1, -32},  // 0x25 '%'
    {417, 28, 34, 31, 2, -32},  // 0x26 '&'
    {536, 4, 12, 9, 2, -32},    // 0x27 '''
    {542, 10, 44, 16, 3, -33},  // 0x28 '('
    {597, 10, 44, 16, 2, -33},  // 0x29 ')'
    {652, 14, 14, 18, 2, -33},  // 0x2A '*'
    {677, 23, 22, 27, 2, -21},  // 0x2B '+'
    {741, 4, 12, 13, 4, -4},    // 0x2C ','
    {747, 11, 4, 16, 2, -14},   // 0x2D '-'
    {753, 4, 5, 12, 4, -4},     // 0x2E '.'
    {756, 13, 35, 13, 0, -33},  // 0x2F '/'
    {813, 22, 34, 26, 2, -32},  // 0x30 '0'
    {907, 11, 33, 17, 2, -32},  // 0x31 '1'
    {953, 22, 33, 26, 2, -32},  // 0x32 '2'
    {1044, 23, 34, 26, 1, -32}, // 0x33 '3'
    {1142, 23, 33, 26, 1, -32}, // 0x34 '4'
    {1237, 22, 34, 26, 2, -32}, // 0x35 '5'
    {1331, 22, 34, 26, 2, -32}, // 0x36 '6'
    {1425, 21, 33, 26, 2, -32}, // 0x37 '7'
    {1512, 22, 34, 26, 2, -32}, // 0x38 '8'
    {1606, 21, 34, 26, 2, -32}, // 0x39 '9'
    {1696, 4, 25, 12, 4, -24},  // 0x3A ':'
    {1709, 4, 32, 12, 4, -24},  // 0x3B ';'
    {1725, 23, 23, 27, 2, -22}, // 0x3C '<'
    {1792, 23, 12, 27, 2, -16}, // 0x3D '='
    {1827, 23, 23, 27, 2, -22}, // 0x3E '>'
    {1894, 20, 35, 26, 4, -34}, // 0x3F '?'
    {1982, 43, 42, 48, 2, -34}, // 0x40 '@'
    {2208, 30, 34, 31, 1, -33}, // 0x41 'A'
    {2336, 25, 34, 31, 4, -33}, // 0x42 'B'
    {2443, 29, 36, 33, 2, -34}, // 0x43 'C'
    {2574, 27, 34, 33, 4, -33}, // 0x44 'D'
    {2689, 24, 34, 30, 4, -33}, // 0x45 'E'
    {2791, 22, 34, 28, 4, -33}, // 0x46 'F'
    {2885, 31, 36, 36, 2, -34}, // 0x47 'G'
    {3025, 26, 34, 34, 4, -33}, // 0x48 'H'
    {3136, 4, 34, 13, 5, -33},  // 0x49 'I'
    {3153, 19, 35, 25, 2, -33}, // 0x4A 'J'
    {3237, 27, 34, 32, 4, -33}, // 0x4B 'K'
    {3352, 21, 34, 26, 4, -33}, // 0x4C 'L'
    {3442, 32, 34, 40, 4, -33}, // 0x4D 'M'
    {3578, 26, 34, 34, 4, -33}, // 0x4E 'N'
    {3689, 33, 36, 37, 2, -34}, // 0x4F 'O'
    {3838, 24, 34, 31, 4, -33}, // 0x50 'P'
    {3940, 33, 38, 37, 2, -34}, // 0x51 'Q'
    {4097, 26, 34, 33, 4, -33}, // 0x52 'R'
    {4208, 27, 36, 31, 2, -34}, // 0x53 'S'
    {4330, 26, 34, 30, 2, -33}, // 0x54 'T'
    {4441, 26, 35, 34, 4, -33}, // 0x55 'U'
    {4555, 29, 34, 30, 1, -33}, // 0x56 'V'
    {4679, 42, 34, 44, 1, -33}, // 0x57 'W'
    {4858, 29, 34, 31, 1, -33}, // 0x58 'X'
    {4982, 30, 34, 32, 1, -33}, // 0x59 'Y'
    {5110, 27, 34, 29, 1, -33}, // 0x5A 'Z'
    {5225, 8, 44, 13, 3, -33},  // 0x5B '['
    {5269, 13, 35, 13, 0, -33}, // 0x5C '\'
    {5326, 8, 44, 13, 1, -33},  // 0x5D ']'
    {5370, 18, 18, 22, 2, -32}, // 0x5E '^'
    {5411, 28, 2, 26, -1, 7},   // 0x5F '_'
    {5418, 10, 7, 12, 1, -34},  // 0x60 '`'
    {5427, 24, 27, 26, 1, -25}, // 0x61 'a'
    {5508, 22, 35, 26, 3, -33}, // 0x62 'b'
    {5605, 21, 27, 24, 1, -25}, // 0x63 'c'
    {5676, 23, 35, 26, 1, -33}, // 0x64 'd'
    {5777, 22, 27, 25, 1, -25}, // 0x65 'e'
    {5852, 10, 34, 13, 1, -33}, // 0x66 'f'
    {5895, 22, 36, 26, 1, -25}, // 0x67 'g'
    {5994, 19, 34, 25, 3, -33}, // 0x68 'h'
    {6075, 4, 34, 10, 3, -33},  // 0x69 'i'
    {6092, 8, 44, 11, 0, -33},  // 0x6A 'j'
    {6136, 21, 34, 24, 3, -33}, // 0x6B 'k'
    {6226, 4, 34, 10, 3, -33},  // 0x6C 'l'
    {6243, 32, 26, 38, 3, -25}, // 0x6D 'm'
    {6347, 20, 26, 25, 3, -25}, // 0x6E 'n'
    {6412, 23, 27, 25, 1, -25}, // 0x6F 'o'
    {6490, 22, 35, 26, 3, -25}, // 0x70 'p'
    {6587, 23, 35, 26, 1, -25}, // 0x71 'q'
    {6688, 12, 26, 16, 3, -25}, // 0x72 'r'
    {6727, 20, 27, 23, 1, -25}, // 0x73 's'
    {6795, 10, 32, 13, 1, -30}, // 0x74 't'
    {6835, 20, 26, 25, 3, -24}, // 0x75 'u'
    {6900, 23, 25, 23, 0, -24}, // 0x76 'v'
    {6972, 34, 25, 34, 0, -24}, // 0x77 'w'
    {7079, 22, 25, 22, 0, -24}, // 0x78 'x'
    {7148, 22, 35, 22, 0, -24}, // 0x79 'y'
    {7245, 20, 25, 23, 1, -24}, // 0x7A 'z'
    {7308, 11, 44, 16, 2, -33}, // 0x7B '{'
    {7369, 3, 44, 12, 4, -33},  // 0x7C '|'
    {7386, 11, 44, 16, 2, -33}, // 0x7D '}'
    {7447, 19, 7, 24, 2, -19}}; // 0x7E '~'

const GFXfont MySans24pt7b PROGMEM = {(uint8_t *)FreeSans24pt7bBitmaps,
                                        (GFXglyph *)MySans24pt7bGlyphs, 0x20,
                                        0x7E, 56};

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

  const GFXfont *f = &Lato_Black_72;
  String str1 = String("GPS");
  String str2 = String("Altometer");
  uint16_t w, h;

  display.setFont(f);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setTextWrap(false);

  getStringDimensions(str1, &w, &h);
  display.setCursor((display.width()-w)/2, (display.height()-h*2-20)/2 + ascenderForFont(f) - 10);
  display.print(str1);

  getStringDimensions(str2, &w, &h);
  display.setCursor((display.width() - w) / 2, (display.height() - h*2-20) / 2 + ascenderForFont(f) + h + 10);
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

const GFXfont *textFont = &MySans24pt7b;
const GFXfont *suffixFont = &FreeSans9pt7b;
const GFXfont *glyphFont = &iconFont;

void showCell(int16_t x, int16_t y, char glyph, String str, int16_t oneOffset, const GFXfont* strFont, String suffix, const GFXfont* sufFont) {
  uint16_t glyphAscender = ascenderForFont(glyphFont, glyph);
  // uint16_t textAscender = ascenderForFont(strFont);
  uint16_t xOffset = -2;

  if (str.startsWith(String("1"))) {
    xOffset += oneOffset;
  }

  display.setFont(glyphFont);
  display.setCursor(x, y + glyphAscender);
  display.print(glyph);
  display.setFont((strFont != NULL) ? strFont : textFont);
  display.setCursor(display.getCursorX() + xOffset, display.getCursorY() - 15);
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
    showCell(xOffset, yStart, emptyTimeGlyph, timeString, -4, NULL, suffixFromDayMinutes(time), NULL);
    drawTime(xOffset + 32, yStart + 32, time / 60, time % 60);

    showCell(xOffset + cellWidth, yStart, altitudeGlyph, String(altitude), -4, NULL, String("ft"), NULL);

    int16_t direction = ((int16_t)((heading + 11.25) / 22.5)) % 16; 

    showCell(xOffset, yStart + cellHeight, emptyDirectionGlyph, String(directionNames[direction]), 0, NULL, String(""), NULL);
    // drawPolarLine(xOffset + 32, yStart + cellHeight + 32, heading, 15, 5);
    drawPointer(xOffset + 32, yStart + cellHeight + 32, heading, 15, 8, 140);

    showCell(xOffset + cellWidth, yStart + cellHeight, emptySpeedlyph, String(speed, 1), 0, NULL, String("mph"), NULL);
    drawPolarLine(xOffset + cellWidth + 32, yStart + cellHeight + 40, -100.0 + (speed / 75.0) * 200.0, 14, 5);

    showCell(xOffset, yStart + cellHeight * 2, sunriseGlyph, timeFromDayMinutes(sunriseTime, false), -9, NULL, suffixFromDayMinutes(sunriseTime), NULL);
    showCell(xOffset + cellWidth, yStart + cellHeight * 2, sunsetGlyph, timeFromDayMinutes(sunsetTime, false), -9, NULL, suffixFromDayMinutes(sunsetTime), NULL);

    display.setFont(&FreeSans18pt7b);

    display.setCursor(390 - getStringWidth(status), 230);
    display.print(status);
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

  showCell(xOffset, 200, satelliteGlyph, String(satCount), 0, &FreeSans12pt7b, String(""), NULL);

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

    GPSTimeZoneLookup tz1(latitude, longitude);
    int16_t offset = tz1.GMTOffset;
  
    if (tz1.implementsDST && dateIsDST(year, month, day, hour, minute)) {
      offset++;
    }

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
    Debug_print(", usesDST=");
    Debug_print(tz1.implementsDST);
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
    Debug_print(", alt=");
    Debug_print(altitude);
    Debug_print(", spd=");
    Debug_print(speed);
    Debug_println();

    showData(curTime, altitude, heading, speed, sunriseTime, sunsetTime, satellites, haveHadFix, status);
  }
}