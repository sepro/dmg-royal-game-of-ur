/**
 * game.h
 * Game board display and main gameplay state
 */

#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include "game_types.h"
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
#define PHASE_ROSETTE_BONUS 6   // Extra roll after landing on rosette
#define PHASE_LINK_RECV_MOVE 7  // Waiting for remote player's move via link

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

// Phase 8c: Selection border (4 sprites for 16x16, uses flip flags)
#define OAM_SELECTION_TL    6   // Top-left corner
#define OAM_SELECTION_TR    7   // Top-right corner
#define OAM_SELECTION_BL    8   // Bottom-left corner
#define OAM_SELECTION_BR    9   // Bottom-right corner

// Phase 8c: Destination preview (4 sprites for 16x16)
#define OAM_DEST_TL         10  // Top-left
#define OAM_DEST_TR         11  // Top-right
#define OAM_DEST_BL         12  // Bottom-left
#define OAM_DEST_BR         13  // Bottom-right

// Phase 8c: Reserve piece indicator (4 sprites for 16x16)
// Lower OAM index = rendered on top, so reserve indicator is behind selection border
#define OAM_RESERVE_TL      14  // Top-left
#define OAM_RESERVE_TR      15  // Top-right
#define OAM_RESERVE_BL      16  // Bottom-left
#define OAM_RESERVE_BR      17  // Bottom-right

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
#define DEST_BLINK_INTERVAL 15  // Frames between blink toggles (~4Hz)
#define NO_MOVES_PAUSE      45  // Frames to show "NO VALID MOVES"

// ============================================================================
// Pause Screen Constants
// ============================================================================
// Window layer positioning (hardware offsets: WX has +7, WY is absolute)
#define PAUSE_WIN_X             7   // Left edge (WX minimum is 7)
#define PAUSE_WIN_Y_HIDDEN      144 // Off screen (below visible area)
#define PAUSE_WIN_Y_VISIBLE     0   // Target Y when paused (full screen)
#define PAUSE_ANIM_SPEED        8   // Pixels per frame for slide animation

// Border tile allocation (use available VRAM 222-246)
#define PAUSE_BORDER_TILE_START VRAM_PAUSE_BORDER_START
#define PAUSE_BORDER_COUNT      VRAM_PAUSE_BORDER_COUNT

// Border drawing constants
#define BORDER_WIDTH            7
#define BORDER_HEIGHT           7

// Pause window layout (tile coordinates within window)
#define PAUSE_TITLE_X           6   // "PAUSED" centered
#define PAUSE_TITLE_Y           3
#define PAUSE_TURN_X            2   // "TURN: XXX"
#define PAUSE_TURN_Y            5
#define PAUSE_TIME_X            2   // "TIME: MM:SS"
#define PAUSE_TIME_Y            6
#define PAUSE_YOU_X             2   // "YOU FINISHED: X"
#define PAUSE_YOU_Y             8
#define PAUSE_CPU_X             2   // "CPU FINISHED: X"
#define PAUSE_CPU_Y             9
#define PAUSE_VS_X              2   // "VS THE SCHOLAR"
#define PAUSE_VS_Y              11
#define PAUSE_DIFF_X            2   // "DIFFICULTY: EASY"
#define PAUSE_DIFF_Y            12
#define PAUSE_HINT_X            3   // "PRESS START"
#define PAUSE_HINT_Y            15

// Window dimensions in tiles
#define PAUSE_WIN_WIDTH         20
#define PAUSE_WIN_HEIGHT        18

// ============================================================================
// Game Constants
// ============================================================================
#define PIECES_PER_PLAYER   7   // Each player has 7 pieces (set to 1 for testing)
#define NUM_DICE            4   // 4 binary dice

// ============================================================================
// Phase 8c: Reserve and Bearoff Indicator Positions (tile coordinates)
// ============================================================================
// Reserve indicator: right of starting square (position 1)
// Human reserve: right of human start (tile 8,6 -> indicator at 10,6)
#define HUMAN_RESERVE_TILE_X    10
#define HUMAN_RESERVE_TILE_Y    6
// CPU reserve: right of CPU start (tile 8,2 -> indicator at 10,2)
#define CPU_RESERVE_TILE_X      10
#define CPU_RESERVE_TILE_Y      2

// Bear-off indicator: left of position 14 (exit point)
// Human bearoff: left of human exit (tile 14,6 -> indicator at 12,6)
#define HUMAN_BEAROFF_TILE_X    12
#define HUMAN_BEAROFF_TILE_Y    6
// CPU bearoff: right of CPU exit (tile 14,2 -> indicator at 12,2)
#define CPU_BEAROFF_TILE_X      12
#define CPU_BEAROFF_TILE_Y      2

// ============================================================================
// Opponent Portrait (5x5 tiles on right side of UI)
// ============================================================================
#define GAME_PORTRAIT_TILE_START  38  // After board tiles (0-37)
#define GAME_PORTRAIT_X           14  // Tile X position (right side)
#define GAME_PORTRAIT_Y           10  // Tile Y position (one row above CPU info)
#define GAME_PORTRAIT_WIDTH       5
#define GAME_PORTRAIT_HEIGHT      5

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

/**
 * Win/lose state for endgame screen
 * 1 = human won, 0 = human lost
 */
extern uint8_t human_won;

/**
 * Current game mode (single player or link cable)
 */
extern GameMode_t game_mode;

#endif // GAME_H
