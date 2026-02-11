/**
 * screen_utils.h
 * Screen utility functions
 */

#ifndef SCREEN_UTILS_H
#define SCREEN_UTILS_H

#include <gb/gb.h>
#include <stdint.h>

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

#endif // SCREEN_UTILS_H
