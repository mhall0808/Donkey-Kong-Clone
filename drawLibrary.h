#ifndef DRAW_LIBRARY
#define DRAW_LIBRARY

#include "msp430g2553.h"

// Pin Definitions
#define LCD_MOSI	0x0080		// data out at P1.7
#define LCD_SCK		0x0020		// serial clk at P1.5
#define LCD_RST 	0x0001		// reset at P2.0
#define LCD_CS		0x0002		// chip select at P2.1
#define LCD_DC		0x0004		// Data/Cmd at P2.2

// Color Definitions
#define RED			0xF800
#define GRN			0x07E0
#define BLU 		0x001F
#define BLK			0x0000
#define CYAN 		0x07FF
#define MAGENTA 	0xF81F
#define YELLOW 		0xFFE0
#define WHITE 		0xFFFF
#define BROWN       0x79E0

/**
 * # define for the touchscreen input.
 */
#define TS_XP 0x0004 // X+ @ P1.2
#define TS_XM 0x0001 // X- at P1.0 (Analog In)
#define TS_YP 0x0002 // Y+ at P1.1 (Analog In)
#define TS_YM 0x0008 // Y- at P1.3

// In case I end up with another screen, this is hard coded in.
#define X_MAX 240
#define Y_MAX 320

typedef struct Item {
	int xCoords;
	int yCoords;
} Item;

/**
 * I am not going to calibrate anything on the screen; I really don't need to.
 * 	Instead, I am going to simply have a "Tap to pause/continue" function.
 */
int adcZ = 0;
int count = 0;
int score = 0;
int joystickValue = 0;
int joystickSpeed = 0; // This will throttle the joystick speed via clocking it.
int enemySpeed = 0;
int playerY = 20 * 16;
int playerX = 13 * 16;
int jumpSpeed = 0;
int jumpCount = 0;
int spawnSpeed = 256;
int spawnCounter = 0;
int lives = 5;

const char brick[32] = { 0xff, 0xff, 0x80, 0x21, 0x80, 0x21, 0x80, 0x21, 0xff,
		0xff, 0x82, 0x01, 0x82, 0x01, 0x82, 0x01, 0xff, 0xff, 0x80, 0x81, 0x80,
		0x81, 0x80, 0x81, 0xff, 0xff, 0x88, 0x01, 0x88, 0x01, 0xff, 0xff };

const char ladder[32] = { 0x30, 0x0c, 0x30, 0x0c, 0x3f, 0xfc, 0x3f, 0xfc, 0x30,
		0x0c, 0x30, 0x0c, 0x3f, 0xfc, 0x3f, 0xfc, 0x30, 0x0c, 0x30, 0x0c, 0x3f,
		0xfc, 0x3f, 0xfc, 0x30, 0x0c, 0x30, 0x0c, 0x3f, 0xfc, 0x3f, 0xfc };

const char penguin[32] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x0c,
		0x30, 0x1a, 0x58, 0x12, 0x48, 0x32, 0x4c, 0x60, 0x06, 0x67, 0xe6, 0x63,
		0xc6, 0x61, 0x86, 0x60, 0x06, 0x30, 0x0c, 0x18, 0x18, 0x0f, 0xf0 };

const char badGuy[32] = { 0x00, 0x00, 0x07, 0xc0, 0x0f, 0xe0, 0x1f, 0xf0, 0x1b,
		0x70, 0x1b, 0x70, 0x1b, 0x70, 0x1f, 0xf0, 0x58, 0x34, 0x4f, 0xe4, 0x43,
		0x84, 0x7f, 0xfc, 0x07, 0x80, 0x0c, 0xc0, 0x08, 0x60, 0x18, 0x20 };

const char badGuyHead[32] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x0f, 0xe0, 0x1b, 0xb0, 0x3b, 0xb8,
		0x3b, 0xb8, 0x3b, 0xb8, 0x3f, 0xf8, 0x38, 0x38, 0x2f, 0xe8, 0x27, 0xc8 };

const char player[32] = { 0x07, 0xe0, 0x08, 0x10, 0x10, 0x08, 0x22, 0x44, 0x28,
		0x14, 0x2c, 0x34, 0x17, 0xc8, 0x08, 0x10, 0x47, 0xe2, 0x61, 0x06, 0x31,
		0x0c, 0x1f, 0xf8, 0x01, 0x00, 0x01, 0x00, 0x03, 0x80, 0x06, 0xc0 };

