/**
 * portrait.c
 * Shared opponent portrait drawing using merged tileset
 * All 4 characters x 3 expressions share deduplicated tiles
 */

#include <gb/gb.h>
#include <stdint.h>
#include "util/portrait.h"

// External references to merged profile asset
extern const uint8_t profiles_merged_tiles[];
extern const unsigned char profiles_merged_map[];

// Remap table for per-character tile loading
// Maps global tile index -> local index (0xFF = not loaded)
static uint8_t tile_remap[PORTRAIT_TILE_COUNT];

/**
 * Get pointer to start of sub-map for a character + expression
 * Returns offset into profiles_merged_map for the first row
 */
static uint16_t get_submap_offset(uint8_t char_idx, uint8_t expression) {
    return (uint16_t)(char_idx * PORTRAIT_SUB_HEIGHT) * PORTRAIT_MAP_WIDTH
         + (uint16_t)(expression * PORTRAIT_SUB_WIDTH);
}

/**
 * Load full merged tileset into VRAM
 */
void load_portrait_tiles(uint8_t tile_base) {
    set_bkg_data(tile_base, PORTRAIT_TILE_COUNT, profiles_merged_tiles);
}

/**
 * Load only tiles needed by one character (all 3 expressions)
 * Builds remap table and loads individual tiles
 */
uint8_t load_portrait_tiles_for_char(uint8_t char_idx, uint8_t tile_base) {
    uint8_t loaded = 0;
    uint8_t i, row, col, expr;
    uint8_t tile_idx;

    // Clear remap table
    for (i = 0; i < PORTRAIT_TILE_COUNT; i++) {
        tile_remap[i] = 0xFF;
    }

    // Scan all 3 expression sub-maps for this character
    for (expr = 0; expr < 3; expr++) {
        uint16_t offset = get_submap_offset(char_idx, expr);
        for (row = 0; row < PORTRAIT_SUB_HEIGHT; row++) {
            for (col = 0; col < PORTRAIT_SUB_WIDTH; col++) {
                tile_idx = profiles_merged_map[offset + (uint16_t)row * PORTRAIT_MAP_WIDTH + col];
                if (tile_idx < PORTRAIT_TILE_COUNT && tile_remap[tile_idx] == 0xFF) {
                    // New tile - load it and assign a local index
                    tile_remap[tile_idx] = loaded;
                    set_bkg_data(tile_base + loaded, 1,
                                 &profiles_merged_tiles[(uint16_t)tile_idx * 16]);
                    loaded++;
                }
            }
        }
    }

    return loaded;
}

/**
 * Draw a portrait expression from the merged tilemap
 */
void draw_portrait_expr(uint8_t char_idx, uint8_t expression,
                        uint8_t tile_base, uint8_t x, uint8_t y,
                        uint8_t use_remap) {
    uint16_t offset = get_submap_offset(char_idx, expression);
    uint8_t row_buf[PORTRAIT_SUB_WIDTH];
    uint8_t row, col;

    for (row = 0; row < PORTRAIT_SUB_HEIGHT; row++) {
        for (col = 0; col < PORTRAIT_SUB_WIDTH; col++) {
            uint8_t tile_idx = profiles_merged_map[offset + (uint16_t)row * PORTRAIT_MAP_WIDTH + col];
            if (use_remap) {
                row_buf[col] = tile_base + tile_remap[tile_idx];
            } else {
                row_buf[col] = tile_base + tile_idx;
            }
        }
        set_bkg_tiles(x, y + row, PORTRAIT_SUB_WIDTH, 1, row_buf);
    }
}

/**
 * Convenience: load full tileset + draw normal expression
 */
void draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y) {
    load_portrait_tiles(tile_base);
    draw_portrait_expr(opponent_idx, PORTRAIT_EXPR_NORMAL, tile_base, x, y, 0);
}


/**
 * Draw a mirrored portrait expression from the merged tilemap
 */
void draw_portrait_expr_mirrored(uint8_t char_idx, uint8_t expression,
                                 uint8_t tile_base, uint8_t x, uint8_t y,
                                 uint8_t use_remap) {
    uint16_t offset = get_submap_offset(char_idx, expression);
    uint8_t row_buf[PORTRAIT_SUB_WIDTH];
    uint8_t row, col;

    for (row = 0; row < PORTRAIT_SUB_HEIGHT; row++) {
        for (col = 0; col < PORTRAIT_SUB_WIDTH; col++) {
            uint8_t src_col = (uint8_t)(PORTRAIT_SUB_WIDTH - 1 - col);
            uint8_t tile_idx = profiles_merged_map[offset + (uint16_t)row * PORTRAIT_MAP_WIDTH + src_col];
            if (use_remap) {
                row_buf[col] = tile_base + tile_remap[tile_idx];
            } else {
                row_buf[col] = tile_base + tile_idx;
            }
        }
        set_bkg_tiles(x, y + row, PORTRAIT_SUB_WIDTH, 1, row_buf);
    }
}

uint8_t get_portrait_expr_tile(uint8_t char_idx, uint8_t expression,
                               uint8_t tile_base, uint8_t row, uint8_t col,
                               uint8_t use_remap) {
    uint16_t offset;
    uint8_t tile_idx;

    offset = get_submap_offset(char_idx, expression);
    tile_idx = profiles_merged_map[offset + (uint16_t)row * PORTRAIT_MAP_WIDTH + col];

    if (use_remap) {
        return tile_base + tile_remap[tile_idx];
    }

    return tile_base + tile_idx;
}
