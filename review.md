# Code Review: Royal Game of Ur (DMG)

Full review of all `.c` and `.h` source files. Issues are grouped by category.
Each entry includes file, line(s), a description, and a suggested fix.

---

## 1. Dead Code - FIXED

---

## 2. Stale / Incorrect Comments - FIXED

---

## 3. Duplicated Code - FIXED

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

## 5. Minor Logic / Clarity Issues - FIXED


---

## 6. Summary Table

| # | Category | Severity | File(s) |
|---|---|---|---|
| 4.1 | Naming | Low | 4 headers |
| 4.2 | Style | Trivial | `difficulty_select.c` |
| 4.3 | Style | Trivial | `board_state.c` |
| 4.4 | Magic literals | Low | `link_connect.c` |
| 4.5 | Magic literals | Low | `link_profile.c` |
