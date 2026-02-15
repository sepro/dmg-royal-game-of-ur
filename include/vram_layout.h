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
 * Opponent Select Screen (Tiles 1-139)
 * Screen-isolated: Overlaps with title screen range
 * ---------------------------------------------------------------------------- */
// Merged portrait tileset: all 4 characters x 3 expressions, deduplicated
#define VRAM_OPPONENT_PORTRAITS_START 1
#define VRAM_OPPONENT_PORTRAITS_END 117
#define VRAM_OPPONENT_PORTRAITS_COUNT 117

// Selection border: 25 unique border tiles for 7x7 frame (from border.png)
#define VRAM_BORDER_START 118
#define VRAM_BORDER_END 139
#define VRAM_BORDER_COUNT 25

/* ----------------------------------------------------------------------------
 * Difficulty Select Screen (Tiles 1-117)
 * Screen-isolated: Overlaps with title/opponent screen ranges
 * Uses merged portrait tileset (loads all 117 tiles for one portrait)
 * ---------------------------------------------------------------------------- */
#define VRAM_DIFF_PORTRAIT_START 1
#define VRAM_DIFF_PORTRAIT_END 117
#define VRAM_DIFF_PORTRAIT_COUNT 117  // Full merged tileset

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
 * Font System (Tiles 141-224) - SHARED across all screens
 * These tiles persist across screen transitions and are never reloaded
 * ---------------------------------------------------------------------------- */
// Regular font: 26 letters + space + blank + 10 numbers (0-9) + colon + period + exclaim = 42 tiles
#define VRAM_FONT_START 141
#define VRAM_FONT_END 182
#define VRAM_FONT_COUNT 42

// Inverted font: Generated at runtime via XOR (black text on white bg)
// Contains: 26 letters + space + blank + white + 10 numbers + colon + period + exclaim = 42 tiles
#define VRAM_FONT_INVERTED_START 183
#define VRAM_FONT_INVERTED_END 224
#define VRAM_FONT_INVERTED_COUNT 42

/* ----------------------------------------------------------------------------
 * Game Board (Tiles 0-37) - SCREEN ISOLATED
 * Game board is screen-isolated and uses tiles 0-37
 * Portrait (38-68), pieces (69-140), font (141+) follow in game screen
 * ---------------------------------------------------------------------------- */
#define VRAM_GAMEBOARD_START 0
#define VRAM_GAMEBOARD_END 37
#define VRAM_GAMEBOARD_COUNT 38

/* ----------------------------------------------------------------------------
 * Piece Tiles (Tiles 69-140) - SCREEN ISOLATED
 * 6 square types x 3 piece states x 4 tiles per 16x16 square = 72 tiles
 * Used to overlay pieces on board squares during gameplay
 * Starts at 69 to leave room for per-char portrait tiles (38-68, max 31)
 * ---------------------------------------------------------------------------- */
#define VRAM_PIECE_TILES_START    69
#define VRAM_PIECE_TILES_COUNT    72   // 6 types x 3 states x 4 tiles
#define VRAM_PIECE_TILES_END      140

/* ----------------------------------------------------------------------------
 * Pause Screen Border (Tiles 222-246)
 * Loaded only when pause is active, game screen only
 * ---------------------------------------------------------------------------- */
#define VRAM_PAUSE_BORDER_START 225
#define VRAM_PAUSE_BORDER_END 249
#define VRAM_PAUSE_BORDER_COUNT 25

/* ----------------------------------------------------------------------------
 * Background Tile Budget Summary
 * ---------------------------------------------------------------------------- */
#define VRAM_BG_TOTAL 256
#define VRAM_BG_USED 250   // Peak usage: font (141-224) + pause border (225-249)
#define VRAM_BG_AVAILABLE 6  // Tiles 250-255 available for future use

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
 * Game UI Sprites (Tiles 5-8)
 * ---------------------------------------------------------------------------- */
// Piece indicator sprites (8x8 each)
#define VRAM_SPRITE_PIECE_WHITE      5
#define VRAM_SPRITE_PIECE_BLACK      6
// Dice sprites (8x8 each)
#define VRAM_SPRITE_DICE_WHITE       7
#define VRAM_SPRITE_DICE_BLACK       8

/* ----------------------------------------------------------------------------
 * Move Selection Sprites (Tiles 9-17)
 * ---------------------------------------------------------------------------- */
// Selection border: 1 tile (uses flip flags for 4 corners)
#define VRAM_SPRITE_SELECTION_START    9
#define VRAM_SPRITE_SELECTION_COUNT    1

// Destination preview white piece: 4 tiles (16x16 = 4 x 8x8)
#define VRAM_SPRITE_DEST_WHITE_START   10
#define VRAM_SPRITE_DEST_WHITE_COUNT   4

// Destination preview black piece: 4 tiles (16x16 = 4 x 8x8)
#define VRAM_SPRITE_DEST_BLACK_START   14
#define VRAM_SPRITE_DEST_BLACK_COUNT   4

/* ----------------------------------------------------------------------------
 * Game Sprites (Tiles 18-255) - AVAILABLE
 * Reserved for:
 *   - Coin flip animation sprites
 *   - Future game piece animations
 *   - Victory animations
 * ---------------------------------------------------------------------------- */
#define VRAM_SPRITE_GAME_START 18
#define VRAM_SPRITE_GAME_END 255
#define VRAM_SPRITE_GAME_COUNT 238

/* ----------------------------------------------------------------------------
 * Sprite Tile Budget Summary
 * ---------------------------------------------------------------------------- */
#define VRAM_SPRITE_TOTAL 256
#define VRAM_SPRITE_USED 18   // Arrow(1) + Blink(4) + Pieces(2) + Dice(2) + Selection(1) + DestWhite(4) + DestBlack(4)
#define VRAM_SPRITE_AVAILABLE 238

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
// This is OK because game board only uses 0-37 and font starts at 141
#if VRAM_GAMEBOARD_END >= VRAM_FONT_START
#error "VRAM conflict: Game board overlaps with font tiles"
#endif

// Verify piece tiles don't overlap with font
#if VRAM_PIECE_TILES_END >= VRAM_FONT_START
#error "VRAM conflict: Piece tiles overlap with font tiles"
#endif

// Verify pause border doesn't overflow VRAM
#if VRAM_PAUSE_BORDER_END > 255
#error "VRAM overflow: Pause border exceeds 256 tile limit"
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
 * - Font tiles (141-224) are loaded ONCE and never reloaded, persisting
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
 * Performance Considerations:
 * --------------------------
 * - Always wait for VBlank before VRAM writes (see vblank_wait() in CLAUDE.md)
 * - Loading large tile sets (>50 tiles) may take multiple frames
 * - Consider spreading VRAM updates across frames if needed
 */

#endif // VRAM_LAYOUT_H