const Item ladders[24] = { { 4 * 16, 13 * 16 }, { 4 * 16, 12 * 16 }, { 4 * 16,
		11 * 16 }, { 15 * 16, 10 * 16 }, { 15 * 16, 9 * 16 },
		{ 15 * 16, 8 * 16 }, { 9 * 16, 10 * 16 }, { 9 * 16, 9 * 16 }, { 9 * 16,
				8 * 16 }, { 3 * 16, 7 * 16 }, { 3 * 16, 6 * 16 }, { 3 * 16, 5
				* 16 }, { 3 * 16, 4 * 16 }, { 16 * 16, 3 * 16 }, { 16 * 16, 2
				* 16 } };

/**
 * This is an array of chars that output 128 different characters based on the selection of the
 * user.  Makes things easy.
 */
const char font[1024] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Char 000 (.)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 001 (.)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 002 (.)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 003 (.)
		0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,	// Char 004 (.)
		0x3C, 0x3C, 0x18, 0xFF, 0xE7, 0x18, 0x3C, 0x00,	// Char 005 (.)
		0x10, 0x38, 0x7C, 0xFE, 0xEE, 0x10, 0x38, 0x00,	// Char 006 (.)
		0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,	// Char 007 (.)
		0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,	// Char 008 (.)
		0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,	// Char 009 (.)
		0xFF, 0xC3, 0x99, 0xBD, 0xBD, 0x99, 0xC3, 0xFF,	// Char 010 (.)
		0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,	// Char 011 (.)
		0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,	// Char 012 (.)
		0x08, 0x0C, 0x0A, 0x0A, 0x08, 0x78, 0xF0, 0x00,	// Char 013 (.)
		0x18, 0x14, 0x1A, 0x16, 0x72, 0xE2, 0x0E, 0x1C,	// Char 014 (.)
		0x10, 0x54, 0x38, 0xEE, 0x38, 0x54, 0x10, 0x00,	// Char 015 (.)
		0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,	// Char 016 (.)
		0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,	// Char 017 (.)
		0x18, 0x3C, 0x5A, 0x18, 0x5A, 0x3C, 0x18, 0x00,	// Char 018 (.)
		0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,	// Char 019 (.)
		0x7F, 0xDB, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x00,	// Char 020 (.)
		0x1C, 0x22, 0x38, 0x44, 0x44, 0x38, 0x88, 0x70,	// Char 021 (.)
		0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,	// Char 022 (.)
		0x18, 0x3C, 0x5A, 0x18, 0x5A, 0x3C, 0x18, 0x7E,	// Char 023 (.)
		0x18, 0x3C, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x00,	// Char 024 (.)
		0x18, 0x18, 0x18, 0x18, 0x5A, 0x3C, 0x18, 0x00,	// Char 025 (.)
		0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,	// Char 026 (.)
		0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00,	// Char 027 (.)
		0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00,	// Char 028 (.)
		0x00, 0x24, 0x42, 0xFF, 0x42, 0x24, 0x00, 0x00,	// Char 029 (.)
		0x00, 0x10, 0x38, 0x7C, 0xFE, 0xFE, 0x00, 0x00,	// Char 030 (.)
		0x00, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00,	// Char 031 (.)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 032 ( )
		0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,	// Char 033 (!)
		0x6C, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 034 (")
		0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,	// Char 035 (#)
		0x10, 0x7C, 0xD0, 0x7C, 0x16, 0xFC, 0x10, 0x00,	// Char 036 ($)
		0x00, 0x66, 0xAC, 0xD8, 0x36, 0x6A, 0xCC, 0x00,	// Char 037 (%)
		0x38, 0x4C, 0x38, 0x78, 0xCE, 0xCC, 0x7A, 0x00,	// Char 038 (&)
		0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 039 (')
		0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,	// Char 040 (()
		0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,	// Char 041 ())
		0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00,	// Char 042 (*)
		0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00,	// Char 043 (+)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20,	// Char 044 (,)
		0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,	// Char 045 (-)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,	// Char 046 (.)
		0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00,	// Char 047 (/)
		0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xE6, 0x7C, 0x00,	// Char 048 (0)
		0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x7E, 0x00,	// Char 049 (1)
		0x7C, 0xC6, 0x06, 0x1C, 0x70, 0xC6, 0xFE, 0x00,	// Char 050 (2)
		0x7C, 0xC6, 0x06, 0x3C, 0x06, 0xC6, 0x7C, 0x00,	// Char 051 (3)
		0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00,	// Char 052 (4)
		0xFE, 0xC0, 0xFC, 0x06, 0x06, 0xC6, 0x7C, 0x00,	// Char 053 (5)
		0x7C, 0xC6, 0xC0, 0xFC, 0xC6, 0xC6, 0x7C, 0x00,	// Char 054 (6)
		0xFE, 0xC6, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,	// Char 055 (7)
		0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0x00,	// Char 056 (8)
		0x7C, 0xC6, 0xC6, 0x7E, 0x06, 0xC6, 0x7C, 0x00,	// Char 057 (9)
		0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00,	// Char 058 (:)
		0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20,	// Char 059 (;)
		0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00,	// Char 060 (<)
		0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00,	// Char 061 (=)
		0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,	// Char 062 (>)
		0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00,	// Char 063 (?)
		0x7C, 0x82, 0x9E, 0xA6, 0x9E, 0x80, 0x7C, 0x00,	// Char 064 (@)
		0x7C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00,	// Char 065 (A)
		0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,	// Char 066 (B)
		0x7C, 0xC6, 0xC0, 0xC0, 0xC0, 0xC6, 0x7C, 0x00,	// Char 067 (C)
		0xFC, 0x66, 0x66, 0x66, 0x66, 0x66, 0xFC, 0x00,	// Char 068 (D)
		0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00,	// Char 069 (E)
		0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00,	// Char 070 (F)
		0x7C, 0xC6, 0xC6, 0xC0, 0xCE, 0xC6, 0x7E, 0x00,	// Char 071 (G)
		0xC6, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00,	// Char 072 (H)
		0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00,	// Char 073 (I)
		0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,	// Char 074 (J)
		0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00,	// Char 075 (K)
		0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,	// Char 076 (L)
		0x82, 0xC6, 0xEE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,	// Char 077 (M)
		0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,	// Char 078 (N)
		0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,	// Char 079 (O)
		0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,	// Char 080 (P)
		0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0xDE, 0x7C, 0x06,	// Char 081 (Q)
		0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xE6, 0x00,	// Char 082 (R)
		0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00,	// Char 083 (S)
		0x7E, 0x5A, 0x5A, 0x18, 0x18, 0x18, 0x3C, 0x00,	// Char 084 (T)
		0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,	// Char 085 (U)
		0xC6, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00,	// Char 086 (V)
		0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x82, 0x00,	// Char 087 (W)
		0xC6, 0x6C, 0x38, 0x38, 0x38, 0x6C, 0xC6, 0x00,	// Char 088 (X)
		0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x00,	// Char 089 (Y)
		0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,	// Char 090 (Z)
		0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,	// Char 091 ([)
		0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,	// Char 092 (\)
		0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,	// Char 093 (])
		0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00,	// Char 094 (^)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,	// Char 095 (_)
		0x30, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 096 (`)
		0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,	// Char 097 (a)
		0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00,	// Char 098 (b)
		0x00, 0x00, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00,	// Char 099 (c)
		0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,	// Char 100 (d)
		0x00, 0x00, 0x7C, 0xC6, 0xFE, 0xC0, 0x7C, 0x00,	// Char 101 (e)
		0x1C, 0x36, 0x30, 0x78, 0x30, 0x30, 0x78, 0x00,	// Char 102 (f)
		0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x78,	// Char 103 (g)
		0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,	// Char 104 (h)
		0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,	// Char 105 (i)
		0x00, 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0x78,	// Char 106 (j)
		0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,	// Char 107 (k)
		0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00,	// Char 108 (l)
		0x00, 0x00, 0xCC, 0xFE, 0xD6, 0xD6, 0xD6, 0x00,	// Char 109 (m)
		0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x00,	// Char 110 (n)
		0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,	// Char 111 (o)
		0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0,	// Char 112 (p)
		0x00, 0x00, 0x7C, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,	// Char 113 (q)
		0x00, 0x00, 0xDE, 0x76, 0x60, 0x60, 0xF0, 0x00,	// Char 114 (r)
		0x00, 0x00, 0x7C, 0xC0, 0x7C, 0x06, 0x7C, 0x00,	// Char 115 (s)
		0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,	// Char 116 (t)
		0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00,	// Char 117 (u)
		0x00, 0x00, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00,	// Char 118 (v)
		0x00, 0x00, 0xC6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00,	// Char 119 (w)
		0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00,	// Char 120 (x)
		0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,	// Char 121 (y)
		0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,	// Char 122 (z)
		0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00,	// Char 123 ({)
		0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,	// Char 124 (|)
		0xE0, 0x30, 0x30, 0x18, 0x30, 0x30, 0xE0, 0x00,	// Char 125 (})
		0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// Char 126 (~)
		0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00,	// Char 127 (.)
		};

// bad guy head allocates to 0, or null.
Item badHead[8] = { 0, 0 };

// location of other key items
Item penguinLoc = { 19 * 16, 0 };
Item badGuyLoc = { 17 * 16, 1 * 16 };

// Move my prototypes here.
void fillScreen(int);
void drawPixel(int, int, int);
void writeLCDData(char);
void writeLCDControl(char);
void waitMS(unsigned int);
void initClk(void);
void initPins(void);
void initLCD(void);
void initUSCI(void);
void drawLetter(int, int, int, char);
void drawNumber(int, int, int, int);
void displayVoltage();
void drawString(int, int, int, char[16]);
void drawInteger(int, int, int, int);
void drawRect(int, int, int, int, int);
void channelCheck();
void draw16(int, int, const char[], char);
void createBricks(char[]);
void collisionDetection();
void joystickCheck();
void fallCheck();
void enemyMovementCheck();
void enemyFallCheck();
void badGuyHeadCheck();
void buttonCheck();
void victoryConditionCheck();
void victorySequence();
void gameOver();
int ladderCheck(int, int);
int brickCheck(int, int);
void openingLoad();

/**
 * Pulled from: https://www.min.at/prinz/o/software/pixelfont/
 *
 * This was a program that could create an 8x8 char array and convert it to
 * a 256 ascii table.  I only need 128, so I am chopping half of it, then
 * adding my own functions to it.
 */
// Font used: Standard.pf
void drawLetter(int y, int x, int color, char letter) {
	char letterArray[8] = { 0 };
	unsigned int i = 0;
	unsigned int j = 0;

	for (i; i < 8; i++) {
		letterArray[i] = font[i + (letter * 8)];
	}

	i = 0;
	for (i; i < 8; i++)	// Loop through all memory locations 16 bit color
			{
		for (j = 8; j > 0; j--) {
			if ((letterArray[i] & 0x80) == 0x80) {
				drawPixel(i + y, j + x, color);
			}
			letterArray[i] = letterArray[i] << 1;
		}
	}
}

/**
 * These functions were totally made by me, I decided to move all draw
 * functions into one file.  Rock and roll!
 */
/**
 * DRAW STRING
 * 	For loop that just draws a string.
 */
void drawString(int y, int x, int color, char string[16]) {
	unsigned int i = 0;
	for (i; i < 16; i++) {
		drawLetter(y, x - (8 * i), color, string[i]);
	}
}

void drawNumber(int y, int x, int color, int number) {
	char letterArray[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Initialize to 0.

	if (number == 0) {
		letterArray[0] = 0x60;
		letterArray[1] = 0x4C;
		letterArray[2] = 0x8C;
		letterArray[3] = 0x94;
		letterArray[4] = 0xA4;
		letterArray[5] = 0xC4;
		letterArray[6] = 0xC8;
		letterArray[7] = 0x30;
	}

	else {
		/**
		 * SWITCH (NUMBER)
		 *
		 * 	Switch will take in a number and create it in a hex array.
		 */
		switch (number) {
		case 0:
			letterArray[0] = 0x60;
			letterArray[1] = 0x4C;
			letterArray[2] = 0x8C;
			letterArray[3] = 0x94;
			letterArray[4] = 0xA4;
			letterArray[5] = 0xC4;
			letterArray[6] = 0xC8;
			letterArray[7] = 0x30;
			break;
		case 1:
			letterArray[0] = 0x38;
			letterArray[1] = 0x78;
			letterArray[2] = 0x68;
			letterArray[3] = 0x18;
			letterArray[4] = 0x18;
			letterArray[5] = 0x18;
			letterArray[6] = 0x18;
			letterArray[7] = 0xFC;
			break;
		case 2:
			letterArray[0] = 0x40;
			letterArray[1] = 0xEC;
			letterArray[2] = 0xCC;
			letterArray[3] = 0x18;
			letterArray[4] = 0x30;
			letterArray[5] = 0x60;
			letterArray[6] = 0xC0;
			letterArray[7] = 0xFC;
			break;
		case 3:
			letterArray[0] = 0x58;
			letterArray[1] = 0xFC;
			letterArray[2] = 0x8C;
			letterArray[3] = 0x38;
			letterArray[4] = 0x38;
			letterArray[5] = 0x8C;
			letterArray[6] = 0xFC;
			letterArray[7] = 0x58;
			break;
		case 4:
			letterArray[0] = 0xD8;
			letterArray[1] = 0xD8;
			letterArray[2] = 0xD8;
			letterArray[3] = 0xFC;
			letterArray[4] = 0xFC;
			letterArray[5] = 0x18;
			letterArray[6] = 0x18;
			letterArray[7] = 0x18;
			break;
		case 5:
			letterArray[0] = 0xFC;
			letterArray[1] = 0x80;
			letterArray[2] = 0xB8;
			letterArray[3] = 0xCC;
			letterArray[4] = 0x04;
			letterArray[5] = 0x04;
			letterArray[6] = 0x88;
			letterArray[7] = 0x70;
			break;
		case 6:
			letterArray[0] = 0x38;
			letterArray[1] = 0x44;
			letterArray[2] = 0x80;
			letterArray[3] = 0xB8;
			letterArray[4] = 0xC4;
			letterArray[5] = 0x84;
			letterArray[6] = 0x44;
			letterArray[7] = 0x38;
			break;
		case 7:
			letterArray[0] = 0xFC;
			letterArray[1] = 0x84;
			letterArray[2] = 0x0C;
			letterArray[3] = 0x18;
			letterArray[4] = 0xFC;
			letterArray[5] = 0x60;
			letterArray[6] = 0xC0;
			letterArray[7] = 0x80;
			break;
		case 8:
			letterArray[0] = 0x70;
			letterArray[1] = 0x88;
			letterArray[2] = 0x88;
			letterArray[3] = 0x70;
			letterArray[4] = 0xF8;
			letterArray[5] = 0x88;
			letterArray[6] = 0x88;
			letterArray[7] = 0x70;
			break;
		case 9:
			letterArray[0] = 0x74;
			letterArray[1] = 0x86;
			letterArray[2] = 0x86;
			letterArray[3] = 0xFC;
			letterArray[4] = 0x0C;
			letterArray[5] = 0x0C;
			letterArray[6] = 0x0C;
			letterArray[7] = 0x0C;
			break;
// As a default, I am setting it to display all black.
// Why?  Because then I will be sure as to what is not working
// properly.
		default:
			letterArray[0] = 0xFF;
			letterArray[1] = 0xFF;
			letterArray[2] = 0xFF;
			letterArray[3] = 0xFF;
			letterArray[4] = 0xFF;
			letterArray[5] = 0xFF;
			letterArray[6] = 0xFF;
			letterArray[7] = 0xFF;
			break;
		}
	}

	/**
	 * 	For this part, we have a double nested for-loop, and it adds
	 * up to 64 pixels (8x8)
	 *
	 * IF the letter array is a 1, then output a pixel.  Else, do nothing
	 *
	 * Lastly, left shift the letter array.
	 */
	unsigned int i = 0;

	unsigned int j = 0;
	for (i; i < 8; i++)	// Loop through all memory locations 16 bit color
			{
		for (j = 8; j > 0; j--) {
			if ((letterArray[i] & 0x80) == 0x80) {
				drawPixel(i + y, j + x, color);
			} else  // White pixel
			{
				drawPixel(i + y, j + x, 0xFFFF);
			}
			letterArray[i] = letterArray[i] << 1;
		}
	}
}

#endif