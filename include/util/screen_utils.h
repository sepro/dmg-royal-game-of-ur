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

#endif // SCREEN_UTILS_H
