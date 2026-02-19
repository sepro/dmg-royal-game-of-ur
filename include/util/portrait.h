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
 * Load one character's portrait tiles with a fade-to-white effect.
 *
 * @param char_idx   Character index (0-3)
 * @param tile_base  VRAM tile index where tiles are loaded
 * @param fade_level 0 = normal, 1..8 = progressively whiter (8 = full white)
 * @return Number of unique tiles loaded
 */
uint8_t load_portrait_tiles_for_char_fade(uint8_t char_idx, uint8_t tile_base,
                                          uint8_t fade_level);

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
 * Convenience: load full tileset + draw normal expression.
 * For screens that just need a single portrait displayed.
 */
void draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y);

#endif // PORTRAIT_H
