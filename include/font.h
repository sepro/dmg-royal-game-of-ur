/**
 * font.h
 * Simple 8x8 font for menu text
 */

#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Font configuration
#define FONT_TILE_START 140  // Start loading font tiles after title tiles (139 used)
#define FONT_CHAR_COUNT 28   // Number of characters in font (26 letters + space + blank)

// Character indices (offset from FONT_TILE_START)
#define CHAR_SPACE 0
#define CHAR_A 1
#define CHAR_B 2
#define CHAR_C 3
#define CHAR_D 4
#define CHAR_E 5
#define CHAR_F 6
#define CHAR_G 7
#define CHAR_H 8
#define CHAR_I 9
#define CHAR_J 10
#define CHAR_K 11
#define CHAR_L 12
#define CHAR_M 13
#define CHAR_N 14
#define CHAR_O 15
#define CHAR_P 16
#define CHAR_Q 17
#define CHAR_R 18
#define CHAR_S 19
#define CHAR_T 20
#define CHAR_U 21
#define CHAR_V 22
#define CHAR_W 23
#define CHAR_X 24
#define CHAR_Y 25
#define CHAR_Z 26
#define CHAR_BLANK 27  // Solid black tile for text background

// Font tile data (defined in font.c)
extern const uint8_t font_tiles[];

/**
 * Load font tiles into VRAM
 * Call this after loading background tiles
 */
void load_font(void);

/**
 * Draw a string at the specified tile position
 * Only supports characters in the font (uppercase subset)
 * @param x Tile X position (0-19)
 * @param y Tile Y position (0-17)
 * @param str String to draw (uppercase, limited charset)
 */
void draw_text(uint8_t x, uint8_t y, const char *str);

/**
 * Clear a row of tiles with black background
 * @param x Starting tile X position
 * @param y Tile Y position
 * @param width Number of tiles to clear
 */
void clear_text_row(uint8_t x, uint8_t y, uint8_t width);

#endif // FONT_H
