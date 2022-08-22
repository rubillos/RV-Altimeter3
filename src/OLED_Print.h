#include "Arduino.h"
#include "heltec.h"

extern SSD1306Wire displayOLED;

bool OLEDinited = false;

constexpr uint16_t oledLineCount = 6;
constexpr uint16_t oledCharCount = 32;
constexpr uint16_t oledLineHeight = 10;

typedef struct {
	char line[oledCharCount];
} OLED_Line;

OLED_Line oledLines[6];
uint16_t oledLineIndex = 0;
uint16_t oledCharIndex = 0;
bool oledNeedScroll = false;

void OLEDclear() {
	displayOLED.clear();
	displayOLED.display();
}

void OLEDshow() {
	displayOLED.clear();
	for (uint16_t i=0; i<oledLineCount; i++) {
		if (oledLines[i].line[0]) {
			displayOLED.drawString(0, i*oledLineHeight, oledLines[i].line);
		}
	}
	displayOLED.display();
}

void OLEDscrollIfNeeded() {
	if (oledNeedScroll) {
		memmove(&oledLines[0], &oledLines[1], (oledLineCount-1) * oledCharCount);
		memset(&oledLines[oledLineCount-1], 0, oledCharCount);
		oledNeedScroll = false;
	}
}

void OLEDprintln(const char* str, bool serialAlso=true) {
	if (serialAlso) {
		Serial.println(str);
	}

	if (OLEDinited) {
		OLEDscrollIfNeeded();
		if (oledCharIndex < oledCharCount-1) {
			memcpy(&oledLines[oledLineIndex].line[oledCharIndex], str, min((uint16_t)strlen(str), (uint16_t)(oledCharCount-1-oledCharIndex)) );
		}
		oledCharIndex = 0;
		if (oledLineIndex < (oledLineCount-1)) {
			oledLineIndex++;
		}
		else {
			oledNeedScroll = true;
		}
		OLEDshow();
	}
}

void OLEDprintln(int32_t val, bool serialAlso=true) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLEDprintln(valueBuff, serialAlso);
}

void OLEDprintln(float val, bool serialAlso=true) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLEDprintln(valueBuff, serialAlso);
}

void OLEDprint(const char* str, bool serialAlso=true) {
	if (serialAlso) {
		Serial.print(str);
	}

	if (OLEDinited) {
		OLEDscrollIfNeeded();
		if (oledCharIndex < oledCharCount-1) {
			uint16_t charsToCopy = min((uint16_t)strlen(str), (uint16_t)(oledCharCount-1-oledCharIndex));
			memcpy(&oledLines[oledLineIndex].line[oledCharIndex], str, charsToCopy );
			oledCharIndex += charsToCopy;
		}
	}
}

void OLEDprint(int32_t val, bool serialAlso=true) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLEDprint(valueBuff, serialAlso);
}

void OLEDprint(float val, bool serialAlso=true) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLEDprint(valueBuff, serialAlso);
}

