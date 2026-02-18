/**
 * screen_utils.c
 * Screen utility functions implementation
 */

#include "util/screen_utils.h"
#include "util/font.h"

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

void draw_waiting_text(uint8_t x, uint8_t y, uint8_t dot_count) {
    uint8_t i;
    clear_text_row_inverted(x, y, 16);
    draw_text_inverted(x, y, "WAITING");
    for (i = 0; i < dot_count; i++) {
        draw_text_inverted((uint8_t)(x + 7 + i), y, ".");
    }
}

void draw_full_width_border(uint8_t tile_start, uint8_t y_start, uint8_t rows, uint8_t use_window) {
    uint8_t row_buf[20];
    uint8_t x;

    // Top edge: corner + repeating top tiles + corner
    row_buf[0] = (uint8_t)(tile_start + 0x00);
    for (x = 1; x < 19; x++) {
        row_buf[x] = (uint8_t)(tile_start + 0x01 + ((x - 1) % 5));
    }
    row_buf[19] = (uint8_t)(tile_start + 0x06);
    if (use_window) {
        set_win_tiles(0, y_start, 20, 1, row_buf);
    } else {
        set_bkg_tiles(0, y_start, 20, 1, row_buf);
    }

    // Middle rows: left edge + fill + right edge
    row_buf[0] = (uint8_t)(tile_start + 0x07);
    for (x = 1; x < 19; x++) {
        row_buf[x] = (uint8_t)(tile_start + 0x08);
    }
    row_buf[19] = (uint8_t)(tile_start + 0x09);
    for (uint8_t y = (uint8_t)(y_start + 1); y < (uint8_t)(y_start + rows - 1); y++) {
        if (use_window) {
            set_win_tiles(0, y, 20, 1, row_buf);
        } else {
            set_bkg_tiles(0, y, 20, 1, row_buf);
        }
    }

    // Bottom edge: corner + repeating bottom tiles + corner
    row_buf[0] = (uint8_t)(tile_start + 0x12);
    for (x = 1; x < 19; x++) {
        row_buf[x] = (uint8_t)(tile_start + 0x13 + ((x - 1) % 5));
    }
    row_buf[19] = (uint8_t)(tile_start + 0x18);
    if (use_window) {
        set_win_tiles(0, (uint8_t)(y_start + rows - 1), 20, 1, row_buf);
    } else {
        set_bkg_tiles(0, (uint8_t)(y_start + rows - 1), 20, 1, row_buf);
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
