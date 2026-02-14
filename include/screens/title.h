/**
 * title.h
 * Title screen state management
 */

#ifndef TITLE_H
#define TITLE_H

#include <stdint.h>
#include "vram_layout.h"

// Title screen feature flags
#define TITLE_ENABLE_FALLING_PIECES 1

#if TITLE_ENABLE_FALLING_PIECES
// Falling piece animation (title screen)
#define FALLING_PIECE_SPRITE_INDEX 2      // Base OAM index (uses 4 sprites: 2-5)
#define FALLING_PIECE_SPAWN_DELAY_MIN 25  // Min frames between spawns (~0.4s)
#define FALLING_PIECE_SPAWN_DELAY_MAX 80  // Max frames between spawns (~1.3s)
#define FALLING_PIECE_MIN_X 12            // Keep inside visible area
#define FALLING_PIECE_MAX_X 152
#define FALLING_PIECE_START_Y_FP -3072    // -12 px in 8.8 fixed-point
#define FALLING_PIECE_DESPAWN_Y 144       // Remove when fully below 144px screen
#define FALLING_PIECE_INITIAL_VY_FP 32    // 0.125 px/frame in 8.8 fixed-point
#define FALLING_PIECE_ACCEL_FP 5          // 0.02 px/frame^2 in 8.8 fixed-point
#define FALLING_PIECE_WHITE_TILE_START VRAM_SPRITE_GAME_START
#define FALLING_PIECE_BLACK_TILE_START (VRAM_SPRITE_GAME_START + 4)
#endif

/**
 * Initialize title screen state
 * Loads background tiles and map, sets up arrow sprite
 */
void init_title(void);

/**
 * Update title screen state (called every frame)
 * Handles input and arrow movement
 */
void update_title(void);

/**
 * Cleanup title screen state
 * Called when transitioning to another state
 */
void cleanup_title(void);

#endif // TITLE_H
