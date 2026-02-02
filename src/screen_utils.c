/**
 * screen_utils.c
 * Screen utility functions implementation
 */

#include "screen_utils.h"

void fill_screen_with_tile(uint8_t tile_index) {
    uint8_t row[20];
    for (uint8_t i = 0; i < 20; i++) {
        row[i] = tile_index;
    }
    for (uint8_t y = 0; y < 18; y++) {
        set_bkg_tiles(0, y, 20, 1, row);
    }
}
