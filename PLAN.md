# Royal Game of Ur - Game Boy DMG Development Plan

## Overview

This plan breaks down the development into manageable phases, each building on the previous one. Each phase results in a testable milestone. The plan assumes familiarity with GBDK-2020 and Game Boy hardware constraints (4 colors, 8x8 tiles, 40 sprites max, etc.).

---

## Completed Phases

### Phase 1: Project Setup & Title Screen (Static) ✓
- Project scaffolding with GBDK-2020 toolchain and Makefile
- Title screen background with tileset and tilemap
- Menu navigation with arrow sprite between "Start Game" and "Link Cable"

### Phase 2: Title Screen Animation ✓
- Sprite-based specular highlight animation across title letters
- Seamless looping shine effect

### Phase 3: Opponent Selection Screen ✓
- 2x2 grid layout with 4 opponent portraits
- Description box updates on selection change
- Navigation and confirmation with screen transitions

### Phase 4: Difficulty Selection Screen ✓
- Vertical menu with Easy/Medium/Hard options
- B returns to opponent selection, A proceeds to coin flip
- Difficulty stored in game state for AI reference

---

## Phase 5: Side Selection & Coin Flip

**Goal:** Player chooses Light (sun) or Dark (moon) side, then coin flip determines who moves first.

### Screen Layout

```
┌────────────────────────────────────┐
│         "CHOOSE YOUR SIDE"         │  <- Title text
│                                    │
│    ┌──────────┐    ┌──────────┐    │
│    │   SUN    │    │   MOON   │    │  <- 40x40 coin graphics
│    │  (Light) │    │  (Dark)  │    │     reused from animation
│    └──────────┘    └──────────┘    │
│         ▲                          │  <- Selection border on current
│────────────────────────────────────│
│                                    │
│       [Coin flip animation]        │  <- 40x40 coin flip plays here
│         "LIGHT STARTS" or          │     after A is pressed
│         "DARK STARTS"              │
│                                    │
└────────────────────────────────────┘
        White background throughout
```

### Tasks

1. **Screen layout**
   - White background (all tiles use lightest shade)
   - "CHOOSE YOUR SIDE" title text at top
   - Two coin options side by side on top half: Sun (Light) and Moon (Dark)
   - Use the flat sun/moon frames from the coin animation (reuse assets)
   - Selection border drawn around currently highlighted coin
   - Bottom half reserved for coin flip animation and result text

2. **Navigation**
   - Left/Right D-pad switches selection between Sun and Moon
   - Selection border moves to indicate current choice
   - B returns to difficulty selection screen
   - A confirms selection and triggers coin flip

3. **Coin flip animation**
   - After A pressed, animate spinning coin in bottom half of screen
   - Use 8-frame rotation sequence (sun → edge → moon → edge → repeat)
   - Animation runs for ~2-3 seconds with gradual slowdown
   - Use DIV register or input timing for random outcome

4. **Result display**
   - Coin lands on sun or moon (random)
   - Display "LIGHT STARTS" or "DARK STARTS" text below coin
   - If landed side matches player's choice, they move first
   - Brief pause to show result

5. **Transition (placeholder for now)**
   - After result shown, placeholder transition (e.g., flash screen)
   - Will connect to game board in Phase 6

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Sun/Moon coin frames | 40x40 PNG, 4 colors | 8 frames: sun flat, sun angled, edge, moon angled, moon flat, moon angled, edge, sun angled |
| Selection border tiles | 8x8 tiles | Corner and edge pieces to highlight selected coin |

**Note:** The same coin artwork is reused for both the selection display (frames 1 and 5) and the flip animation (all 8 frames). Single asset, multiple uses.

### Technical Notes

- **White background:** Set all background tiles to color 0 (white `#FFFFFF` on DMG)
- **Asset reuse:** Sun flat = frame 1, Moon flat = frame 5 from animation sequence
- **Selection border:** Can reuse border tiles from opponent selection screen
- **VBlank updates:** Swap coin tiles during VBlank for animation (~1.1ms available)
- **Randomness:** Sample DIV register at moment of A press, or use frame counter

### Deliverable

Player can select Light or Dark side with visual feedback. Coin flip animation determines starting player. B returns to difficulty. A triggers flip with placeholder transition to next phase.

---

## Phase 6: Game Board Display (Static)

**Goal:** Draw the Royal Game of Ur board and piece reserves.

### Tasks

1. **Board layout design**
   - Map the Ur board shape to tile positions (the distinctive H-shape)
   - Design tiles for: empty square, rosette square, board edges
   - Position board centered on screen

2. **Reserve areas**
   - Left side: space to show unplayed pieces
   - Right side: space to show captured/finished pieces (or use consistent side)

3. **UI elements**
   - Bottom text area for prompts ("Press A to roll")
   - Area for dice display (4 binary dice)

4. **Piece sprites**
   - Light pieces and dark pieces as distinct sprites
   - Load both sets, display based on player assignment

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Board tiles | 8x8 each | Empty, rosette, border variants |
| Light piece sprite | 8x8 or 8x16 | Distinct shape/pattern |
| Dark piece sprite | 8x8 or 8x16 | Visually different from light |
| Dice sprites | 8x8 each | Showing 0 and 1 states |

### Deliverable

Game screen shows complete board, piece reserves, and UI areas (no interaction yet).

---

## Phase 7: Dice Rolling

**Goal:** Implement dice roll mechanic with animation.

### Tasks

1. **Dice data**
   - 4 binary dice (each 50% chance of 0 or 1)
   - Total roll = sum (0-4)

2. **Roll animation**
   - Dice sprites cycle through 0/1 rapidly
   - Settle to final values after ~1 second
   - Display total prominently

3. **Input flow**
   - "Press A to roll" prompt
   - On A press, animate and calculate roll
   - Store result for move selection phase

### Assets Needed

*Already covered in Phase 6*

### Deliverable

Player presses A, dice animate and show result (0-4).

---

## Phase 8: Move Selection (Human Player)

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
- Light/Dark piece icons (for coin flip result)
- Dice × 2 states (0 and 1)
- Hand pointer
- Destination highlight (or use BG tiles)
- Selection border (could be BG tiles instead)

### Backgrounds (BG)
- Title screen (full 160x144)
- Opponent select screen
- Difficulty select screen
- Side selection / Coin flip screen (white background, 40x40 sun/moon coin frames)
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
