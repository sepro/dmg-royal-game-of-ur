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

// VRAM tile indices (screen-isolated: 0-38 for board)
// Board uses tiles 0-37 (38 unique tiles from png2asset)
#define GAME_BOARD_TILE_START   0
#define GAME_BOARD_TILE_COUNT   38

// Background tile for UI area (use board's background tile)
#define GAME_BG_TILE            0

// ============================================================================
// Game Phase Constants
// ============================================================================
#define PHASE_WAIT_ROLL     0   // Waiting for player to press A to roll
#define PHASE_ROLLING       1   // Dice animation in progress
#define PHASE_SHOW_RESULT   2   // Showing dice result briefly
#define PHASE_SELECT_MOVE   3   // Player selecting which piece to move (Phase 8)
#define PHASE_CPU_THINK     4   // CPU calculating move
#define PHASE_ANIMATE_MOVE  5   // Piece movement animation (Phase 8)

// ============================================================================
// Sprite Tile Allocation (0-255 in sprite VRAM)
// ============================================================================
// Existing (from vram_layout.h):
// - Tile 0: Arrow cursor
// - Tiles 1-4: Blink animation

// New for game screen:
#define SPRITE_PIECE_WHITE      5   // White piece indicator (8x8)
#define SPRITE_PIECE_BLACK      6   // Black piece indicator (8x8)
#define SPRITE_DICE_WHITE       7   // White die face (8x8)
#define SPRITE_DICE_BLACK       8   // Black die face (8x8)

// ============================================================================
// OAM Sprite Indices (0-39 sprites available)
// ============================================================================
#define OAM_CPU_PIECE       0   // CPU player's piece indicator
#define OAM_HUMAN_PIECE     1   // Human player's piece indicator
#define OAM_DICE_0          2   // First die
#define OAM_DICE_1          3   // Second die
#define OAM_DICE_2          4   // Third die
#define OAM_DICE_3          5   // Fourth die

// ============================================================================
// UI Layout Constants (tile coordinates)
// ============================================================================
// Row 11: CPU player info
#define UI_CPU_LABEL_X      1   // "CPU"
#define UI_CPU_LABEL_Y      11
#define UI_CPU_RESERVE_X    6   // "R:7"
#define UI_CPU_RESERVE_Y    11
#define UI_CPU_FINISH_X     10  // "F:0"
#define UI_CPU_FINISH_Y     11

// Row 13: Human player info
#define UI_HUMAN_LABEL_X    1   // "YOU"
#define UI_HUMAN_LABEL_Y    13
#define UI_HUMAN_RESERVE_X  6   // "R:7"
#define UI_HUMAN_RESERVE_Y  13
#define UI_HUMAN_FINISH_X   10  // "F:0"
#define UI_HUMAN_FINISH_Y   13

// Row 15: Turn indicator
#define UI_TURN_X           1
#define UI_TURN_Y           15

// Row 16: Action prompt / roll result
#define UI_PROMPT_X         1
#define UI_PROMPT_Y         16

// ============================================================================
// Sprite Pixel Positions (for piece indicators in UI)
// ============================================================================
// Note: Sprite X/Y positions are offset by 8/16 pixels on Game Boy
// Pixel position = tile * 8 + 8 for X, tile * 8 + 16 for Y
#define PIECE_SPRITE_X      (5 * 8 + 8)   // After "CPU:" text (tile 5)
#define CPU_PIECE_SPRITE_Y  (11 * 8 + 16) // Row 11
#define HUMAN_PIECE_SPRITE_Y (13 * 8 + 16) // Row 13

// ============================================================================
// Dice Sprite Positions (centered in UI area)
// ============================================================================
#define DICE_Y              (12 * 8 + 16) // Row 12 (between CPU and human info)
#define DICE_START_X        (6 * 8 + 8)   // Starting X for first die
#define DICE_SPACING        12            // Pixels between dice centers

// ============================================================================
// Animation Timing
// ============================================================================
#define ROLL_ANIM_DURATION  60  // ~1 second at 60fps
#define ROLL_UPDATE_INTERVAL 4  // Update dice every 4 frames
#define RESULT_PAUSE_FRAMES 30  // Show result for half second

// ============================================================================
// Game Constants
// ============================================================================
#define PIECES_PER_PLAYER   7   // Each player has 7 pieces
#define NUM_DICE            4   // 4 binary dice

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
