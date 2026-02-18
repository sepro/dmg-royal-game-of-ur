# Code Review: Royal Game of Ur (DMG)

Full review of all `.c` and `.h` source files. Issues are grouped by category.
Each entry includes file, line(s), a description, and a suggested fix.

---

## 1. Dead Code - FIXED

---

## 2. Stale / Incorrect Comments - FIXED

---

## 3. Duplicated Code

### 3.1 Border drawing in `draw_difficulty_box()` and `draw_pause_border()` are near-identical
**Files:** `src/screens/difficulty_select.c:48–76`, `src/screens/game.c:767–797`

Both functions build a 20-wide border row-by-row with the same hex-offset pattern (`0x00` corner, `0x01–0x05` repeating top edge, `0x06` corner, `0x07`/`0x08`/`0x09` sides, `0x12–0x18` bottom). The only differences are which VRAM base constant and target layer (`set_bkg_tiles` vs `set_win_tiles`) are used.

**Suggested fix:** Extract a shared `draw_full_width_border(uint8_t tile_start, uint8_t y_start, uint8_t rows, set_tiles_fn)` helper, or at minimum document that these must stay in sync if the border format ever changes.

---

### 3.2 "WAITING" + animated dots duplicated in three files
**Files:**
- `src/screens/game.c:264–292` (`draw_waiting_prompt`, `reset_waiting_dots`, `update_waiting_dots`)
- `src/link/link_connect.c:34–46` (`draw_waiting_status`)
- `src/link/link_profile.c:292–300` (`draw_waiting_dots`)

All three draw "WAITING" then append 0–3 dots, advancing the counter on a timer. The logic is identical; only the tile row and X position differ.

**Suggested fix:** Move a shared `draw_waiting_text(uint8_t x, uint8_t y, uint8_t dot_count)` into `screen_utils.c` or `font.c`.

---

### 3.3 `grid_col()` and `grid_row()` duplicated verbatim
**Files:** `src/screens/opponent_select.c:88–97`, `src/link/link_profile.c:90–99`

Both files contain:
```c
static inline uint8_t grid_col(uint8_t idx) { return idx & 1; }
static inline uint8_t grid_row(uint8_t idx) { return idx >> 1; }
```

