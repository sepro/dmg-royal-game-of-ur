# Royal Game of Ur - Game Boy DMG Development Plan

## Overview

This plan breaks down the development into manageable phases, each building on the previous one. Each phase results in a testable milestone. The plan assumes familiarity with GBDK-2020 and Game Boy hardware constraints (4 colors, 8x8 tiles, 40 sprites max, etc.).

## Phase 9: AI Opponent (Basic)

**Goal:** Port Python AI to C and integrate with game loop.

### Tasks


1. **AI port from Python**
   - Convert specific AI approaches from python to C
   - Each opponent will use a different AI
     - THE MERCHANT should be similar to greedy_agent.py
     - THE MUSICIAN should be similar to turn_economy_agent.py
     - THE SCHOLAR should be similar to the adaptive_agent.py
     - THE PRIESTESS should be similar to the phase_based_agent.py
   - A full Python program is included in ./python
      - This also contains optimized weights for some of the agents
   - Agent Code is in ./python/src/agents


2. **Difficulty scaling**
   - Easy: When multiple moves are possible, pick the one the AI favors 40% of the time
   - MEDIUM: When multiple moves are possible, pick the one the AI favors 65% of the time
   - HARD: Pick the move the AI favors

3. **AI turn execution**
   - Calculate best move
   - Show AI "thinking" indicator briefly (if needed)

### Assets Needed

| Asset | Format | Notes |
|-------|--------|-------|
| "Thinking" indicator | Sprite or tiles | Optional: animated dots or similar |

### Deliverable

AI opponent makes legal, strategic moves. Difficulty affects play strength.

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
