/**
 * portrait.h
 * Shared opponent portrait drawing functionality
 */

#ifndef PORTRAIT_H
#define PORTRAIT_H

#include <stdint.h>

/**
 * Draw opponent portrait at specified position
 *
 * @param opponent_idx Opponent index (0-3)
 * @param tile_base VRAM tile index to load portrait tiles
 * @param x X position in tiles (0-19)
 * @param y Y position in tiles (0-17)
 */
void draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y);

/**
 * Draw sad opponent portrait (loads tiles + tilemap)
 */
void draw_portrait_sad(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y);

/**
 * Redraw portrait tilemap only (fast path for animation toggling)
 *
 * @param use_sad 0 = normal map, 1 = sad map
 */
void redraw_portrait_map(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y, uint8_t use_sad);

#endif // PORTRAIT_H
