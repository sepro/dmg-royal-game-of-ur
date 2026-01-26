# Royal Game of Ur - Game Boy DMG Development Plan

## Overview

This plan breaks down the development into manageable phases, each building on the previous one. Each phase results in a testable milestone. The plan assumes familiarity with GBDK-2020 and Game Boy hardware constraints (4 colors, 8x8 tiles, 40 sprites max, etc.).

---

## [Completed] Phase 1: Project Setup & Title Screen (Static)

**Goal:** Establish project structure, build system, and display a static title screen.

### Tasks

1. **Project scaffolding**
   - Set up GBDK-2020 toolchain and Makefile
   - Create folder structure: `src/`, `assets/`, `include/`, `build/`
   - Implement basic main loop with VBlank wait

2. **Title screen background**
   - Convert title image to Game Boy tile format (use `png2asset` or similar)
   - Load tileset and tilemap into VRAM
   - Display full-screen title image

3. **Menu text and arrow**
   - Add "Start Game" and "Link Cable" text (can be part of background or use window layer)
   - Create arrow sprite for menu selection
   - Implement up/down input to move arrow between options
   - A button advances to next screen (placeholder for now - just flash screen or similar)

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Title screen image | 160x144 PNG, 4 colors | Will be converted to tiles + tilemap |
| Menu arrow sprite | 8x8 or 8x16 PNG | Simple arrow pointing right |

### Deliverable

ROM loads and shows title screen. Player can move arrow between menu options with D-pad. Pressing A on "Start Game" triggers a placeholder transition.

---

## [Completed] Phase 2: Title Screen Animation

**Goal:** Add sprite-based specular highlight animation to title letters.

### Tasks

1. **Identify highlight positions**
   - Determine which tiles/positions on the title need the "shine" effect
   - Plan sprite movement path across the letters

2. **Implement shine sprite**
   - Create small bright sprite (or sprite cluster) for the highlight
   - Animate position moving across title letters in a loop
   - Use sprite palette to ensure it appears as a bright reflection

3. **Animation timing**
   - Use frame counter to control animation speed
   - Ensure animation loops seamlessly while on title screen

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Specular highlight sprite(s) | 8x8 or 8x16 PNG | Bright accent, possibly 2-3 frames |

### Deliverable

Title screen has an animated shine effect moving across the title text.

---

## [Completed] Phase 3: Opponent Selection Screen

**Goal:** Implement character selection with 2x2 grid and description box.

### Tasks

1. **Screen layout**
   - Design background with 4 portrait slots (2x2 grid)
   - Reserve bottom area for description text box
   - Create border/highlight tiles for selection indicator

2. **Character data structure**
   - Define struct for opponent: portrait tiles, name, description, AI parameters
   - Create array of 4 opponents (can use placeholder art initially)

3. **Navigation and selection**
   - Track current selection (0-3) as grid position
   - Move selection border based on D-pad input
   - Update description text when selection changes
   - A confirms selection, stores chosen opponent

4. **Screen transitions**
   - Fade or cut from title to opponent select
   - Fade or cut to difficulty select on confirmation

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| 4 opponent portraits | ~32x32 or 40x40 each | Fits in 2x2 grid with spacing |
| Selection border tiles | 8x8 tiles | Corner and edge pieces for highlight box |
| Description box background | Tiles | Simple bordered text area |
| Font tiles (if custom) | 8x8 per character | Or use GBDK's built-in font |

### Deliverable

Player can navigate 2x2 grid, see descriptions update, and confirm selection.

---

## Phase 4: Difficulty Selection Screen

**Goal:** Simple 3-option menu for difficulty.

### Tasks

1. **Screen layout**
   - Display "Easy", "Medium", "Hard" as vertical list
   - Show arrow or highlight on current selection

2. **Navigation**
   - Up/down to change selection
   - A to confirm and proceed to game
   - B to return to opponent selection

3. **Store settings**
   - Save difficulty level in game state for AI to reference later

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Difficulty screen background | 160x144 or simpler | Can reuse elements from other screens |

### Deliverable

Difficulty can be selected. B returns to previous screen. A proceeds to coin flip.

---

## Phase 5: Coin Flip Animation

**Goal:** Animated coin flip to determine starting player/color.

### Tasks

1. **Coin sprite animation**
   - Create spinning coin sprite sequence
   - Animate flip with timing that builds tension

2. **Random outcome**
   - Use DIV register or input timing for randomness
   - Determine light/dark assignment based on flip result

3. **Result display**
   - Show coin landing on heads/tails
   - Brief text: "You are LIGHT" or "You are DARK"
   - Indicate who moves first

4. **Transition to game**
   - Short delay then load game board screen

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| Coin sprite frames | 8x16 or 16x16 | 4-8 frames for flip animation |
| Light/Dark indicator icons | Small sprites or tiles | Optional: visual for result |

### Deliverable

Coin flips with animation, result shown, game begins with correct player order.

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
- Coin flip (4-8 frames)
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
