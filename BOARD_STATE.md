# Board State Implementation Guide

This document describes how to implement piece tracking and board state visualization for Phase 8b.

## Current State (Phase 8a)

Currently, only piece **counts** are tracked in `src/game.c`:

```c
static uint8_t human_reserve;    // Pieces not yet on board (starts at 7)
static uint8_t human_finished;   // Pieces that completed the path
static uint8_t cpu_reserve;
static uint8_t cpu_finished;
```

The board is drawn once as a static image from `board_tiles` and `board_map` and never updated during gameplay.

---

## Board Path Structure

The Royal Game of Ur uses an H-shaped board with this logical path:

```
        P1 Private       Shared Path                    P1 Private
        [1][2][3][4] → [5][6][7][8*][9][10][11][12] → [13][14*] → FINISH
                        ↑
        P2 Private      ↓                              P2 Private
        [1][2][3][4] ←─────────────────────────────── [13][14*] → FINISH

        * = Rosette squares (safe from capture, grants extra turn)
```

- **Positions 1-4**: Player's private start section (no captures possible)
- **Positions 5-12**: Shared middle section (captures possible)
- **Positions 13-14**: Player's private exit section (no captures possible)
- **Rosettes at positions 4, 8, 14**: Safe squares, landing grants another turn

---

## Board layout

Note that the board is layed out like as shown below:


[4][3][2][1][S][F][14][13]    // P1 Private side
[5][6][7][8][9][10][11][12]   // Shared path
[4][3][2][1][S][F][14][13]    // P2 Private side

[S] is the starting zone, here a sprite could be shown, which when selected would move a piece on the board at a later phase.
[F] is the final zone, when a piece would move off the board when selected a sprite could be shown here. 

[S] and [F] are empty gaps in the board.

## Required Data Structures

### Piece Position Arrays

```c
// Position constants
#define POS_RESERVE   0   // Piece not yet on board
#define POS_FINISHED  15  // Piece has completed the path

// Each player has 7 pieces, each with a position value
static uint8_t human_pieces[7];  // human_pieces[i] = position of piece i (0-15)
static uint8_t cpu_pieces[7];    // cpu_pieces[i] = position of piece i (0-15)

// Initialize all pieces in reserve
for (uint8_t i = 0; i < 7; i++) {
    human_pieces[i] = POS_RESERVE;
    cpu_pieces[i] = POS_RESERVE;
}
```

### Square Type Constants

The board uses 6 distinct visual square patterns (see tile template):

```c
#define SQUARE_TYPE_A       0  // Simple bordered pattern
#define SQUARE_TYPE_B       1  // Dots pattern
#define SQUARE_TYPE_C       2  // Diamond pattern
#define SQUARE_TYPE_ROSETTE 3  // Flower/star design
#define SQUARE_TYPE_EYE_A   4  // Large eye pattern
#define SQUARE_TYPE_EYE_B   5  // Eye variant pattern
```

### Piece State Constants

```c
#define PIECE_NONE   0  // Empty square
#define PIECE_WHITE  1  // White piece on square
#define PIECE_BLACK  2  // Black piece on square
```

---

## Board Position to Screen Mapping

Each board position maps to a tile coordinate and square type. The board tilemap uses 2x2 tiles (16x16 pixels) per game square.

### Player 1 Path Coordinates

| Position | Tile (x,y) | Square Type | Notes |
|----------|------------|-------------|-------|
| 1 | (2, 2) | TYPE_A | P1 start |
| 2 | (4, 2) | TYPE_B | P1 start |
| 3 | (6, 2) | TYPE_C | P1 start |
| 4 | (8, 2) | TYPE_B | P1 start, ROSETTE |
| 5 | (2, 4) | TYPE_EYE_A | Shared |
| 6 | (4, 4) | TYPE_C | Shared |
| 7 | (6, 4) | TYPE_EYE_B | Shared |
| 8 | (8, 4) | TYPE_A | Shared, ROSETTE |
| 9 | (10, 4) | TYPE_C | Shared |
| 10 | (12, 4) | TYPE_EYE_B | Shared |
| 11 | (14, 4) | TYPE_B | Shared |
| 12 | (16, 4) | TYPE_C | Shared |
| 13 | (14, 2) | TYPE_A | P1 exit |
| 14 | (16, 2) | TYPE_ROSETTE | P1 exit, ROSETTE |

