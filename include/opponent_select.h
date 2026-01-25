/**
 * opponent_select.h
 * Opponent selection screen state management
 */

#ifndef OPPONENT_SELECT_H
#define OPPONENT_SELECT_H

#include <stdint.h>

// Number of opponents
#define OPPONENT_COUNT 4

// Portrait dimensions (tiles)
#define PORTRAIT_WIDTH 5
#define PORTRAIT_HEIGHT 5

// Portrait positions (tile coordinates)
#define PORTRAIT_0_X 2
#define PORTRAIT_0_Y 2
#define PORTRAIT_1_X 11
#define PORTRAIT_1_Y 2
#define PORTRAIT_2_X 2
#define PORTRAIT_2_Y 9
#define PORTRAIT_3_X 11
#define PORTRAIT_3_Y 9

// Selection border dimensions (7x7 tiles around 5x5 portrait)
#define BORDER_WIDTH 7
#define BORDER_HEIGHT 7

// Description text position
#define DESC_TEXT_X 1
#define DESC_TEXT_Y 15
#define DESC_TEXT_WIDTH 18

// VRAM tile indices
#define PORTRAIT_TILE_START 1     // Profile tiles start at 1 (0 is blank/black)
#define BORDER_TILE_START 87      // Border tiles after portraits (25+21+20+20=86 portrait tiles)
#define BLANK_TILE 0              // Black/empty tile

// Selected opponent (readable by other modules)
extern uint8_t selected_opponent;

/**
 * Initialize opponent selection screen
 * Loads tiles, draws portraits, initial border and text
 */
void init_opponent_select(void);

/**
 * Update opponent selection screen (called every frame)
 * Handles D-pad navigation and A/B buttons
 */
void update_opponent_select(void);

/**
 * Cleanup opponent selection screen
 * Called when transitioning to another state
 */
void cleanup_opponent_select(void);

#endif // OPPONENT_SELECT_H
