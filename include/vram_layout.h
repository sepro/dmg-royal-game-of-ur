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
 * Opponent Select Screen (Tiles 1-94)
 * Screen-isolated: Overlaps with title screen range
 * ---------------------------------------------------------------------------- */
// Opponent portraits: 4 characters with variable sizes
// Profile sizes: 25, 21, 20, 20 tiles = 86 tiles total
#define VRAM_OPPONENT_PORTRAITS_START 1
#define VRAM_OPPONENT_PORTRAITS_END 86
#define VRAM_OPPONENT_PORTRAITS_COUNT 86

// Selection border: 8 unique border tiles for 7x7 frame
#define VRAM_BORDER_START 87
#define VRAM_BORDER_END 94
#define VRAM_BORDER_COUNT 8

/* ----------------------------------------------------------------------------
 * Difficulty Select Screen (Tiles 1-24)
 * Screen-isolated: Overlaps with title/opponent screen ranges
 * ---------------------------------------------------------------------------- */
#define VRAM_DIFF_PORTRAIT_START 1
#define VRAM_DIFF_PORTRAIT_END 24
#define VRAM_DIFF_PORTRAIT_COUNT 25  // Selected opponent's portrait (5x5 tiles)

/* ----------------------------------------------------------------------------
 * Font System (Tiles 140-196) - SHARED across all screens
 * These tiles persist across screen transitions and are never reloaded
 * ---------------------------------------------------------------------------- */
// Regular font: 26 letters + space + blank tile
#define VRAM_FONT_START 140
#define VRAM_FONT_END 167
#define VRAM_FONT_COUNT 28

// Inverted font: Generated at runtime via XOR (black text on white bg)
// Contains: 26 letters + space + blank + white tile
#define VRAM_FONT_INVERTED_START 168
#define VRAM_FONT_INVERTED_END 196
#define VRAM_FONT_INVERTED_COUNT 29

/* ----------------------------------------------------------------------------
 * Phase 5: Coin Flip Screen (Tiles 197-220) - RESERVED
 * Estimated requirement: ~24 tiles for coin animation + UI elements
 * ---------------------------------------------------------------------------- */
#define VRAM_COINFLIP_START 197
#define VRAM_COINFLIP_END 220
#define VRAM_COINFLIP_COUNT 24

/* ----------------------------------------------------------------------------
 * Phase 6: Game Board (Tiles 221-255) - RESERVED
 * Estimated requirement: ~35 tiles for board graphics + UI
 * NOTE: This is a tight allocation. May need optimization via:
 *   - Reusing symmetrical tiles
 *   - Using sprites for some UI elements
 *   - Palette tricks for color variation
 * ---------------------------------------------------------------------------- */
#define VRAM_GAMEBOARD_START 221
#define VRAM_GAMEBOARD_END 255
#define VRAM_GAMEBOARD_COUNT 35

/* ----------------------------------------------------------------------------
 * Background Tile Budget Summary
 * ---------------------------------------------------------------------------- */
#define VRAM_BG_TOTAL 256
#define VRAM_BG_USED 197   // Peak usage: difficulty screen with inverted font
#define VRAM_BG_AVAILABLE 59  // Tiles 197-255 free for future phases

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

// Verify inverted font fits before reserved ranges
#if VRAM_FONT_INVERTED_END >= VRAM_COINFLIP_START
#error "VRAM conflict: Inverted font overlaps with coin flip reserved range"
#endif

// Verify coin flip and game board ranges don't overlap
#if VRAM_COINFLIP_END >= VRAM_GAMEBOARD_START
#error "VRAM conflict: Coin flip overlaps with game board reserved range"
#endif

// Verify game board doesn't exceed VRAM
#if VRAM_GAMEBOARD_END > 255
#error "VRAM overflow: Game board allocation exceeds 256 tile limit"
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
 * - Title, opponent select, and difficulty screens each completely reload
 *   VRAM on entry, so tiles 0-138 can be reused across these screens
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
 * - Phase 5 (coin flip) has 24 tiles reserved at 197-220
 * - Phase 6 (game board) has 35 tiles at 221-255 (may need optimization)
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
