# Code Improvements Analysis

This document identifies areas for improvement before continuing with AI implementation. The Game Boy has limited ROM (32KB), RAM (~8KB WRAM), and CPU (~1MHz). Unnecessary complexity wastes these resources.

---

## Critical Issues

These issues should be addressed before adding more features.

### 1. Massive Code Duplication: Transition System

**Location:** `title.c:115-155`, `opponent_select.c:172-212`, `difficulty_select.c:58-101`, `coinflip.c:98-138`

**Problem:** The exact same `transition_start()` and `update_transition()` functions are copy-pasted across 4 files (~40 lines each = ~160 lines of duplicated code).

**Impact:**
- Wastes ~400 bytes of ROM
- Bug fixes require changes in 4 places
- Increases maintenance burden

**Solution:** Extract to a shared `transition.c` module:
```c
// transition.h
void transition_start(ScreenState_t target, uint8_t phase_count);
uint8_t update_transition(void);
```

### 2. Duplicated Random Number Generators

**Location:** `game.c:124-137`, `coinflip.c:67-72`, `title.c:42-51`

**Problem:** Three different implementations of random number generation:
- `game.c`: Full Galois LFSR with seeding
- `coinflip.c`: Same Galois LFSR (duplicated)
- `title.c`: Simple DIV_REG read

**Impact:**
- Wastes ~50 bytes of ROM
- Inconsistent random quality across screens
- Duplicated state variables (`rand_state` in both game.c and coinflip.c)

**Solution:** Create a shared `random.c` module with one LFSR implementation.

### 3. Duplicated `white_tile` Data

**Location:** `opponent_select.c:164-168`, `difficulty_select.c:46-50`, `coinflip.c:55-59`, `endgame.c:28-32`

**Problem:** The same 16-byte `white_tile` array is defined in 4 different files.

**Impact:** Wastes 48 bytes of ROM (3 extra copies of 16 bytes).

**Solution:** Define once in a shared location or use GBDK's built-in fill functions.

### 4. Repeated Portrait Drawing Code

**Location:** `game.c:161-203`, `difficulty_select.c:119-160`, `endgame.c:52-93`

**Problem:** Nearly identical `draw_opponent_portrait()` / `draw_selected_portrait()` functions with the same switch statement pattern.

**Impact:** ~100 lines of duplicated code across 3 files.

**Solution:** Create a shared `draw_portrait(uint8_t opponent_idx, uint8_t tile_base, uint8_t x, uint8_t y)` function in a common module.

### 5. Repeated Border Drawing Code

**Location:** `opponent_select.c:103-134`, `coinflip.c:173-205`, `endgame.c:96-109`

**Problem:** `draw_border()` and `clear_border()` functions duplicated with minor parameter differences.

**Impact:** ~60 lines of duplicated code.

**Solution:** Extract to shared utility with parameters for position and dimensions.

---

## Important Issues

These should be addressed to improve code quality and reduce resource usage.

### 6. Redundant Font Loading

**Location:** Multiple files call `load_font_inverted()` on every screen init

**Problem:** According to `vram_layout.h`, font tiles (140-220) are "SHARED across all screens" and "persist across screen transitions." Yet every screen re-calls `load_font_inverted()`.

**Impact:**
- Wastes CPU cycles regenerating XOR'd tiles every screen transition
- The inversion loop in `font.c:690-715` runs 40+ times per screen change

**Solution:** Load fonts once during initial boot (in `main.c` before first screen), add a `fonts_loaded` flag to skip redundant loads.

### 7. Unnecessary `fill_screen_white()` Duplication

**Location:** `difficulty_select.c:106-114`, `coinflip.c:143-151`, `endgame.c:34-42`

**Problem:** Same screen-filling function duplicated 3 times.

**Solution:** Move to a shared utility module.

### 8. Large Embedded Font Data

**Location:** `font.c:21-626` - 626 lines, ~640 bytes

**Problem:** The font tiles are stored as C arrays in ROM. For 40 characters at 16 bytes each = 640 bytes.

**Potential Optimization:**
- Consider using GBDK's built-in font if style permits
- Or use compression for the tile data
- Or generate some characters programmatically (0-9 could share pixel patterns with letters)

### 9. Inefficient Board Display Updates

**Location:** `board_state.c:179-218`

**Problem:** `update_board_display()` redraws ALL 20 squares every call, even if only one piece moved.

**Impact:** Excessive VRAM writes. On a move, only 1-3 squares change (source, destination, possibly capture).

**Solution:** Track dirty squares and only redraw changed positions:
```c
void mark_square_dirty(uint8_t pos);
void update_dirty_squares(void);
```

### 10. Game Phase State Machine Complexity

**Location:** `game.c:1054-1155`

**Problem:** The game phase switch statement is 100+ lines with nested conditions. Some phases (like `PHASE_CPU_THINK`) are repurposed for unrelated tasks ("Used as wait state").

