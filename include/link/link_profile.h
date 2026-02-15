/**
 * link_profile.h
 * Link cable profile selection screen
 * Each connected player picks a portrait, exchanges the choice over link,
 * then sees a VS reveal before continuing to the game.
 */

#ifndef LINK_PROFILE_H
#define LINK_PROFILE_H

#include <stdint.h>

// Internal phases
#define LPROFILE_PHASE_SELECTING  0
#define LPROFILE_PHASE_WAITING    1
#define LPROFILE_PHASE_SYNCING    2
#define LPROFILE_PHASE_VS_REVEAL  3

// Number of profile choices
#define LPROFILE_COUNT 4

// Portrait dimensions (tiles)
#define LPROFILE_PORTRAIT_WIDTH  5
#define LPROFILE_PORTRAIT_HEIGHT 5

// Selection grid positions (tile coordinates, same as opponent_select)
#define LPROFILE_0_X 2
#define LPROFILE_0_Y 2
#define LPROFILE_1_X 11
#define LPROFILE_1_Y 2
#define LPROFILE_2_X 2
#define LPROFILE_2_Y 9
#define LPROFILE_3_X 11
#define LPROFILE_3_Y 9

// Selection border dimensions (7x7 around 5x5 portrait)
#define LPROFILE_BORDER_WIDTH  7
#define LPROFILE_BORDER_HEIGHT 7

// Description text position
#define LPROFILE_DESC_X     1
#define LPROFILE_DESC_Y     15
#define LPROFILE_DESC_WIDTH 18

// VS reveal layout (tile coordinates)
#define LPROFILE_VS_YOU_X       2     // Your portrait X
#define LPROFILE_VS_YOU_Y       4     // Your portrait Y
#define LPROFILE_VS_YOU_BRD_X   1     // Your border X
#define LPROFILE_VS_YOU_BRD_Y   3     // Your border Y
#define LPROFILE_VS_OTHER_X     13    // Other portrait X
#define LPROFILE_VS_OTHER_Y     4     // Other portrait Y
#define LPROFILE_VS_OTHER_BRD_X 12    // Other border X
#define LPROFILE_VS_OTHER_BRD_Y 3     // Other border Y
#define LPROFILE_VS_TEXT_X      9     // "VS" text X
#define LPROFILE_VS_TEXT_Y      6     // "VS" text Y
#define LPROFILE_VS_NAME_Y      10    // Names row
#define LPROFILE_VS_PROMPT_Y    16    // "PRESS A" row

// VRAM allocation (screen-isolated, reloaded on entry)
#define LPROFILE_BLANK_TILE         0
#define LPROFILE_PORTRAIT_START     1    // Merged tileset (117 tiles, 1-117)
#define LPROFILE_BORDER_START       118  // Border tiles for selection (25 tiles, 118-139 fits before font at 141)

// VS screen VRAM (reloaded, shares tileset with selection)
#define LPROFILE_VS_YOU_TILE_START  1    // Same as LPROFILE_PORTRAIT_START (shared tileset)
#define LPROFILE_VS_BORDER_START    118  // Border tiles (25 tiles, 118-139)

// Timing
#define LPROFILE_DOT_CYCLE       20   // Frames per dot animation step
#define LPROFILE_VS_DURATION     180  // ~3 seconds at 60fps
#define LPROFILE_SYNC_TIMEOUT    360  // Max frames to wait for peer sync (~6s)

// Exported globals
extern uint8_t link_local_profile;
extern uint8_t link_remote_profile;

/**
 * Initialize link profile selection screen
 */
void init_link_profile(void);

/**
 * Update link profile selection screen (called every frame)
 */
void update_link_profile(void);

/**
 * Cleanup link profile selection screen
 */
void cleanup_link_profile(void);

#endif // LINK_PROFILE_H
