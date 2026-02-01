# Royal Game of Ur - Game Boy DMG Development Plan

## Overview

This plan breaks down the development into manageable phases, each building on the previous one. Each phase results in a testable milestone. The plan assumes familiarity with GBDK-2020 and Game Boy hardware constraints (4 colors, 8x8 tiles, 40 sprites max, etc.).



## Phase 8b: Track game state

**Goal:** Implement a way to keep track of pieces on the board.

### Tasks

1. Add two lists, one for the CPU and one for the human player which can be used to keep track where pieces are on the board.
2. Implement a way to check if a move is valid
3. After a dice roll, check which moves are valid (including moving a new piece on the board) and execute a random one, if there are no valid moves the turn is over (this will later be replaced with AI for CPU and selection of the player)
4. Move the selected piece (capturing other pieces if needed or finishing)
5. Draw the board with the current game state. There will be tiles available with a black or white piece on them. These should be used to draw the board with the pieces.

**Note:** While both players will have their own list of 14 positions on the board (1 to 14). This can be one bit (either there is a piece or not, you cannot move to a spot where you already have a piece), but positions 5-12 are shared on the board. Here if a piece would move to a space with an opponents piece that piece is captured and goes back to the reserve pile. However, spot 8 is safe. Furthermore, position 4, 8, 14 are rosette spots, if a piece moves here the current player get's an extra turn.

### Assets needed

Board square images can be found in `assets/images/raw/board_tiles.png`:

- **Size**: 48x96 pixels (3 columns x 6 rows of 16x16 squares)
- **Layout**:
  - Column 0 (x: 0-15): Empty squares
  - Column 1 (x: 16-31): Squares with white piece
  - Column 2 (x: 32-47): Squares with black piece
- **Rows** (top to bottom):
  - Row 0: Rosette
  - Row 1: Type A
  - Row 2: Type B
  - Row 3: Type C
  - Row 4: Type D
  - Row 5: Type E

### Deliverable

The board state should be tracked, each turn based on the roll a random move will be carried out (also for the human player). The board state is correctly visualized. 

## Phase 8c: Move Selection (Human Player)

**Goal:** Allow player to select which piece to move.

### Tasks

1. **Valid move calculation**
   - Given roll value, determine which pieces can legally move
   - Handle: entering board, normal movement, capturing, rosette rules, bearing off

2. **Hand pointer sprite**
   - Create pointing hand sprite
   - Position over first valid piece

3. **Navigation**
   - Left/right cycles through valid pieces only
   - Update hand position accordingly

4. **Destination preview**
   - Calculate where selected piece would land
   - Show blinking rectangle around destination square

5. **Confirm move**
   - A executes the move
   - Animate piece moving to destination
   - Handle captures (remove opponent piece)
   - Check for rosette (grants another turn)

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Hand pointer sprite | 8x16 or 16x16 | Pointing finger |
| Destination highlight | Tiles or sprite | Blinking border effect |

### Deliverable

Human player can see valid moves, select piece, see destination, and execute move.

---

## Phase 9: AI Opponent (Basic)

**Goal:** Port Python AI to C and integrate with game loop.

### Tasks

1. **Game state representation**
   - Define structs for board state, piece positions
   - Functions to copy/modify state for AI search

2. **AI port from Python**
   - Convert expectiminimax or evaluation function to C
   - Implement move generation
   - Implement position evaluation (material, board control, etc.)

3. **Difficulty scaling**
   - Easy: random valid move or 1-ply evaluation
   - Medium: shallow search (2-3 ply)
   - Hard: deeper search with full evaluation

4. **AI turn execution**
   - Calculate best move
   - Show AI "thinking" indicator briefly
   - Animate AI piece moving with clear visual feedback
   - Brief pause so player can see what happened

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| "Thinking" indicator | Sprite or tiles | Optional: animated dots or similar |

### Deliverable

AI opponent makes legal, strategic moves. Difficulty affects play strength.

---

## Phase 10: Game Loop & Win Condition

**Goal:** Complete turn cycle and detect victory.

### Tasks