**Impact:** Hard to maintain and extend for AI.

**Solution:** Consider function pointers for phase handlers:
```c
typedef void (*PhaseHandler)(void);
static const PhaseHandler phase_handlers[] = {
    handle_wait_roll,
    handle_rolling,
    handle_show_result,
    // ...
};
```

### 11. Comment Mismatch on PIECES_PER_PLAYER

**Location:** `game.h:171`

**Problem:**
```c
#define PIECES_PER_PLAYER   3   // Each player has 7 pieces (set to 1 for testing)
```
Comment says 7, define says 3. This is confusing and likely a leftover from testing.

**Solution:** Fix comment or restore to actual game value (7) before AI work.

### 12. Extern Declaration Explosion

**Location:** Multiple files have 10-20 `extern` declarations each

**Problem:** Asset references like `profile_01_tiles`, `profile_02_tiles`, etc. are declared `extern` in 4+ files.

**Solution:** Create header files for generated assets that consolidate declarations:
```c
// assets.h
extern const uint8_t profile_01_tiles[];
extern const unsigned char profile_01_map[];
// ... etc
```

---

## Optional Improvements

Nice-to-have optimizations that can be deferred.

### 13. Switch Statement in Main Loop

**Location:** `main.c:37-108`

**Problem:** Three separate switch statements handle cleanup, init, and update. This is verbose but readable.

**Alternative:** Function pointer tables would be more compact but less debuggable. Current approach is acceptable for 6 states.

### 14. Pause Screen Over-Engineering

**Location:** `game.c:599-805` - 200+ lines for pause screen

**Problem:** The pause screen implementation is elaborate (sliding animation, border drawing, full stats display). For an embedded game, a simpler "PAUSED" overlay would suffice.

**Impact:** Not a bug, but adds complexity. The animation code alone is ~100 lines.

**Consider:** Whether this complexity is worth it for the final product.

### 15. Multiple Input State Variables

**Location:** `input.c` uses static globals; various screens also track input state

**Problem:** Minimal, but `input_reset()` is called on every screen init. Consider if this is necessary or if input state should persist.

### 16. Coordinate System Inconsistency

**Location:** Throughout codebase

**Problem:** Some code uses tile coordinates, some uses pixel coordinates, some uses sprite coordinates (with +8/+16 offset). Comments help but conversions are scattered.

**Improvement:** Create explicit conversion macros:
```c
#define TILE_TO_PIXEL_X(tx) ((tx) * 8)
#define TILE_TO_SPRITE_X(tx) ((tx) * 8 + 8)
#define TILE_TO_SPRITE_Y(ty) ((ty) * 8 + 16)
```

### 17. Magic Numbers in UI Layout

**Location:** `game.h:79-127`

**Problem:** While constants are defined, there are still hardcoded values like `18` for row clear width throughout the code.

**Improvement:** Add `#define UI_WIDTH 18` or similar.

### 18. Board Lookup Table Memory

**Location:** `board_state.c:36-63`

**Problem:** The `p1_squares` and `p2_private_squares` arrays store `BoardSquare_t` structs with 4 bytes each. For 20 squares = 80 bytes of RAM.

**Optimization:** These are constant and could be `const` (ROM) instead of RAM:
```c
static const BoardSquare_t p1_squares[14] = { ... };  // Already const, good!
```
Actually, they ARE const. No issue here.

### 19. Piece Tile Lookup Table Size

**Location:** `board_state.c:78-115`

**Problem:** The `piece_tiles[6][3][4]` array is 72 bytes. It stores pre-computed tile indices.

**Status:** This is a reasonable space/time tradeoff for an embedded system. The alternative (computing indices at runtime) would be slower.

---

## Summary

| Priority | Issue | Est. ROM Savings | Est. RAM Savings |
|----------|-------|------------------|------------------|
| Critical | Transition system duplication | ~400 bytes | 12 bytes |
| Critical | Random number duplication | ~50 bytes | 4 bytes |
| Critical | white_tile duplication | ~48 bytes | - |
| Critical | Portrait drawing duplication | ~200 bytes | - |
| Critical | Border drawing duplication | ~100 bytes | - |
| Important | Redundant font loading | - | CPU cycles |
| Important | fill_screen_white duplication | ~60 bytes | - |
| Important | Board display inefficiency | - | CPU/VRAM |
| **Total Estimated** | | **~858 bytes** | **~16 bytes** |

## Recommended Action Plan

1. **Before AI work:** Create shared modules for:
   - `transition.c/.h` - Screen transition system
   - `random.c/.h` - PRNG with seeding
   - `ui_common.c/.h` - Portrait drawing, border drawing, screen fill

2. **During AI work:** Keep the AI module isolated and clean to avoid adding more duplication.

3. **After AI work:** Consider the optional improvements if ROM/RAM becomes tight.

---

*Analysis performed on 2026-02-02*
