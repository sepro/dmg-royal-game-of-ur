/**
 * screen_utils.h
 * Screen utility functions
 */

#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <gb/gb.h>
#include <stdint.h>

/** Blank tile data (8x8 pixels, all color 0 = white on DMG) */
extern const uint8_t white_tile[16];

/**
 * Fill the entire screen with a single tile
 * @param tile_index The tile index to fill the screen with
 */
void fill_screen_with_tile(uint8_t tile_index);

/**
 * Draw a border frame using a tilemap (skips inner 5x5 area)
 * @param x X position of the border's top-left corner
 * @param y Y position of the border's top-left corner
 * @param width Width of the border frame
 * @param height Height of the border frame
 * @param border_tile_start Starting tile index for border tiles
 * @param border_map Array mapping border positions to tile offsets
 */
void draw_border_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                       uint8_t border_tile_start, const uint8_t* border_map);

/**
 * Clear a border frame (skips inner 5x5 area)
 * @param x X position of the border's top-left corner
 * @param y Y position of the border's top-left corner
 * @param width Width of the border frame
 * @param height Height of the border frame
 * @param clear_tile Tile index to use for clearing
 */
void clear_border_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        uint8_t clear_tile);

/**
 * Clear a rectangular area with a given tile
 * @param tile Tile index to fill with
 * @param x Tile X position
 * @param y Tile Y position
 * @param w Width in tiles (max 20)
 * @param h Height in tiles
 */
void clear_rect(uint8_t tile, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * Draw a rectangular area of sequentially-numbered tiles
 * Tile at (col,row) = tile_base + row * w + col
 * @param x Tile X position
 * @param y Tile Y position
 * @param w Width in tiles
 * @param h Height in tiles
 * @param tile_base Starting VRAM tile index
 */
void draw_tile_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile_base);

/**
 * Draw "WAITING" followed by 0-3 animated dots at the given tile position.
 * Clears the row first (16 tiles wide), then draws "WAITING" + dot_count dots.
 * @param x Tile X position
 * @param y Tile Y position
 * @param dot_count Number of dots to draw (0-3)
 */
void draw_waiting_text(uint8_t x, uint8_t y, uint8_t dot_count);

/**
 * Draw a full-screen-width (20 tile) border box using the standard border tile layout.
 * Top/middle/bottom rows use offset pattern: 0x00 corner, 0x01-0x05 top edge, 0x06 corner,
 * 0x07/0x08/0x09 sides, 0x12-0x18 bottom.
 * @param tile_start VRAM index of the first border tile
 * @param y_start Top tile row of the border
 * @param rows Total height in tile rows (must be >= 2)
 * @param use_window 0 = draw to background layer, non-zero = draw to window layer
 */
void draw_full_width_border(uint8_t tile_start, uint8_t y_start, uint8_t rows, uint8_t use_window);

#endif // SCREEN_UTILS_H
