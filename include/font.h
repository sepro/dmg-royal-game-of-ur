/**
 * font.h
 * Simple 8x8 font for menu text
 */

#ifndef FONT_H
#define FONT_H

#include <stdint.h>

// Font configuration
#define FONT_TILE_START 140  // Start loading font tiles after title tiles (139 used)
#define FONT_CHAR_COUNT 15   // Number of characters in font

// Character indices (offset from FONT_TILE_START)
#define CHAR_SPACE 0
#define CHAR_A 1
#define CHAR_B 2
#define CHAR_C 3
#define CHAR_E 4
#define CHAR_G 5
#define CHAR_I 6
#define CHAR_K 7
#define CHAR_L 8
#define CHAR_M 9
#define CHAR_N 10
#define CHAR_R 11
#define CHAR_S 12
#define CHAR_T 13
#define CHAR_BLANK 14  // Solid black tile for text background

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
