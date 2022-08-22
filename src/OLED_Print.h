#include "Arduino.h"
#include "heltec.h"

extern SSD1306Wire displayOLED;

bool OLED_inited = false;

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

void OLED_Clear() {
	displayOLED.clear();
	displayOLED.display();
}

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

void OLED_println(const char* str, bool serialAlso=true) {
	if (serialAlso) {
		Serial.println(str);
	}

	if (OLED_inited) {
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
}

void OLED_println(int32_t val, bool serialAlso=true) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLED_println(valueBuff, serialAlso);
}

void OLED_println(float val, bool serialAlso=true) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLED_println(valueBuff, serialAlso);
}

void OLED_print(const char* str, bool serialAlso=true) {
	if (serialAlso) {
		Serial.print(str);
	}

	if (OLED_inited) {
		OLED_scrollIfNeeded();
		if (oled_char_index < oled_char_count-1) {
			uint16_t charsToCopy = min((uint16_t)strlen(str), (uint16_t)(oled_char_count-1-oled_char_index));
			memcpy(&oled_lines[oled_line_index].line[oled_char_index], str, charsToCopy );
			oled_char_index += charsToCopy;
		}
	}
}

void OLED_print(int32_t val, bool serialAlso=true) {
	char valueBuff[20];
	itoa(val, valueBuff, 10);
	OLED_print(valueBuff, serialAlso);
}

void OLED_print(float val, bool serialAlso=true) {
	char valueBuff[20];
	dtostrf(val, 0, 2, valueBuff);
	OLED_print(valueBuff, serialAlso);
}