### Player 2 Path Coordinates

| Position | Tile (x,y) | Square Type | Notes |
|----------|------------|-------------|-------|
| 1 | (2, 6) | TYPE_A | P2 start |
| 2 | (4, 6) | TYPE_B | P2 start |
| 3 | (6, 6) | TYPE_C | P2 start |
| 4 | (8, 6) | TYPE_B | P2 start, ROSETTE |
| 5-12 | (same as P1) | (same) | Shared section |
| 13 | (14, 6) | TYPE_A | P2 exit |
| 14 | (16, 6) | TYPE_ROSETTE | P2 exit, ROSETTE |

### Lookup Table Implementation

```c
typedef struct {
    uint8_t tile_x;      // Tile column (0-19)
    uint8_t tile_y;      // Tile row (0-9)
    uint8_t square_type; // SQUARE_TYPE_* constant
    uint8_t is_rosette;  // 1 if rosette, 0 otherwise
} BoardSquare_t;

// P1's path (index 0-13 for positions 1-14)
const BoardSquare_t p1_squares[14] = {
    { 2, 2, SQUARE_TYPE_A,       0}, // Pos 1
    { 4, 2, SQUARE_TYPE_B,       0}, // Pos 2
    { 6, 2, SQUARE_TYPE_C,       0}, // Pos 3
    { 8, 2, SQUARE_TYPE_B,       1}, // Pos 4 (rosette)
    { 2, 4, SQUARE_TYPE_EYE_A,   0}, // Pos 5
    { 4, 4, SQUARE_TYPE_C,       0}, // Pos 6
    { 6, 4, SQUARE_TYPE_EYE_B,   0}, // Pos 7
    { 8, 4, SQUARE_TYPE_A,       1}, // Pos 8 (rosette)
    {10, 4, SQUARE_TYPE_C,       0}, // Pos 9
    {12, 4, SQUARE_TYPE_EYE_B,   0}, // Pos 10
    {14, 4, SQUARE_TYPE_B,       0}, // Pos 11
    {16, 4, SQUARE_TYPE_C,       0}, // Pos 12
    {14, 2, SQUARE_TYPE_A,       0}, // Pos 13
    {16, 2, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};

// P2's private squares only (positions 1-4 and 13-14)
// Shared squares (5-12) use same coordinates as P1
const BoardSquare_t p2_private_squares[6] = {
    { 2, 6, SQUARE_TYPE_A,       0}, // Pos 1
    { 4, 6, SQUARE_TYPE_B,       0}, // Pos 2
    { 6, 6, SQUARE_TYPE_C,       0}, // Pos 3
    { 8, 6, SQUARE_TYPE_B,       1}, // Pos 4 (rosette)
    {14, 6, SQUARE_TYPE_A,       0}, // Pos 13
    {16, 6, SQUARE_TYPE_ROSETTE, 1}, // Pos 14 (rosette)
};
```

---

## Tile Graphics System

### Template File

A tile template has been created at `assets/images/raw/board_tiles_template.png`:

- **Size**: 48x96 pixels (3 columns x 6 rows of 16x16 squares)
- **Layout**:
  - Column 0 (x: 0-15): Empty squares
  - Column 1 (x: 16-31): Squares with white piece
  - Column 2 (x: 32-47): Squares with black piece
- **Rows** (top to bottom):
  - Row 0: Type A (simple bordered)
  - Row 1: Type B (dots pattern)
  - Row 2: Type C (diamond pattern)
  - Row 3: Rosette (flower design)
  - Row 4: Eye A (large eye)
  - Row 5: Eye B (eye variant)

### Color Palette (Game Boy 4-color)

```
Color 0: #FFFFFF (white)
Color 1: #C3C3C3 (light gray)
Color 2: #7F7F7F (dark gray)
Color 3: #000000 (black)
```

### png2asset Conversion

After the template is complete with pieces drawn:

```bash
# Convert to C arrays (tiles only, no map needed)
png2asset assets/images/raw/board_tiles.png \
    -o assets/generated/board_tiles_pieces.c \
    -tiles_only \
    -noflip
```

This generates:
- 18 tile sets (6 types x 3 states)
- Each tile set is 4 tiles (2x2 for 16x16 pixel square)
- Total: 72 tiles x 16 bytes = 1152 bytes

### Tile Index Lookup

After conversion, create a lookup array:

```c
// Tile indices for each (square_type, piece_state) combination
// Each entry is 4 tile indices for the 2x2 block
// Order: top-left, top-right, bottom-left, bottom-right
extern const uint8_t piece_tiles[6][3][4];

// Example access:
// piece_tiles[SQUARE_TYPE_A][PIECE_WHITE][0] = top-left tile index
// piece_tiles[SQUARE_TYPE_A][PIECE_WHITE][1] = top-right tile index
// etc.
```

---

## Drawing Functions

### Draw Single Square

```c
/**
 * Draw a single board square with optional piece
 * @param tile_x  Left tile column (0-18, must be even for 2x2 alignment)
 * @param tile_y  Top tile row (0-8, must be even for 2x2 alignment)
 * @param square_type  SQUARE_TYPE_* constant (0-5)
 * @param piece_state  PIECE_NONE, PIECE_WHITE, or PIECE_BLACK
 */
void draw_board_square(uint8_t tile_x, uint8_t tile_y,
                       uint8_t square_type, uint8_t piece_state) {
    const uint8_t *tiles = piece_tiles[square_type][piece_state];

    // Wait for VBlank before VRAM write
    wait_vbl_done();

    // Draw 2x2 tile block
    set_bkg_tile_xy(tile_x,     tile_y,     tiles[0]);
    set_bkg_tile_xy(tile_x + 1, tile_y,     tiles[1]);
    set_bkg_tile_xy(tile_x,     tile_y + 1, tiles[2]);
    set_bkg_tile_xy(tile_x + 1, tile_y + 1, tiles[3]);
}
```

### Update Entire Board

```c
/**
 * Redraw all board squares based on current piece positions
 * Call after any piece moves, captures, or game state changes
 */
void update_board_display(void) {
    // Draw P1 private squares (positions 1-4)
    for (uint8_t pos = 1; pos <= 4; pos++) {
        uint8_t piece = get_piece_at(pos, PLAYER_1);
        BoardSquare_t sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw P1 exit squares (positions 13-14)
    for (uint8_t pos = 13; pos <= 14; pos++) {
        uint8_t piece = get_piece_at(pos, PLAYER_1);
        BoardSquare_t sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw shared squares (positions 5-12) - check both players
    for (uint8_t pos = 5; pos <= 12; pos++) {
        uint8_t piece = get_piece_at_shared(pos);
        BoardSquare_t sq = p1_squares[pos - 1];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }

    // Draw P2 private squares (positions 1-4, 13-14)
    for (uint8_t i = 0; i < 6; i++) {
        uint8_t pos = (i < 4) ? (i + 1) : (i + 9); // 1-4 or 13-14
        uint8_t piece = get_piece_at(pos, PLAYER_2);
        BoardSquare_t sq = p2_private_squares[i];
        draw_board_square(sq.tile_x, sq.tile_y, sq.square_type, piece);
    }
}

/**
 * Get piece state at a position for a specific player's private section
 */
uint8_t get_piece_at(uint8_t pos, uint8_t player) {
    uint8_t *pieces = (player == PLAYER_1) ? human_pieces : cpu_pieces;
    uint8_t color = (player == PLAYER_1) ? human_color : cpu_color;

    for (uint8_t i = 0; i < 7; i++) {
        if (pieces[i] == pos) {
            return (color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    return PIECE_NONE;
}

/**
 * Get piece state at a shared position (5-12) - either player could have piece here
 */
uint8_t get_piece_at_shared(uint8_t pos) {
    // Check human pieces
    for (uint8_t i = 0; i < 7; i++) {
        if (human_pieces[i] == pos) {
            return (human_color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    // Check CPU pieces
    for (uint8_t i = 0; i < 7; i++) {
        if (cpu_pieces[i] == pos) {
            return (cpu_color == SIDE_LIGHT) ? PIECE_WHITE : PIECE_BLACK;
        }
    }
    return PIECE_NONE;
}
```

