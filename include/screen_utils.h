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

#endif // SCREEN_UTILS_H
