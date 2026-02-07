/**
 * portrait.c
 * Shared opponent portrait drawing functionality
 */

#include <gb/gb.h>
#include <stdint.h>
#include "util/portrait.h"
#include "util/opponent_data.h"

// External references to generated profile assets
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
extern const uint8_t profile_02_tiles[];
extern const unsigned char profile_02_map[];
extern const uint8_t profile_03_tiles[];
extern const unsigned char profile_03_map[];
extern const uint8_t profile_04_tiles[];
extern const unsigned char profile_04_map[];

// Portrait dimensions (all portraits are 5x5 tiles)
#define PORTRAIT_WIDTH 5
#define PORTRAIT_HEIGHT 5

/**
 * Draw opponent portrait at specified position
 */
void draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y) {
    const uint8_t *tiles;
    const unsigned char *map;
    uint8_t tile_count;

    // Select the correct profile based on opponent_idx
    switch (opponent_idx) {
        case 0:
            tiles = profile_01_tiles;
            map = profile_01_map;
            tile_count = profile_tile_counts[0];
            break;
        case 1:
            tiles = profile_02_tiles;
            map = profile_02_map;
            tile_count = profile_tile_counts[1];
            break;
        case 2:
            tiles = profile_03_tiles;
            map = profile_03_map;
            tile_count = profile_tile_counts[2];
            break;
        case 3:
            tiles = profile_04_tiles;
            map = profile_04_map;
            tile_count = profile_tile_counts[3];
            break;
        default:
            return;
    }

    // Load portrait tiles at specified VRAM location
    set_bkg_data(tile_base, tile_count, tiles);

    // Draw 5x5 portrait using tilemap
    uint8_t row_buf[PORTRAIT_WIDTH];
    for (uint8_t row = 0; row < PORTRAIT_HEIGHT; row++) {
        for (uint8_t col = 0; col < PORTRAIT_WIDTH; col++) {
            row_buf[col] = tile_base + map[row * PORTRAIT_WIDTH + col];
        }
        set_bkg_tiles(x, y + row, PORTRAIT_WIDTH, 1, row_buf);
    }
}
