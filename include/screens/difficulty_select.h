/**
 * difficulty_select.h
 * Difficulty selection screen state management
 */

#ifndef DIFFICULTY_SELECT_H
#define DIFFICULTY_SELECT_H

#include <stdint.h>
#include "vram_layout.h"  // See vram_layout.h for global allocation map

// Difficulty levels
#define DIFFICULTY_EASY   0
#define DIFFICULTY_MEDIUM 1
#define DIFFICULTY_HARD   2
#define DIFFICULTY_COUNT  3

// Screen layout constants (tile coordinates)
#define DIFF_PORTRAIT_X      1
#define DIFF_PORTRAIT_Y      2
#define DIFF_NAME_X          7
#define DIFF_NAME_Y          4
#define DIFF_TITLE_X         2
#define DIFF_TITLE_Y         10
#define DIFF_OPTION_X        4
#define DIFF_OPTION_START_Y  13

// Arrow sprite position (pixel coordinates with GBDK offsets)
#define DIFF_ARROW_X         24    // Sprite X = tile 2 * 8 + 8 offset
#define DIFF_ARROW_START_Y   120   // Sprite Y = row 13 * 8 + 16 offset
#define DIFF_ARROW_SPACING   8     // 1 tile row = 8 pixels

// VRAM tile indices for this screen

// Border tile allocation (reuse pause/opponent border tiles)
#define DIFF_BORDER_TILE_START VRAM_PAUSE_BORDER_START
#define DIFF_BORDER_TILE_COUNT VRAM_PAUSE_BORDER_COUNT
#define WHITE_TILE           0     // Tile 0 will be solid white (screen-specific semantic constant)
#define DIFF_PORTRAIT_TILE_START VRAM_DIFF_PORTRAIT_START
#define DIFF_MIRROR_TILE_BASE  64

// Selected difficulty (readable by other modules)
extern uint8_t selected_difficulty;

/**
 * Initialize difficulty selection screen
 * Loads tiles, draws portrait, menu options
 */
void init_difficulty_select(void);

/**
 * Update difficulty selection screen (called every frame)
 * Handles Up/Down navigation and A/B buttons
 */
void update_difficulty_select(void);

/**
 * Cleanup difficulty selection screen
 * Called when transitioning to another state
 */
void cleanup_difficulty_select(void);

#endif // DIFFICULTY_SELECT_H
