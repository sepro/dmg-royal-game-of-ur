/**
 * link_connect.h
 * Link cable connection screen
 * Shows waiting animation and reveals player side assignment
 */

#ifndef LINK_CONNECT_H
#define LINK_CONNECT_H

#include <stdint.h>

// Connection phases
#define CONNECT_PHASE_WAITING     0
#define CONNECT_PHASE_SIDE_REVEAL 1
#define CONNECT_PHASE_SYNCING     2

// Timing (frames)
#define CONNECT_DOT_CYCLE       20    // Frames per dot animation step
#define CONNECT_REVEAL_DURATION 120   // Frames to show side before transition
#define CONNECT_SYNC_TIMEOUT   180   // Max frames to wait for peer sync (~3s)

// Screen layout (tile coordinates)
#define CONNECT_TITLE_X    2
#define CONNECT_TITLE_Y    4
#define CONNECT_STATUS_X   2
#define CONNECT_STATUS_Y   7
#define CONNECT_COIN_X     7
#define CONNECT_COIN_Y     5
#define CONNECT_SIDE_X     3
#define CONNECT_SIDE_Y     12

// VRAM tile allocation (screen-isolated, same as coinflip)
#define CONNECT_WHITE_TILE        0
#define CONNECT_LIGHT_TILE_START  1
#define CONNECT_LIGHT_TILE_COUNT  25
#define CONNECT_DARK_TILE_START   26
#define CONNECT_DARK_TILE_COUNT   25

/**
 * Initialize link connection screen
 */
void init_link_connect(void);

/**
 * Update link connection screen (called every frame)
 */
void update_link_connect(void);

/**
 * Cleanup link connection screen
 */
void cleanup_link_connect(void);

#endif // LINK_CONNECT_H
