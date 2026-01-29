/**
 * game.h
 * Game board display and main gameplay state
 */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "vram_layout.h"

// Board layout constants (tile coordinates)
// Board is 20x10 tiles (160x80 pixels), top-justified
#define BOARD_X         0
#define BOARD_Y         0
#define BOARD_WIDTH     20
#define BOARD_HEIGHT    10

// UI area (below board)
#define UI_START_Y      10
#define UI_END_Y        17

// UI text positions (tile coordinates)
#define GAME_TURN_X     1
#define GAME_TURN_Y     12
#define GAME_PROMPT_X   1
#define GAME_PROMPT_Y   14

// VRAM tile indices (screen-isolated: 0-38 for board)
// Board uses tiles 0-37 (38 unique tiles from png2asset)
#define GAME_BOARD_TILE_START   0
#define GAME_BOARD_TILE_COUNT   38

// Background tile for UI area (use board's background tile)
#define GAME_BG_TILE            0

/**
 * Initialize game screen
 * Loads board tiles, draws board, sets up UI area
 */
void init_game(void);

/**
 * Update game screen (called every frame)
 * Handles input, game logic, AI turns
 */
void update_game(void);

/**
 * Cleanup game screen
 * Called when transitioning to another state
 */
void cleanup_game(void);

#endif // GAME_H
