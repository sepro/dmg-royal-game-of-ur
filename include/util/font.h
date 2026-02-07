/**
 * font.h
 * Simple 8x8 font for menu text
 */

#ifndef FONT_H
#define FONT_H

#include <stdint.h>
#include "vram_layout.h"  // See vram_layout.h for global allocation map

// Font configuration
#define FONT_TILE_START VRAM_FONT_START
#define FONT_CHAR_COUNT VRAM_FONT_COUNT

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
#define CHAR_WHITE 28  // Solid white tile for inverted text background
#define CHAR_0 29
#define CHAR_1 30
#define CHAR_2 31
#define CHAR_3 32
#define CHAR_4 33
#define CHAR_5 34
#define CHAR_6 35
#define CHAR_7 36
#define CHAR_8 37
#define CHAR_9 38
#define CHAR_COLON 39  // Colon character for "R:7 F:0" display

// Inverted font configuration (black text on white background)
#define FONT_INVERTED_TILE_START VRAM_FONT_INVERTED_START
#define FONT_INVERTED_CHAR_COUNT VRAM_FONT_INVERTED_COUNT

// Font tile data (defined in font.c)
extern const uint8_t font_tiles[];

/**
 * Load font tiles into VRAM
 * Call this after loading background tiles
 */
void load_font(void);

/**
 * Load inverted font tiles into VRAM (black text on white background)
 * For use on white background screens
 *
 * Generates inverted tiles at runtime by XOR'ing normal font with 0xFF
 * This saves ~464 bytes of ROM compared to storing pre-inverted data
 */
void load_font_inverted(void);

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

/**
 * Draw a string using inverted font (black text on white background)
 * Uses FONT_INVERTED_TILE_START as base
 * @param x Tile X position (0-19)
 * @param y Tile Y position (0-17)
 * @param str String to draw (uppercase, limited charset)
 */
void draw_text_inverted(uint8_t x, uint8_t y, const char *str);

/**
 * Clear a row of tiles with white background (for inverted text screens)
 * @param x Starting tile X position
 * @param y Tile Y position
 * @param width Number of tiles to clear
 */
void clear_text_row_inverted(uint8_t x, uint8_t y, uint8_t width);

/**
 * Draw a string using inverted font to either background or window layer
 * @param x Tile X position (0-19)
 * @param y Tile Y position (0-17 for BKG, 0-31 for WIN)
 * @param str String to draw (uppercase, limited charset)
 * @param use_window 0 = background layer, 1 = window layer
 */
void draw_text_at(uint8_t x, uint8_t y, const char *str, uint8_t use_window);

/**
 * Load border tiles inverted (XOR with 0xFF) into VRAM
 * Used for dark background screens (opponent select) where border
 * needs to be light-on-dark instead of dark-on-light
 * @param vram_start Starting VRAM tile index to load inverted tiles into
 */
void load_border_inverted(uint8_t vram_start);

#endif // FONT_H
