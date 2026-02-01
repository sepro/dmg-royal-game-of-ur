/**
 * vram_layout.h
 * Centralized VRAM Tile Allocation Map
 *
 * SINGLE SOURCE OF TRUTH for all VRAM tile indices.
 * All graphics-related headers should reference constants defined here.
 *
 * Game Boy DMG VRAM Structure:
 * - Background tiles: 256 slots (0-255) at $8000-$8FFF
 * - Sprite tiles: 256 slots (0-255) at $8000-$8FFF (separate from bg in mode 0)
 * - Each tile is 16 bytes (8x8 pixels, 2bpp)
 *
 * Current Usage:
 * - Background: 197 tiles used, 59 available (197-255)
 * - Sprites: 5 tiles used, 251 available (5-255)
 */

#ifndef VRAM_LAYOUT_H
#define VRAM_LAYOUT_H

/* ============================================================================
 * BACKGROUND TILES (256 total, 0-255)
 * ============================================================================ */

/* ----------------------------------------------------------------------------
 * Tile 0: Screen-Dependent Special Purpose
 * ---------------------------------------------------------------------------- */
#define VRAM_TILE_ZERO 0
// NOTE: Tile 0 has different meanings per screen (by design):
//   - Title screen: Part of title image
//   - Opponent select: Black/blank tile for borders
//   - Difficulty select: White tile for background
//   - This is intentional; each screen reloads VRAM on entry

/* ----------------------------------------------------------------------------
 * Title Screen (Tiles 1-138) - 139 tiles total including tile 0
 * Screen-isolated: These tiles are overwritten by opponent/difficulty screens
 * ---------------------------------------------------------------------------- */
#define VRAM_TITLE_START 0
#define VRAM_TITLE_END 138
#define VRAM_TITLE_COUNT 139

/* ----------------------------------------------------------------------------
 * Opponent Select Screen (Tiles 1-111)
 * Screen-isolated: Overlaps with title screen range
 * ---------------------------------------------------------------------------- */
// Opponent portraits: 4 characters with variable sizes
// Profile sizes: 25, 21, 20, 20 tiles = 86 tiles total
#define VRAM_OPPONENT_PORTRAITS_START 1
#define VRAM_OPPONENT_PORTRAITS_END 86
#define VRAM_OPPONENT_PORTRAITS_COUNT 86

// Selection border: 25 unique border tiles for 7x7 frame (from border.png)
#define VRAM_BORDER_START 87
#define VRAM_BORDER_END 111
#define VRAM_BORDER_COUNT 25

/* ----------------------------------------------------------------------------
 * Difficulty Select Screen (Tiles 1-24)
 * Screen-isolated: Overlaps with title/opponent screen ranges
 * ---------------------------------------------------------------------------- */
#define VRAM_DIFF_PORTRAIT_START 1
#define VRAM_DIFF_PORTRAIT_END 24
#define VRAM_DIFF_PORTRAIT_COUNT 25  // Selected opponent's portrait (5x5 tiles)

/* ----------------------------------------------------------------------------
 * Coin Flip Screen (Tiles 0-75) - Screen-isolated
 * Overlaps with title/opponent/difficulty screens (each reloads on entry)
 * ---------------------------------------------------------------------------- */
// Tile 0: White tile (screen-specific)
#define VRAM_COINFLIP_WHITE_TILE 0

// Light coin: 5x5 = 25 tiles
#define VRAM_COINFLIP_LIGHT_START 1
#define VRAM_COINFLIP_LIGHT_END 25
#define VRAM_COINFLIP_LIGHT_COUNT 25

// Dark coin: 5x5 = 25 tiles
#define VRAM_COINFLIP_DARK_START 26
#define VRAM_COINFLIP_DARK_END 50
#define VRAM_COINFLIP_DARK_COUNT 25

// Border tiles: 25 tiles (from border.png)
#define VRAM_COINFLIP_BORDER_START 51
#define VRAM_COINFLIP_BORDER_END 75
#define VRAM_COINFLIP_BORDER_COUNT 25

/* ----------------------------------------------------------------------------
 * Font System (Tiles 140-219) - SHARED across all screens
 * These tiles persist across screen transitions and are never reloaded
 * ---------------------------------------------------------------------------- */
// Regular font: 26 letters + space + blank + 10 numbers (0-9) + colon = 40 tiles
#define VRAM_FONT_START 140
#define VRAM_FONT_END 179
#define VRAM_FONT_COUNT 40

// Inverted font: Generated at runtime via XOR (black text on white bg)
// Contains: 26 letters + space + blank + white + 10 numbers + colon = 41 tiles
#define VRAM_FONT_INVERTED_START 180
#define VRAM_FONT_INVERTED_END 220
#define VRAM_FONT_INVERTED_COUNT 41

/* ----------------------------------------------------------------------------
 * Phase 6: Game Board (Tiles 0-37) - SCREEN ISOLATED
 * Game board is screen-isolated and uses tiles 0-37
 * Board tiles do not overlap with font system (starts at 140)
 * Tiles 217-255 remain available for future use
 * ---------------------------------------------------------------------------- */
#define VRAM_GAMEBOARD_START 0
#define VRAM_GAMEBOARD_END 37
#define VRAM_GAMEBOARD_COUNT 38

