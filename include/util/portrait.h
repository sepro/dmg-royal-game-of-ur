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

#endif // PORTRAIT_H
