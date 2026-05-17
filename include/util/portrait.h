/**
 * portrait.h
 * Shared opponent portrait drawing functionality
 * Uses merged tileset with all 4 characters x 3 expressions
 */

#ifndef PORTRAIT_H
#define PORTRAIT_H

#include <stdint.h>

// Expression indices (columns in merged image)
#define PORTRAIT_EXPR_NORMAL 0
#define PORTRAIT_EXPR_SAD    1
#define PORTRAIT_EXPR_HAPPY  2

// Portrait dimensions (all portraits are 5x5 tiles)
#define PORTRAIT_MAP_WIDTH  15  // Full merged map width in tiles
#define PORTRAIT_SUB_WIDTH  5   // Single portrait width
#define PORTRAIT_SUB_HEIGHT 5   // Single portrait height
#define PORTRAIT_TILE_COUNT 117 // Total unique tiles in merged tileset

// CGB BG palette index used for portraits. Palette 0 is grayscale (everything
// else). Palette 1 is white / light beige / dark brown / black, installed
// once at boot in main.c.
#define PORTRAIT_CGB_PALETTE 1

/**
 * CGB-only: zero the entire 32x32 BG tile-attribute plane so every tile uses
 * palette 0 with no flip and no priority. No-op on DMG. Intended to be called
 * once per state transition before the next screen draws its tiles, so leftover
 * palette assignments from the previous screen don't bleed through.
 */
void clear_bg_attributes(void);

/**
 * CGB-only: derive a 4-color CGB sprite palette from the current OBP0_REG value
 * and install it as CGB sprite palette 0. On DMG the OBP0_REG byte remaps the
 * tile pixel shades 0-3 to output shades 0-3; on CGB in CGB mode the OBP
 * register is ignored and shade N indexes CGB sprite palette[N] directly. This
 * helper bridges that gap by translating the OBP0_REG byte into the equivalent
 * CGB palette so sprite shades render the same on CGB as on DMG. Call once
 * immediately after every `OBP0_REG = ...` assignment. No-op on DMG.
 */
void sync_sprite_palette_to_obp0(void);

/**
 * Load full merged tileset into VRAM at tile_base.
 * For screens with ample VRAM (opponent_select, difficulty_select, endgame, link_profile).
 */
void load_portrait_tiles(uint8_t tile_base);

/**
 * Load only tiles needed by one character's 3 expressions.
 * Builds internal remap table for use with draw_portrait_expr(..., use_remap=1).
 * Returns number of unique tiles loaded.
 */
uint8_t load_portrait_tiles_for_char(uint8_t char_idx, uint8_t tile_base);

/**
 * Draw a portrait expression using the merged tilemap.
 *
 * @param char_idx   Character index (0-3)
 * @param expression PORTRAIT_EXPR_NORMAL/SAD/HAPPY
 * @param tile_base  VRAM tile index where tiles were loaded
 * @param x          Background X position in tiles
 * @param y          Background Y position in tiles
 * @param use_remap  0 = full tileset loaded (global indices),
 *                   1 = per-char tileset loaded (remapped indices)
 */
void draw_portrait_expr(uint8_t char_idx, uint8_t expression,
                        uint8_t tile_base, uint8_t x, uint8_t y,
                        uint8_t use_remap);


/**
 * Draw a horizontally mirrored portrait expression.
 * Expects mirrored tile graphics to be present at tile_base.
 *
 * @param char_idx   Character index (0-3)
 * @param expression PORTRAIT_EXPR_NORMAL/SAD/HAPPY
 * @param tile_base  VRAM tile index where mirrored tiles were loaded
 * @param x          Background X position in tiles
 * @param y          Background Y position in tiles
 * @param use_remap  0 = full tileset loaded (global indices),
 *                   1 = per-char tileset loaded (remapped indices)
 */
void draw_portrait_expr_mirrored(uint8_t char_idx, uint8_t expression,
                                 uint8_t tile_base, uint8_t x, uint8_t y,
                                 uint8_t use_remap);

/**
 * Resolve one portrait tile index from the merged tilemap.
 *
 * @param char_idx   Character index (0-3)
 * @param expression PORTRAIT_EXPR_NORMAL/SAD/HAPPY
 * @param tile_base  VRAM tile index where tiles were loaded
 * @param row        Portrait row (0-4)
 * @param col        Portrait column (0-4)
 * @param use_remap  0 = full tileset loaded (global indices),
 *                   1 = per-char tileset loaded (remapped indices)
 * @return Background tile index for the requested portrait tile
 */
uint8_t get_portrait_expr_tile(uint8_t char_idx, uint8_t expression,
                               uint8_t tile_base, uint8_t row, uint8_t col,
                               uint8_t use_remap);

/**
 * Convenience: load full tileset + draw normal expression.
 * For screens that just need a single portrait displayed.
 */
void draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y);

#endif // PORTRAIT_H