/* ----------------------------------------------------------------------------
 * Phase 8b: Piece Tiles (Tiles 64-135) - SCREEN ISOLATED
 * 6 square types x 3 piece states x 4 tiles per 16x16 square = 72 tiles
 * Used to overlay pieces on board squares during gameplay
 * ---------------------------------------------------------------------------- */
#define VRAM_PIECE_TILES_START    64
#define VRAM_PIECE_TILES_COUNT    72   // 6 types x 3 states x 4 tiles
#define VRAM_PIECE_TILES_END      135

/* ----------------------------------------------------------------------------
 * Pause Screen Border (Tiles 221-245)
 * Shared with game screen, loaded only when pause is active
 * ---------------------------------------------------------------------------- */
#define VRAM_PAUSE_BORDER_START 221
#define VRAM_PAUSE_BORDER_END 245
#define VRAM_PAUSE_BORDER_COUNT 25

/* ----------------------------------------------------------------------------
 * Background Tile Budget Summary
 * ---------------------------------------------------------------------------- */
#define VRAM_BG_TOTAL 256
#define VRAM_BG_USED 246   // Peak usage: font + inverted font + pause border
#define VRAM_BG_AVAILABLE 10  // Tiles 246-255 available for future use

/* ============================================================================
 * SPRITE TILES (256 total, 0-255, separate address space)
 * ============================================================================ */

/* ----------------------------------------------------------------------------
 * Menu System Sprites (Tiles 0-4)
 * ---------------------------------------------------------------------------- */
// Arrow cursor sprite (used in title, opponent, difficulty screens)
#define VRAM_SPRITE_ARROW 0
#define VRAM_SPRITE_ARROW_COUNT 1

// Blink animation sprite (title screen easter egg)
#define VRAM_SPRITE_BLINK_START 1
#define VRAM_SPRITE_BLINK_END 4
#define VRAM_SPRITE_BLINK_COUNT 4

/* ----------------------------------------------------------------------------
 * Phase 5+: Game Sprites (Tiles 5-255) - AVAILABLE
 * Reserved for:
 *   - Coin flip animation sprites
 *   - Game pieces (7 per player = 14 pieces, could use 1-2 tiles each)
 *   - Dice indicators/animations
 *   - Turn indicators
 *   - Victory animations
 * ---------------------------------------------------------------------------- */
#define VRAM_SPRITE_GAME_START 5
#define VRAM_SPRITE_GAME_END 255
#define VRAM_SPRITE_GAME_COUNT 251

/* ----------------------------------------------------------------------------
 * Sprite Tile Budget Summary
 * ---------------------------------------------------------------------------- */
#define VRAM_SPRITE_TOTAL 256
#define VRAM_SPRITE_USED 5
#define VRAM_SPRITE_AVAILABLE 251

/* ============================================================================
 * COMPILE-TIME VALIDATION
 * Catch allocation overlaps during compilation
 * ============================================================================ */

// Verify font doesn't overlap with opponent select
#if VRAM_FONT_START <= VRAM_BORDER_END
#error "VRAM conflict: Font overlaps with opponent select border tiles"
#endif

// Verify inverted font fits within VRAM
#if VRAM_FONT_INVERTED_END > 255
#error "VRAM overflow: Inverted font allocation exceeds 256 tile limit"
#endif

// Verify game board (screen-isolated) doesn't overlap with font
// This is OK because game board only uses 0-37 and font starts at 140
#if VRAM_GAMEBOARD_END >= VRAM_FONT_START
#error "VRAM conflict: Game board overlaps with font tiles"
#endif

// Verify piece tiles don't overlap with font
#if VRAM_PIECE_TILES_END >= VRAM_FONT_START
#error "VRAM conflict: Piece tiles overlap with font tiles"
#endif

// Verify sprite allocations
#if VRAM_SPRITE_BLINK_END >= VRAM_SPRITE_GAME_START
#error "VRAM conflict: Blink animation overlaps with game sprite range"
#endif

/* ============================================================================
 * USAGE NOTES
 * ============================================================================
 *
 * Screen Isolation vs. Shared Tiles:
 * ----------------------------------
 * - Title, opponent select, difficulty, and coin flip screens each completely
 *   reload VRAM on entry, so tiles 0-138 can be reused across these screens
 * - Font tiles (140-196) are loaded ONCE and never reloaded, persisting
 *   across all screen transitions
 *
 * Adding New Graphics:
 * -------------------
 * 1. Check this file for available tile ranges
 * 2. Update the appropriate section with your allocation
 * 3. Update the *_USED and *_AVAILABLE summary constants
 * 4. Verify compile-time assertions still pass
 * 5. Test visual output to ensure no corruption
 *
 * Future Phase Planning:
 * ---------------------
 * - Phase 5 (coin flip) uses screen-isolated tiles 0-75
 * - Phase 6 (game board) has 59 tiles at 197-255
 * - If game board needs more tiles, consider:
 *   * Reusing font tiles for UI text (already available)
 *   * Using sprites for pieces instead of background tiles
 *   * Palette-based variation instead of unique tiles
 *   * Symmetry: flip tiles in code rather than storing both versions
 *
 * Performance Considerations:
 * --------------------------
 * - Always wait for VBlank before VRAM writes (see vblank_wait() in CLAUDE.md)
 * - Loading large tile sets (>50 tiles) may take multiple frames
 * - Consider spreading VRAM updates across frames if needed
 */

#endif // VRAM_LAYOUT_H
