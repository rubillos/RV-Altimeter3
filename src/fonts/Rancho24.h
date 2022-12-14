// Created by http://oleddisplay.squix.ch/ Consider a donation
// In case of problems make sure that you are using the font file with the correct version!
const uint8_t Rancho_Regular_24Bitmaps[] PROGMEM = {

	// Bitmap Data:
	0x00, // ' '
	0x4C,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x44,0xEC, // '!'
	0xDB,0x4D,0x24,0x92,0x40, // '"'
	0x0C,0xC0,0xD8,0x19,0x87,0xFE,0x7F,0xE1,0x90,0x13,0x07,0xFC,0xFF,0xC3,0x30,0x32,0x03,0x30, // '#'
	0x0C,0x01,0x80,0x38,0x1F,0xC6,0x19,0x83,0x30,0xC7,0x00,0x78,0x07,0xE0,0x1E,0x00,0xE4,0x0D,0xC3,0x1F,0xC1,0xF0,0x0C,0x01,0x80, // '$'
	0x38,0x31,0xF0,0xC6,0xC6,0x31,0x30,0xC4,0xC3,0x36,0x04,0xD8,0x1E,0xC0,0x02,0x70,0x1B,0xE0,0x4C,0x83,0x22,0x09,0x88,0x62,0x21,0x0D,0x8C,0x1C,0x10,0x00, // '%'
	0x0F,0x00,0x7E,0x03,0x98,0x0C,0x60,0x31,0x80,0xEC,0x01,0xF0,0x07,0x80,0x1C,0x00,0xF8,0x07,0x71,0x18,0xC4,0xE1,0x93,0x87,0xC6,0x0F,0x1E,0x7C,0x3F,0xF8,0x7C,0x40, // '&'
	0xDB,0x49,0x00, // '''
	0x0C,0x30,0xE1,0x87,0x0C,0x18,0x30,0xE0,0xC1,0x83,0x06,0x0C,0x0C,0x18,0x18,0x20,0x20, // '('
	0x41,0x86,0x1C,0x30,0xC3,0x86,0x18,0x61,0x86,0x38,0xC7,0x18,0xE3,0x08,0x00, // ')'
	0x46,0x3B,0x03,0xC8,0x7E,0x38,0x39,0x04,0x60,0x10, // '*'
	0x0C,0x03,0x00,0x83,0xFE,0xFF,0x82,0x01,0x80,0x60, // '+'
	0xEE,0x24, // ','
	0x7F,0x7F,0x80, // '-'
	0xEE, // '.'
	0x01,0x01,0x81,0x80,0xC0,0x60,0x60,0x30,0x30,0x18,0x0C,0x0C,0x06,0x03,0x03,0x01,0x80,0xC0,0xC0,0x60,0x00, // '/'
	0x08,0x07,0xE0,0xFE,0x34,0xC6,0x1D,0x81,0xB0,0x36,0x06,0xC0,0xD8,0x1B,0x03,0x60,0x6C,0x1D,0x83,0x38,0x63,0x1C,0x7F,0x03,0xC0, // '0'
	0x13,0x9D,0xE3,0x18,0xC6,0x31,0x8C,0x63,0x18,0xCE,0x73,0x80, // '1'
	0x0E,0x07,0xE1,0xC6,0x30,0xCE,0x1C,0x83,0x80,0x70,0x0C,0x03,0x80,0xE0,0x38,0x0E,0x07,0x00,0xC0,0x38,0x07,0x7C,0x7F,0xCE,0x08, // '2'
	0x7F,0xC7,0xFC,0x7F,0xC0,0x1C,0x03,0x80,0x30,0x06,0x00,0xC0,0x1F,0x81,0xFC,0x01,0xC0,0x0C,0x40,0xC4,0x0C,0xE0,0xC7,0x1C,0x7F,0x81,0xE0, // '3'
	0x03,0x80,0x78,0x0F,0x81,0xD8,0x19,0x83,0x98,0x31,0x87,0x18,0x61,0x8E,0x1E,0xFF,0xE7,0xFC,0x01,0x80,0x18,0x03,0x80,0x38,0x03,0x80,0x38, // '4'
	0x7D,0x9F,0xF3,0xFC,0x60,0x0C,0x01,0x80,0x31,0x07,0xF8,0xFF,0x8C,0x30,0x06,0x00,0xE0,0x1C,0x83,0xB0,0x67,0x1C,0x7F,0x07,0x80, // '5'
	0x0E,0x03,0xF0,0xE6,0x38,0x46,0x01,0x80,0x31,0x06,0xF8,0xF3,0x9C,0x33,0x87,0x60,0x6C,0x0D,0x83,0xB8,0x63,0x0C,0x3F,0x03,0xC0, // '6'
	0x3F,0x1F,0xFB,0xE3,0x20,0xC0,0x38,0x06,0x01,0xC0,0x30,0x06,0x01,0xC0,0x30,0x06,0x01,0xC0,0x38,0x06,0x00,0xC0,0x18,0x02,0x00, // '7'
	0x1C,0x1F,0xC6,0x33,0x8E,0xC1,0xB0,0x6E,0x31,0xFC,0x3F,0x1D,0xCE,0x3B,0x06,0xC1,0xB0,0x6C,0x1B,0x8E,0x7F,0x0F,0x80, // '8'
	0x1C,0x1F,0x86,0x33,0x0E,0xC1,0xB0,0x6C,0x1B,0x06,0xC3,0xB9,0xE7,0xF8,0x6E,0x03,0x01,0xC0,0xE0,0xF0,0x78,0x0C,0x00, // '9'
	0x6E,0x40,0x0E,0xE0, // ':'
	0x6E,0x40,0x06,0xE2,0x40, // ';'
	0x08,0x63,0x98,0xC3,0x06,0x0C,0x18,0x20, // '<'
	0xFE,0xC2,0x00,0xFE,0x80, // '='
	0x81,0x83,0x81,0xC1,0x83,0x0E,0x38,0xE1,0x80, // '>'
	0x1E,0x0F,0xF1,0x86,0x70,0xEE,0x0D,0xE1,0x90,0x70,0x0C,0x03,0x83,0xC0,0x60,0x0C,0x01,0x80,0x38,0x02,0x00,0x60,0x1C,0x03,0x80, // '?'
	0x03,0x80,0x31,0x81,0x81,0x0C,0xD6,0x66,0xC9,0x93,0x2C,0xCC,0xB2,0x22,0xD9,0x8B,0x66,0x6C,0xE9,0x91,0x2C,0x60,0xE0,0xFC,0x00, // '@'
	0x01,0x80,0x0E,0x00,0x7C,0x01,0xB0,0x06,0xC0,0x3B,0x00,0xCC,0x03,0x38,0x1C,0x60,0x61,0x87,0xFE,0x3E,0x18,0x30,0x70,0xC0,0xC7,0x03,0x18,0x0E,0x60,0x39,0xC0,0x60, // 'A'
	0x00,0x00,0xFC,0x1F,0xF9,0xF1,0xED,0x87,0x4C,0x19,0x60,0xC3,0x0E,0x18,0x60,0xCE,0x07,0xC0,0x37,0x81,0x8E,0x0C,0x30,0x61,0x83,0x1C,0x18,0xC3,0xFC,0x0F,0xC0,0x60,0x00, // 'B'
	0x0F,0x81,0xCC,0x38,0xE3,0x06,0x60,0xE6,0x0E,0x63,0xCE,0x00,0xE0,0x0E,0x00,0xE0,0x0E,0x00,0xE0,0x46,0x04,0x70,0xC3,0x0C,0x3F,0x80,0xF0, // 'C'
	0x04,0x00,0x3C,0x03,0xFC,0x3B,0x38,0xCC,0x71,0x30,0xC4,0xC3,0x83,0x0E,0x0C,0x38,0x20,0xE1,0x83,0x86,0x0C,0x18,0x30,0x61,0xC1,0x86,0x06,0x30,0x1B,0x81,0xFC,0x07,0xC0,0x00, // 'D'
	0x3F,0xEF,0xFE,0x7F,0xE3,0x00,0x30,0x03,0x00,0x30,0x03,0x00,0x3F,0x87,0xFC,0x30,0x03,0x00,0x30,0x03,0x00,0x30,0x03,0x00,0x7F,0xE7,0xFC, // 'E'
	0x3F,0xDF,0xFB,0xFF,0x30,0x06,0x00,0xC0,0x18,0x03,0x00,0x7F,0x9F,0xF1,0x80,0x30,0x06,0x00,0xC0,0x18,0x03,0x80,0x70,0x0C,0x00, // 'F'
	0x0F,0x03,0xB8,0xC3,0x38,0x66,0x0D,0xC3,0xB0,0xF6,0x00,0xC0,0x18,0x03,0x03,0x60,0x6C,0x0D,0xC1,0x98,0x73,0x8E,0x3F,0xC3,0xD8, // 'G'
	0x10,0x11,0xC1,0x8C,0x0C,0x60,0x63,0x03,0x18,0x18,0xC0,0xC6,0x06,0x3F,0xF7,0xFF,0x9C,0x0C,0x60,0x63,0x07,0x18,0x38,0xC1,0xC6,0x0E,0x38,0x71,0x83,0x00, // 'H'
	0x3C,0xFE,0xF2,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0xFC,0xFC, // 'I'
	0x03,0xC1,0xFE,0x1F,0xE0,0x30,0x03,0x80,0x38,0x01,0x80,0x18,0x01,0x80,0x18,0x61,0x8E,0x18,0xC1,0x8C,0x18,0xC3,0x8E,0x30,0x77,0x03,0xE0, // 'J'
	0x40,0x8C,0x1C,0xC1,0x8C,0x38,0xC7,0x0C,0x60,0xCC,0x0D,0xC0,0xF8,0x0F,0x80,0xEC,0x0C,0xC0,0xC6,0x0C,0x70,0xC3,0x0C,0x38,0xC1,0xCC,0x0E, // 'K'
	0x60,0x70,0x38,0x1C,0x0E,0x07,0x03,0x81,0x80,0xC0,0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0x9E,0xFF,0x7F,0x80, // 'L'
	0x20,0x0C,0x70,0x1C,0x70,0x3C,0x70,0x3C,0x78,0x3C,0x78,0x3C,0xF8,0x6C,0xEC,0x6C,0xEC,0x6C,0xCC,0xCC,0xC6,0xCC,0xC6,0xCC,0xC7,0x8C,0xE3,0x8C,0xE3,0x8C,0xE3,0x8C,0xE3,0x0C,0xE1,0x0E, // 'M'
	0x20,0x2E,0x0E,0xF0,0xCF,0x0C,0xF0,0xCF,0x8C,0xD8,0xCC,0xCC,0xCC,0xCC,0xCC,0xC6,0xCC,0x6C,0xC3,0xCC,0x3C,0xC3,0xCE,0x1C,0xE1,0xC6,0x08, // 'N'
	0x0E,0x01,0xF8,0x3F,0xC7,0x4C,0x60,0xEE,0x06,0xC0,0x6C,0x06,0xC0,0x6C,0x06,0xC0,0x6C,0x06,0xC0,0xEE,0x0C,0xE0,0xC7,0x18,0x3F,0x81,0xE0, // 'O'
	0x00,0x01,0xF0,0x7F,0xCF,0x9E,0xD8,0xE9,0x86,0x98,0x61,0x86,0x18,0x61,0x8E,0x19,0xC3,0xB8,0x7F,0x03,0x80,0x38,0x03,0x80,0x38,0x03,0x80,0x38,0x00, // 'P'
	0x05,0x00,0x3F,0x01,0xEE,0x0F,0x18,0x70,0x71,0xC1,0xC6,0x03,0x18,0x0C,0xE0,0x33,0x80,0xCE,0x03,0x18,0x1C,0x68,0x71,0xF9,0x87,0x3E,0x0E,0x70,0x3F,0x80,0x3F,0x00,0x0F,0x80,0x08, // 'Q'
	0x00,0x00,0x3F,0x01,0xFF,0x83,0xC7,0x0F,0x87,0x1B,0x0E,0x16,0x0C,0x0C,0x18,0x18,0x70,0x30,0xC0,0x67,0x01,0xFC,0x03,0xDC,0x03,0x18,0x06,0x30,0x0C,0x70,0x18,0x60,0x30,0xE0,0x60,0xF0,0x00,0xC0, // 'R'
	0x1F,0x87,0xF9,0xC7,0x60,0x68,0x0D,0x00,0x30,0x07,0x80,0x7C,0x03,0xE0,0x1E,0x00,0xE6,0x0D,0xC1,0xB8,0x37,0x0E,0x73,0x83,0xE0, // 'S'
	0x1F,0xDF,0xFB,0xFE,0x0C,0x01,0xC0,0x38,0x07,0x00,0xC0,0x18,0x03,0x00,0x60,0x0C,0x01,0x80,0x30,0x06,0x00,0xC0,0x1C,0x01,0x80, // 'T'
	0x20,0xCC,0x3B,0x87,0x70,0xEE,0x1D,0xC3,0xB8,0x36,0x06,0xC0,0xD8,0x1B,0x03,0x60,0x6C,0x0D,0xC1,0x98,0x73,0x8C,0x3F,0x03,0xC0, // 'U'
	0x60,0x6E,0x0E,0xE0,0xCE,0x1C,0xE1,0xC6,0x18,0x61,0x86,0x38,0x73,0x07,0x30,0x33,0x03,0x70,0x36,0x03,0xE0,0x1E,0x01,0xE0,0x1C,0x00,0xC0, // 'V'
	0x40,0x05,0x86,0x1B,0x0C,0x76,0x38,0xEC,0x79,0xD8,0xF3,0x31,0x66,0x66,0xCC,0xED,0x99,0xDB,0x31,0xB2,0x43,0xC5,0x87,0x8F,0x0F,0x1E,0x1E,0x3C,0x1C,0x70,0x30,0xE0,0x60,0xC0, // 'W'
	0x40,0x6E,0x0E,0x60,0xC6,0x1C,0x31,0x83,0x30,0x3F,0x01,0xE0,0x1E,0x00,0xC0,0x1C,0x01,0xE0,0x36,0x03,0x70,0x63,0x8E,0x38,0xC1,0xCC,0x0E, // 'X'
	0x60,0x8C,0x33,0x06,0x60,0xCC,0x1D,0x83,0xB0,0x76,0x0E,0xC1,0xD8,0x3B,0x87,0x31,0xE7,0xEC,0x7F,0x80,0x62,0x0C,0x41,0x8C,0x61,0xFC,0x0E,0x00, // 'Y'
	0x3F,0x0F,0xFE,0xE0,0xE8,0x0E,0x00,0xC0,0x18,0x03,0x80,0x70,0x06,0x00,0xE0,0x1C,0x01,0x80,0x30,0x07,0x00,0x60,0x0D,0xFC,0xFF,0xC6,0x1C, // 'Z'
	0xEE,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xEE,0x60, // '['
	0x40,0xC0,0x60,0x60,0x60,0x60,0x30,0x30,0x30,0x30,0x18,0x18,0x18,0x0C,0x0C,0x0C,0x0E,0x06, // '\'
	0xEE,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0xEE,0xC0, // ']'
	0x18,0x0C,0x0F,0x07,0x83,0xC3,0x61,0x98,0xCC,0xC6,0x63,0x90,0x00, // '^'
	0x7F,0xF7,0xFF,0x00, // '_'
	0x0C,0xC6,0x20, // '`'
	0x1F,0x1D,0x9C,0xCC,0x66,0x36,0x39,0x9C,0xF4,0x3B,0x00, // 'a'
	0x40,0x60,0x30,0x18,0x0C,0x06,0x03,0x01,0xBC,0xD3,0x71,0xB8,0xD8,0x6C,0x37,0x1B,0xD9,0xB8, // 'b'
	0x0C,0x3E,0x6E,0x6E,0x40,0xC0,0xC0,0x62,0x66,0x3C, // 'c'
	0x03,0x01,0x80,0xC0,0x60,0x30,0x18,0x0C,0x7E,0x37,0x33,0x98,0xC8,0x6C,0x36,0x39,0xBC,0xF6, // 'd'
	0x0C,0x3E,0x76,0x66,0x66,0xDC,0xF8,0x42,0x66,0x3C, // 'e'
	0x1E,0x1F,0x8C,0xC6,0x07,0x03,0x63,0xF1,0xC0,0x60,0x18,0x0C,0x06,0x03,0x01,0x80,0xC0, // 'f'
	0x01,0x1F,0x99,0xCC,0x6C,0x36,0x1B,0x08,0x8C,0x6E,0x1D,0x00,0x8C,0x44,0x62,0x31,0xF0,0x60, // 'g'
	0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xCC,0xDE,0xD6,0xE6,0xE6,0xE6,0xE4,0xC4,0xC6, // 'h'
	0xEA,0xC0,0xCC,0xCC,0xCC,0xCC,0xC0, // 'i'
	0x18,0x48,0x60,0x01,0x83,0x06,0x0C,0x18,0x30,0x60,0xC1,0x83,0x06,0x08,0xF0,0xC0, // 'j'
	0xC0,0x30,0x0C,0x03,0x00,0xC0,0x30,0x8C,0x63,0x30,0xD8,0x3E,0x0E,0xC3,0x30,0xCC,0x31,0x0C,0x60,0x1E, // 'k'
	0xEE,0xEC,0xCC,0xCC,0xCC,0xCC,0xCC,0xE0, // 'l'
	0xCE,0x63,0x7B,0xCD,0x6B,0x38,0xCC,0xE3,0x33,0x8C,0xCC,0x23,0x30,0x8C,0xC2,0x38, // 'm'
	0x48,0xDE,0xD6,0xF6,0xE6,0xE6,0xE6,0xC6,0xC6, // 'n'
	0x3C,0x7E,0x76,0x42,0x42,0xC6,0x46,0x66,0x3C, // 'o'
	0x6E,0x39,0x98,0xCC,0x26,0x33,0x19,0x8C,0xEE,0x7E,0x20,0x10,0x08,0x0C,0x06,0x01,0x80, // 'p'
	0x1F,0x11,0x98,0xCC,0x6E,0x37,0x19,0x8C,0xCE,0x3F,0x01,0x80,0xC0,0x60,0x30,0x18,0x0C, // 'q'
	0xDD,0xFB,0xB6,0x4C,0x18,0x30,0x60,0xC0, // 'r'
	0x18,0xDB,0x34,0x6C,0x1C,0x06,0x42,0xCD,0xF0, // 's'
	0x18,0x01,0x80,0x18,0x0F,0xFC,0x18,0x21,0x80,0x10,0x01,0x00,0x30,0x03,0x00,0x30,0x03,0x00,0x30,0x03,0x00,0x38,0x00, // 't'
	0xC6,0xC6,0xC6,0xC6,0xC6,0xCE,0xCE,0xDE,0x76, // 'u'
	0xE6,0x62,0x62,0x66,0x66,0x64,0x24,0x3C,0x38, // 'v'
	0x40,0x07,0x31,0x99,0x8C,0xCC,0x66,0x63,0x33,0x31,0x9D,0x85,0xAC,0x29,0xC1,0xCE,0x00, // 'w'
	0xC6,0x66,0x2C,0x38,0x30,0x38,0x6C,0x64,0x46,0x40, // 'x'
	0xE6,0xC6,0xC6,0xC6,0xC6,0xC6,0xCE,0xDE,0xF6,0x06,0x06,0xC4,0xC4,0xEC,0x78, // 'y'
	0x7E,0xE6,0x06,0x0C,0x18,0x10,0x30,0x7E,0x7E,0x20, // 'z'
	0x08,0xE3,0x9C,0x61,0x86,0x08,0x61,0x8E,0x1C,0x30,0xC7,0x18,0x60,0xC3,0x04, // '{'
	0x4C,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCE, // '|'
	0x83,0x8F,0x0C,0x30,0xC3,0x0C,0x30,0xC3,0x8C,0x71,0xC3,0x0C,0x31,0xC6,0x10 // '}'
};
const GFXglyph Rancho_Regular_24Glyphs[] PROGMEM = {
// bitmapOffset, width, height, xAdvance, xOffset, yOffset
	  {     0,   1,   1,   6,    0,    0 }, // ' '
	  {     1,   4,  18,   5,    1,  -16 }, // '!'
	  {    10,   6,   6,   7,    1,  -16 }, // '"'
	  {    15,  12,  12,  12,    0,  -13 }, // '#'
	  {    33,  11,  18,  11,    0,  -16 }, // '$'
	  {    58,  14,  17,  15,    0,  -15 }, // '%'
	  {    88,  14,  18,  14,    0,  -16 }, // '&'
	  {   120,   3,   6,   4,    1,  -16 }, // '''
	  {   123,   7,  19,   7,    0,  -16 }, // '('
	  {   140,   6,  19,   6,    0,  -16 }, // ')'
	  {   155,  10,   8,  10,    0,  -13 }, // '*'
	  {   165,  10,   8,  10,    0,  -11 }, // '+'
	  {   175,   4,   4,   5,    0,   -2 }, // ','
	  {   177,   9,   2,   9,    0,   -8 }, // '-'
	  {   180,   4,   2,   4,    0,   -2 }, // '.'
	  {   181,   9,  18,   9,    0,  -16 }, // '/'
	  {   202,  11,  18,  13,    1,  -16 }, // '0'
	  {   227,   5,  18,   6,    0,  -16 }, // '1'
	  {   239,  11,  18,  11,    0,  -16 }, // '2'
	  {   264,  12,  18,  12,    0,  -16 }, // '3'
	  {   291,  12,  18,  12,    0,  -16 }, // '4'
	  {   318,  11,  18,  12,    1,  -16 }, // '5'
	  {   343,  11,  18,  12,    1,  -16 }, // '6'
	  {   368,  11,  18,  11,    0,  -16 }, // '7'
	  {   393,  10,  18,  12,    1,  -16 }, // '8'
	  {   416,  10,  18,  12,    1,  -16 }, // '9'
	  {   439,   4,   7,   5,    0,   -7 }, // ':'
	  {   443,   4,   9,   5,    0,   -7 }, // ';'
	  {   448,   6,  10,   7,    0,  -12 }, // '<'
	  {   456,   8,   5,   9,    0,   -9 }, // '='
	  {   461,   7,  10,   7,    0,  -12 }, // '>'
	  {   470,  11,  18,  11,    0,  -16 }, // '?'
	  {   495,  14,  14,  15,    0,  -14 }, // '@'
	  {   520,  14,  18,  14,    0,  -16 }, // 'A'
	  {   552,  13,  20,  13,    0,  -17 }, // 'B'
	  {   585,  12,  18,  12,    0,  -16 }, // 'C'
	  {   612,  14,  19,  14,    0,  -17 }, // 'D'
	  {   646,  12,  18,  12,    0,  -16 }, // 'E'
	  {   673,  11,  18,  11,    1,  -16 }, // 'F'
	  {   698,  11,  18,  13,    1,  -16 }, // 'G'
	  {   723,  13,  18,  14,    0,  -16 }, // 'H'
	  {   753,   8,  18,   8,    0,  -16 }, // 'I'
	  {   771,  12,  18,  12,    0,  -16 }, // 'J'
	  {   798,  12,  18,  12,    1,  -16 }, // 'K'
	  {   825,   9,  18,  10,    1,  -16 }, // 'L'
	  {   846,  16,  18,  17,    1,  -16 }, // 'M'
	  {   882,  12,  18,  14,    1,  -16 }, // 'N'
	  {   909,  12,  18,  14,    1,  -16 }, // 'O'
	  {   936,  12,  19,  12,    0,  -17 }, // 'P'
	  {   965,  14,  20,  14,    0,  -16 }, // 'Q'
	  {  1000,  15,  20,  13,   -1,  -17 }, // 'R'
	  {  1038,  11,  18,  13,    1,  -16 }, // 'S'
	  {  1063,  11,  18,  10,    0,  -16 }, // 'T'
	  {  1088,  11,  18,  13,    1,  -16 }, // 'U'
	  {  1113,  12,  18,  11,    0,  -16 }, // 'V'
	  {  1140,  15,  18,  16,    1,  -16 }, // 'W'
	  {  1174,  12,  18,  11,    0,  -16 }, // 'X'
	  {  1201,  11,  20,  13,    1,  -16 }, // 'Y'
	  {  1229,  12,  18,  12,    0,  -16 }, // 'Z'
	  {  1256,   4,  19,   6,    1,  -16 }, // '['
	  {  1266,   8,  18,   8,    0,  -16 }, // '\'
	  {  1284,   4,  19,   6,    1,  -16 }, // ']'
	  {  1294,   9,  11,   9,    0,  -16 }, // '^'
	  {  1307,  13,   2,  13,    0,   -2 }, // '_'
	  {  1311,   4,   5,  14,    5,  -17 }, // '`'
	  {  1314,   9,   9,  10,    0,   -9 }, // 'a'
	  {  1325,   9,  16,  11,    1,  -16 }, // 'b'
	  {  1343,   8,  10,   9,    0,  -10 }, // 'c'
	  {  1353,   9,  16,  10,    0,  -16 }, // 'd'
	  {  1371,   8,  10,   9,    0,  -10 }, // 'e'
	  {  1381,   9,  15,   7,    0,  -15 }, // 'f'
	  {  1398,   9,  16,   9,    0,  -10 }, // 'g'
	  {  1416,   8,  15,  10,    1,  -15 }, // 'h'
	  {  1431,   4,  13,   5,    1,  -13 }, // 'i'
	  {  1438,   7,  18,   5,   -2,  -13 }, // 'j'
	  {  1454,  10,  16,   9,    1,  -15 }, // 'k'
	  {  1474,   4,  15,   5,    1,  -15 }, // 'l'
	  {  1482,  14,   9,  15,    1,   -9 }, // 'm'
	  {  1498,   8,   9,  10,    1,   -9 }, // 'n'
	  {  1507,   8,   9,   9,    0,   -9 }, // 'o'
	  {  1516,   9,  15,  10,    0,   -9 }, // 'p'
	  {  1533,   9,  15,  10,    0,   -9 }, // 'q'
	  {  1550,   7,   9,   8,    1,   -9 }, // 'r'
	  {  1558,   7,  10,   9,    1,  -10 }, // 's'
	  {  1567,  12,  15,   6,   -1,  -15 }, // 't'
	  {  1590,   8,   9,  10,    1,   -9 }, // 'u'
	  {  1599,   8,   9,   9,    0,   -9 }, // 'v'
	  {  1608,  13,  10,  13,    0,  -10 }, // 'w'
	  {  1625,   8,  10,   8,    0,   -9 }, // 'x'
	  {  1635,   8,  15,  10,    1,   -9 }, // 'y'
	  {  1650,   8,  10,   9,    0,   -9 }, // 'z'
	  {  1660,   6,  20,   6,    0,  -17 }, // '{'
	  {  1675,   4,  18,   6,    1,  -16 }, // '|'
	  {  1684,   6,  20,   6,    0,  -17 } // '}'
};
const GFXfont Rancho_Regular_24 PROGMEM = {
(uint8_t  *)Rancho_Regular_24Bitmaps,(GFXglyph *)Rancho_Regular_24Glyphs,0x20, 0x7E, 31};
