/**
 * game_types.h
 * Common types, constants, and enumerations for Royal Game of Ur
 * NOTE: Renamed from types.h to avoid conflict with GBDK's types.h
 */

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>
#include "vram_layout.h"  // See vram_layout.h for global allocation map

// Screen states for the game state machine
typedef enum {
    STATE_TITLE,
    STATE_OPPONENT_SELECT,
    STATE_DIFFICULTY_SELECT,
    STATE_COINFLIP,
    STATE_GAME,
    STATE_ENDGAME,
    STATE_LINK_CONNECT,
    STATE_LINK_PROFILE_SELECT
} ScreenState_t;

// Title screen menu options
typedef enum {
    MENU_START_GAME = 0,
    MENU_MUSIC_TOGGLE = 1,
    MENU_LINK_CABLE = 2,
    MENU_OPTION_COUNT = 3
} MenuOption_t;

// Game mode (single player vs link cable multiplayer)
typedef enum {
    GAME_MODE_SINGLE = 0,
    GAME_MODE_LINK = 1
} GameMode_t;

// Constants
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define TILE_SIZE 8

// Menu text positions (tile coordinates)
#define MENU_TEXT_X 5         // Text starts at tile column 5
#define MENU_TEXT_ROW_1 12    // "START GAME" at tile row 12
#define MENU_TEXT_ROW_2 14    // "MUSIC ON/OFF" at tile row 14
#define MENU_TEXT_ROW_3 16    // "LINK CABLE" at tile row 16

// Menu arrow sprite positions
// Sprite Y has 16px offset, so row 12 (pixel 96) = sprite Y 112
#define ARROW_X 36            // Pixel X position (left of text at tile 5 = pixel 40)
#define ARROW_START_Y 112     // Sprite Y for row 12 (96 + 16 offset)
#define ARROW_SPACING 16      // 2 tile rows = 16 pixels

// VBlank helper
#define WAIT_VBLANK wait_vbl_done()

// Blink sprite configuration
#define BLINK_SPRITE_INDEX 1      // OAM entry (arrow uses 0)
#define BLINK_TILE_START VRAM_SPRITE_BLINK_START
#define BLINK_FRAME_COUNT 4
#define BLINK_ANIM_SPEED 6        // Frames per animation frame (~0.1s at 60fps)
#define BLINK_DELAY_MIN 30        // Min frames hidden (~0.5s)
#define BLINK_DELAY_MAX 90        // Max frames hidden (~1.5s)

// Blink position bounds (sprite coordinates include +8 X, +16 Y offset)
#define BLINK_MIN_X 24            // Keep within title logo area
#define BLINK_MAX_X 144
#define BLINK_MIN_Y 32            // Upper portion of screen where title is
#define BLINK_MAX_Y 96

// Blink states
#define BLINK_STATE_HIDDEN 0
#define BLINK_STATE_ANIMATING 1

// Screen transition animation constants
#define TRANSITION_FLASH_DURATION 3        // Frames per flash (~50ms at 60fps)
#define TRANSITION_PHASE_COUNT_3 4         // OFF-ON-OFF-ON = 4 phases
#define TRANSITION_PHASE_COUNT_1 2         // OFF-ON = 2 phases (link cable feedback)
#define TRANSITION_IDLE 0xFF               // No transition active

#endif // GAME_TYPES_H
