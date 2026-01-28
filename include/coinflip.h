/**
 * coinflip.h
 * Side selection and coin flip screen state management
 */

#ifndef COINFLIP_H
#define COINFLIP_H

#include <stdint.h>
#include "vram_layout.h"

// Side selection constants
#define SIDE_LIGHT 0
#define SIDE_DARK  1

// Screen layout constants (tile coordinates)
#define COINFLIP_TITLE_X      1
#define COINFLIP_TITLE_Y      1

// Selection coin positions (5x5 tiles each)
#define COINFLIP_LIGHT_X      2
#define COINFLIP_LIGHT_Y      4
#define COINFLIP_DARK_X       13
#define COINFLIP_DARK_Y       4

// Selection label positions (below border which ends at y=9)
#define COINFLIP_LIGHT_LABEL_X  3
#define COINFLIP_LIGHT_LABEL_Y  11
#define COINFLIP_DARK_LABEL_X   14
#define COINFLIP_DARK_LABEL_Y   11

// Animation coin position (centered, 5x5)
#define COINFLIP_ANIM_X       7
#define COINFLIP_ANIM_Y       6

// Result text position (below animation coin)
#define COINFLIP_RESULT_X     3
#define COINFLIP_RESULT_Y     12

// Coin dimensions (tiles)
#define COIN_WIDTH  5
#define COIN_HEIGHT 5
#define COIN_TILES  25

// Selection border dimensions (7x7 tiles around 5x5 coin)
#define COINFLIP_BORDER_WIDTH   7
#define COINFLIP_BORDER_HEIGHT  7

// Animation phase constants
#define ANIM_PHASE_NONE     0
#define ANIM_PHASE_CHAOTIC  1
#define ANIM_PHASE_LOCKIN   2
#define ANIM_PHASE_COMPLETE 3
#define ANIM_PHASE_RESULT   4

// Animation timing (frames)
#define ANIM_CHAOTIC_DURATION  45   // Chaotic flipping phase
#define ANIM_CHAOTIC_INTERVAL   4   // Update interval during chaotic
#define ANIM_LOCKIN_DURATION   75   // Lock-in phase (tiles settle)
#define ANIM_COMPLETE_DURATION 30   // Pause after all locked
#define ANIM_RESULT_DURATION   90   // Show result before transition

// VRAM tile indices (screen-isolated: 0-58)
#define COINFLIP_WHITE_TILE     0

// Light coin tiles: 1-25
#define COINFLIP_LIGHT_TILE_START 1
#define COINFLIP_LIGHT_TILE_END   25
#define COINFLIP_LIGHT_TILE_COUNT 25

// Dark coin tiles: 26-50
#define COINFLIP_DARK_TILE_START  26
#define COINFLIP_DARK_TILE_END    50
#define COINFLIP_DARK_TILE_COUNT  25

// Border tiles: 51-58 (reused pattern from opponent select)
#define COINFLIP_BORDER_TILE_START 51

// Selected side (readable by other modules: SIDE_LIGHT or SIDE_DARK)
extern uint8_t selected_side;

// Coin flip result (SIDE_LIGHT or SIDE_DARK)
extern uint8_t coin_result;

// Starting player (0 = human, 1 = AI)
extern uint8_t starting_player;

/**
 * Initialize coin flip screen
 * Loads tiles, draws UI, sets up selection border
 */
void init_coinflip(void);

/**
 * Update coin flip screen (called every frame)
 * Handles selection, animation phases, transitions
 */
void update_coinflip(void);

/**
 * Cleanup coin flip screen
 * Called when transitioning to another state
 */
void cleanup_coinflip(void);

#endif // COINFLIP_H