---

## VRAM Layout for Piece Tiles

Update `include/vram_layout.h` to allocate space for piece tiles:

```c
// Game screen tile allocation (screen-isolated)
#define VRAM_BOARD_TILES_START    0    // Board with pieces: 72 tiles (0-71)
#define VRAM_BOARD_TILES_COUNT    72   // 6 types x 3 states x 4 tiles
#define VRAM_BOARD_TILES_END      71

#define VRAM_PORTRAIT_START       72   // Opponent portrait: 25 tiles (72-96)
#define VRAM_PORTRAIT_COUNT       25
#define VRAM_PORTRAIT_END         96

// Font tiles remain at 140+ (shared across screens)
```

---

## Move Validation

```c
/**
 * Check if a move is valid
 * @param player  PLAYER_1 or PLAYER_2
 * @param piece_idx  Index of piece to move (0-6)
 * @param roll  Dice roll result (1-4, 0 means no move possible)
 * @return 1 if valid, 0 if invalid
 */
uint8_t is_valid_move(uint8_t player, uint8_t piece_idx, uint8_t roll) {
    if (roll == 0) return 0;  // Can't move on 0 roll

    uint8_t *pieces = (player == PLAYER_1) ? human_pieces : cpu_pieces;
    uint8_t *opponent = (player == PLAYER_1) ? cpu_pieces : human_pieces;
    uint8_t current_pos = pieces[piece_idx];
    uint8_t new_pos = current_pos + roll;

    // Can't move finished pieces
    if (current_pos == POS_FINISHED) return 0;

    // Moving from reserve onto board
    if (current_pos == POS_RESERVE) {
        new_pos = roll;  // Start at position = roll value
    }

    // Can't go past finish
    if (new_pos > POS_FINISHED) return 0;

    // Exact roll needed to finish (optional rule - check PLAN.md)
    if (new_pos > 14 && new_pos != POS_FINISHED) return 0;

    // Can't land on own piece
    for (uint8_t i = 0; i < 7; i++) {
        if (pieces[i] == new_pos && i != piece_idx) return 0;
    }

    // Can't capture on rosette
    if (is_rosette(new_pos)) {
        for (uint8_t i = 0; i < 7; i++) {
            if (opponent[i] == new_pos) return 0;  // Opponent on rosette = safe
        }
    }

    return 1;
}

/**
 * Check if position is a rosette
 */
uint8_t is_rosette(uint8_t pos) {
    return (pos == 4 || pos == 8 || pos == 14);
}
```

---

## File Organization

New/modified files for Phase 8b:

```
include/
├── board_state.h      # NEW: BoardSquare_t, position constants, lookup tables
├── vram_layout.h      # MODIFY: Add piece tile allocation
└── game.h             # MODIFY: Add piece array externs if needed

src/
├── board_state.c      # NEW: Lookup tables, drawing functions, validation
└── game.c             # MODIFY: Add piece arrays, call update_board_display()

assets/
├── images/raw/
│   ├── board_tiles_template.png  # Template for artist (exists)
│   └── board_tiles.png           # Completed tiles with pieces (TODO)
└── generated/
    └── board_tiles_pieces.c      # Generated tile data (after png2asset)
```

---

## Implementation Checklist

- [ ] Artist completes `board_tiles.png` with white/black pieces
- [ ] Run png2asset to generate `board_tiles_pieces.c`
- [ ] Create `board_state.h` with types and constants
- [ ] Create `board_state.c` with lookup tables and functions
- [ ] Update `vram_layout.h` with new tile allocation
- [ ] Add piece position arrays to `game.c`
- [ ] Initialize pieces in `init_game()`
- [ ] Implement `update_board_display()`
- [ ] Implement move validation functions
- [ ] Call `update_board_display()` after each move
- [ ] Update Makefile with new source files
- [ ] Test with random moves (Phase 8b deliverable)