1. **Turn management**
   - Alternate between human and AI turns
   - Handle rosette bonus turns correctly
   - Handle roll of 0 (no move possible, turn passes)

2. **Win detection**
   - Track pieces borne off for each player
   - First to bear off all 7 pieces wins
   - Trigger end game sequence on victory

3. **Move legality edge cases**
   - No valid moves available (pass turn automatically)
   - Cannot land on own piece
   - Can capture opponent (except on rosette)
   - Rosette squares are safe

### Assets Needed

*None new*

### Deliverable

Full game can be played to completion with correct rule enforcement.

---

## Phase 11: End Game Screen

**Goal:** Show victory/defeat with opponent reaction.

### Tasks

1. **Result screen layout**
   - Show opponent portrait (large version or same as select)
   - Happy expression if AI won
   - Sad expression if player won

2. **Opponent emotion variants**
   - Need happy/sad version of each opponent portrait
   - Or: use sprite overlay for expression change

3. **Menu options**
   - "Rematch" - same opponent, same difficulty, swap colors
   - "New Game" - return to opponent select

4. **Navigation**
   - Up/down to select option
   - A to confirm

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| 4 opponent happy portraits | Same size as select | Victory pose/expression |
| 4 opponent sad portraits | Same size as select | Defeat pose/expression |
| "You Win!"/"You Lose!" text | Tiles or pre-rendered | Celebratory/consolation message |

### Deliverable

Game ends with appropriate screen, player can rematch or start new game.

---

## Phase 12: Polish & Optimization

**Goal:** Refine experience, optimize for hardware.

### Tasks

1. **Screen transitions**
   - Implement fade in/out or other transitions
   - Ensure smooth flow between all screens

2. **Sound effects** (if desired)
   - Dice roll sound
   - Piece move sound
   - Capture sound
   - Victory/defeat jingle

3. **Performance optimization**
   - Ensure AI doesn't cause visible lag
   - Optimize tile/sprite updates to prevent tearing

4. **Bug fixes**
   - Playtest all paths thoroughly
   - Edge cases in rules
   - Memory issues

5. **Link Cable placeholder**
   - Show "Coming Soon" or similar when selected
   - Leave hooks for future implementation

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Sound effects | .c arrays or GBT format | Optional but adds polish |
| Additional transition tiles | 8x8 | For fade effects if using tile-based fade |

### Deliverable

Polished, bug-free single-player experience ready for release.

---

## Asset Summary

### Sprites (OBJ)
- Menu arrow
- Specular highlight (2-3 frames)
- Light piece
- Dark piece
- Dice × 2 states (0 and 1)
- Hand pointer
- Destination highlight (or use BG tiles)
- Selection border (could be BG tiles instead)

### Backgrounds (BG)
- Title screen (full 160x144)
- Opponent select screen
- Difficulty select screen
- Side selection / Coin flip screen (white background, two 40x40 coin images: light/dark)
- Game board screen
- End game screen (or reuse elements)

### Character Art
- 4 opponent portraits (select screen size)
- 4 opponent happy variants
- 4 opponent sad variants

### UI Elements
- Font (if custom)
- Text box borders
- Board tiles (empty, rosette, edges)

---

## File Structure

```
royal-ur-gb/
├── Makefile
├── src/
│   ├── main.c              # Entry point, main loop
│   ├── state.c/h           # Game state machine
│   ├── title.c/h           # Title screen logic
│   ├── select.c/h          # Opponent selection
│   ├── difficulty.c/h      # Difficulty selection
│   ├── coinflip.c/h        # Coin flip sequence
│   ├── game.c/h            # Main game logic
│   ├── board.c/h           # Board representation & rules
│   ├── ai.c/h              # AI opponent logic
│   ├── input.c/h           # Joypad handling
│   ├── graphics.c/h        # Sprite/tile helpers
│   └── endgame.c/h         # Victory/defeat screen
├── include/
│   └── types.h             # Common types and constants
├── assets/
│   ├── tiles/              # Raw tile graphics
│   ├── sprites/            # Raw sprite graphics  
│   ├── maps/               # Tilemap data
│   └── generated/          # Output from png2asset
└── build/                  # Compiled output
```
