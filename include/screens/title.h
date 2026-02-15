/**
 * title.h
 * Title screen state management
 */

#ifndef TITLE_H
#define TITLE_H

#include <stdint.h>
#include "vram_layout.h"
#include "util/falling_piece_anim.h"

// Title screen feature flags
#define TITLE_ENABLE_FALLING_PIECES 1

#if TITLE_ENABLE_FALLING_PIECES
#define FALLING_PIECE_SPRITE_INDEX 2      // Base OAM index (uses 4 sprites: 2-5)
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
