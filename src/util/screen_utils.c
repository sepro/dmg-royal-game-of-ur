/**
 * screen_utils.c
 * Screen utility functions implementation
 */

#include "util/screen_utils.h"

const uint8_t white_tile[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void fill_screen_with_tile(uint8_t tile_index) {
    uint8_t row[20];
    for (uint8_t i = 0; i < 20; i++) {
        row[i] = tile_index;
    }
    for (uint8_t y = 0; y < 18; y++) {
        set_bkg_tiles(0, y, 20, 1, row);
    }
}

void draw_border_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                       uint8_t border_tile_start, const uint8_t* border_map) {
    // Draw border frame using tilemap (skip inner 5x5 area)
    for (uint8_t row = 0; row < height; row++) {
        for (uint8_t col = 0; col < width; col++) {
            // Skip inner 5x5 portrait/coin area (rows 1-5, cols 1-5)
            if (row >= 1 && row <= 5 && col >= 1 && col <= 5) continue;
            uint8_t tile = border_tile_start + border_map[row * width + col];
            set_bkg_tile_xy(x + col, y + row, tile);
        }
    }
}

void clear_border_frame(uint8_t x, uint8_t y, uint8_t width, uint8_t height,
                        uint8_t clear_tile) {
    // Clear border frame (skip inner 5x5 area)
    for (uint8_t row = 0; row < height; row++) {
        for (uint8_t col = 0; col < width; col++) {
            // Skip inner 5x5 portrait/coin area (rows 1-5, cols 1-5)
            if (row >= 1 && row <= 5 && col >= 1 && col <= 5) continue;
            set_bkg_tile_xy(x + col, y + row, clear_tile);
        }
    }
}

void clear_rect(uint8_t tile, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    uint8_t row[20];
    uint8_t i;
    for (i = 0; i < w && i < 20; i++) {
        row[i] = tile;
    }
    for (i = 0; i < h; i++) {
        set_bkg_tiles(x, y + i, w, 1, row);
    }
}

void draw_tile_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile_base) {
    uint8_t row_buf[20];
    uint8_t row, col;
    for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
            row_buf[col] = tile_base + (row * w) + col;
        }
        set_bkg_tiles(x, y + row, w, 1, row_buf);
    }
}