**Suggested fix:** Move to a shared header (e.g., `include/util/grid.h`) or inline directly at call sites (they're one-liners).

---

### 3.4 Magic literal `25` for border tile count at 7+ call sites
**Files:** `coinflip.c:314`, `opponent_select.c:116`, `difficulty_select.c:136`, `endgame.c:64`, `game.c:857`, `link_connect.c:86`, `link_profile.c:157, 205`

All pass the literal `25` to `set_bkg_data(... 25, border_tiles)`. `VRAM_BORDER_COUNT` is already defined as 25 in `vram_layout.h` but is not used at these sites.

**Fix:** Replace `25` with `VRAM_BORDER_COUNT` at all call sites.

---

### 3.5 AI move simulation boilerplate repeated four times
**File:** `src/logic/ai.c`

`evaluate_move()` (516), `evaluate_adaptive_move()` (402), `evaluate_phase_move()` (260), and `evaluate_turn_economy_move()` (604) all contain the same block:
```c
// copy arrays → calculate new_pos → cap at POS_FINISHED → check war-zone capture → apply move
```

This is 15–20 lines of identical logic (exact same conditions, same variable names) repeated with different evaluation functions at the end.

**Suggested fix:** Extract a `simulate_move(uint8_t piece_idx, uint8_t roll, uint8_t *temp_cpu, uint8_t *temp_human)` helper that performs the simulation in-place, returning the `new_pos`. The evaluate functions then only differ in their scoring call.

---

### 3.6 Title screen draws menu text twice
**File:** `src/screens/title.c:231–233, 271–273`

In `init_title()`:
```c
// First draw (lines 231-233)
draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
draw_music_option();
draw_text(MENU_TEXT_X, MENU_TEXT_ROW_3, "LINK CABLE");

// ... portrait loading re-clears rows 10-18 ...

// Duplicate draw (lines 271-273)
draw_text(MENU_TEXT_X, MENU_TEXT_ROW_1, "START GAME");
draw_music_option();
draw_text(MENU_TEXT_X, MENU_TEXT_ROW_3, "LINK CABLE");
```

The first three draw calls are immediately overwritten by the portrait tile-loading routine which clears rows 10–18. Only the second set of draws persists. The comment at line 267 ("Re-clear UI rows after portrait tile loads so text stays intact") acknowledges the clearing but the first draw could simply be removed.

**Fix:** Remove the first set of draw calls (lines 228–233, including the three `clear_text_row` calls above them).

---

### 3.7 `TITLE_BOARD_TILE_COUNT` duplicates `VRAM_PIECE_TILES_COUNT`
**File:** `src/screens/title.c:63`
```c
#define TITLE_BOARD_TILE_COUNT 72
```
`VRAM_PIECE_TILES_COUNT` is already 72 in `vram_layout.h`. This is a duplicate constant for the same value.

**Fix:** Replace the local define with `VRAM_PIECE_TILES_COUNT`.

---

### 3.8 `BORDER_TILE_COUNT_IMG` misplaced local constant in `font.c`
**File:** `src/util/font.c:811`
```c
#define BORDER_TILE_COUNT_IMG 25
```
This is defined inside `font.c` to support `load_border_inverted()`, but it belongs conceptually with the border tile data (its value equals `VRAM_BORDER_COUNT` in `vram_layout.h`).

**Fix:** Replace with `VRAM_BORDER_COUNT`.

---

## 4. Naming / Style Inconsistencies

### 4.1 Tile-0 "background" constant has a different name per screen
Three different constants all mean "tile 0 is the solid fill tile for this screen":

| Screen | Constant |
|---|---|
| `opponent_select.h:41` | `BLANK_TILE 0` |
| `difficulty_select.h:38` | `WHITE_TILE 0` |
| `coinflip.h:64` | `COINFLIP_WHITE_TILE 0` |
| `link_connect.h` (via header) | `CONNECT_WHITE_TILE` |

These describe the same VRAM slot with inconsistent semantics (blank vs white). `BLANK_TILE` is misleading on the opponent-select screen because tile 0 there is loaded as a black tile, not blank in the visual sense of `white_tile[]`.

**Suggested fix:** Document the screen-isolation contract clearly in `vram_layout.h` with a comment, and standardise all per-screen names to `SCREEN_BG_TILE 0` or similar.

---

### 4.2 C99 for-loop variable declarations in `difficulty_select.c`
**File:** `src/screens/difficulty_select.c:53, 60, 71`
```c
for (uint8_t x = 1; x < 19; x++) { ...
```
The rest of the codebase declares loop counters before the `for` statement (C89 style). GBDK's `lcc` supports C99 in practice, but the inconsistency is jarring.

**Fix:** Declare `uint8_t x; uint8_t y;` before the loops, matching the rest of the codebase.

---

### 4.3 `(void *)0` instead of `NULL`
**File:** `src/logic/board_state.c:353`
```c
if (out_captured != (void *)0) {
```
Every other null-pointer check in the codebase uses direct comparison `!= 0` or just `if (ptr)`. Using `(void *)0` is non-idiomatic for this C89-style codebase.

**Fix:** `if (out_captured)` or `if (out_captured != 0)`.

---

### 4.4 Magic literals for text positions in `link_connect.c`
**File:** `src/link/link_connect.c:100, 148, 149`
```c
draw_text_inverted(2, 15, "B:CANCEL");
draw_text_inverted(CONNECT_TITLE_X, CONNECT_TITLE_Y, "CONNECTION FAILED");
draw_text_inverted(2, 9, "PRESS A TO RETRY");
```
`2, 15` and `2, 9` are raw tile coordinates with no named constants, while other positions in the same file use `CONNECT_TITLE_X`, `CONNECT_STATUS_X`, etc.

**Fix:** Define `CONNECT_CANCEL_X`, `CONNECT_CANCEL_Y`, `CONNECT_RETRY_X`, `CONNECT_RETRY_Y` in the header.

---

### 4.5 Magic literals for text positions in `link_profile.c`
**File:** `src/link/link_profile.c:166, 167, 168, 171, 177, 181`
```c
draw_text_inverted(LPROFILE_VS_YOU_BRD_X, LPROFILE_VS_YOU_BRD_Y - 1, "YOU");
draw_text_inverted(LPROFILE_VS_OTHER_BRD_X, LPROFILE_VS_OTHER_BRD_Y - 1, "OTHER");
draw_text_inverted(5, LPROFILE_VS_PROMPT_Y, "PRESS A");
draw_text_inverted(2, 0, "CHOOSE YOUR LOOK");
```
The `5` and `2` are bare position constants. `LPROFILE_VS_YOU_BRD_Y - 1` is arithmetic on a constant, which should itself be named.

**Fix:** Add `LPROFILE_VS_YOU_LABEL_Y`, `LPROFILE_TITLE_X`, `LPROFILE_PROMPT_X` etc. to the header.

---

## 5. Minor Logic / Clarity Issues

### 5.1 `TRANSITION_IDLE = 0xFF` as a sentinel is fragile
**File:** `include/game_types.h:82`, `src/util/transition.c:19`
```c
#define TRANSITION_IDLE 0xFF
static uint8_t transition_phase = TRANSITION_IDLE;
```
The idle state is encoded as 0xFF (max uint8_t). When checking `if (transition_phase == TRANSITION_IDLE)` this works because phase never reaches 255 in practice, but the chosen value is not obviously "sentinel" — it reads as a phase number. Using a separate boolean flag or an enum would be clearer.

**Suggested fix:** Add a `static uint8_t transition_active = 0;` flag and check that, removing the sentinel value.

---

### 5.2 `init_game()` seeds RNG with `frame_counter` always 0
**File:** `src/screens/game.c:1176–1177`
```c
frame_counter = 0;
seed_random(DIV_REG ^ ((uint16_t)frame_counter << 8));
```
`frame_counter` is set to 0 two lines above, so the `<< 8` term is always 0. The seed reduces to just `DIV_REG`. This is still random (DIV_REG is a free-running hardware timer), but the comment and formula imply frame entropy that doesn't exist at init time.

**Fix:** Simplify to `seed_random(DIV_REG)` and remove the `frame_counter` term from this one call site.

---

### 5.3 `draw_selected_portraits()` silently skips the mirrored portrait
**File:** `src/screens/difficulty_select.c:111`
```c
if ((uint16_t)DIFF_MIRROR_TILE_BASE + loaded_tiles <= VRAM_FONT_START) {
    mirror_tiles_to_vram(...);
    draw_portrait_expr_mirrored(...);
}
```
If the condition fails, the mirrored portrait is simply not drawn with no visible error or fallback. Since `DIFF_MIRROR_TILE_BASE = 64` and max loaded tiles per character is ~30–40, this check passes in practice, but silent failure leaves the screen partially blank.

**Fix:** Add a comment explaining the safety check and what the screen looks like if it fails:
```c
// Safety: mirror tiles must fit before font starts at VRAM_FONT_START (141)
// DIFF_MIRROR_TILE_BASE (64) + max ~40 tiles = ~104, so this always passes in practice
```

---

### 5.4 `lock_random_tiles()` hardcoded 50-attempt limit
**File:** `src/screens/coinflip.c:148`
```c
for (uint8_t attempts = 0; attempts < 50 && count < max_count; attempts++) {
```
With 25 tiles total and the later tiles mostly locked, this loop may fail to lock `max_count` tiles in late lock-in phase. The caller has a fallback that force-locks all remaining tiles, so there's no bug, but the 50 is an arbitrary magic number.

**Fix:** Define `COIN_LOCK_ATTEMPTS 50` as a named constant, or simplify by iterating through tiles directly when few are left.

---

### 5.5 `start_animation()` clears rows with a nested loop
**File:** `src/screens/coinflip.c:209–213`
```c
for (uint8_t y = 3; y <= 11; y++) {
    for (uint8_t x = 0; x < 20; x++) {
        set_bkg_tile_xy(x, y, COINFLIP_WHITE_TILE);
    }
}
```
`clear_rect()` exists in `screen_utils.c` and does exactly this. The nested loop is an unnecessary reinvention.

**Fix:** Replace with `clear_rect(COINFLIP_WHITE_TILE, 0, 3, 20, 9)`.

---

## 6. Summary Table

| # | Category | Severity | File(s) |
|---|---|---|---|
| 2.1 | Stale comment | Low | `vram_layout.h` |
| 2.2 | Wrong comment | Low | `coinflip.h` |
| 2.3 | Wrong comment | Medium | `board_state.c` |
| 2.4 | Stale comment | Trivial | `game.h` |
| 2.5 | Wrong doc | Low | `ai.h` |
| 2.6 | External ref | Trivial | `ai.h` |
| 2.7 | History note | Trivial | `random.c` |
| 2.8 | External ref | Low | `vram_layout.h` |
| 2.9 | Wrong doc | Medium | `link.h` |
| 2.10 | Malformed doc | Low | `screen_utils.h` |
| 2.11 | Missing doc | Trivial | `endgame.c` |
| 2.12 | Missing doc | Trivial | `falling_piece_anim.h` |
| 3.1 | Duplication | Medium | `difficulty_select.c`, `game.c` |
| 3.2 | Duplication | Medium | `game.c`, `link_connect.c`, `link_profile.c` |
| 3.3 | Duplication | Low | `opponent_select.c`, `link_profile.c` |
| 3.4 | Magic number | Low | 8 files |
| 3.5 | Duplication | Medium | `ai.c` |
| 3.6 | Wasted work | Low | `title.c` |
| 3.7 | Duplicate constant | Trivial | `title.c` |
| 3.8 | Misplaced constant | Trivial | `font.c` |
| 4.1 | Naming | Low | 4 headers |
| 4.2 | Style | Trivial | `difficulty_select.c` |
| 4.3 | Style | Trivial | `board_state.c` |
| 4.4 | Magic literals | Low | `link_connect.c` |
| 4.5 | Magic literals | Low | `link_profile.c` |
| 5.1 | Clarity | Low | `transition.c`, `game_types.h` |
| 5.2 | Misleading code | Trivial | `game.c` |
| 5.3 | Silent failure | Low | `difficulty_select.c` |
| 5.4 | Magic number | Trivial | `coinflip.c` |
| 5.5 | Reinvented util | Low | `coinflip.c` |
