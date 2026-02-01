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

### Phase 5: Side Selection & Coin Flip ✓
- Light/Dark coin selection with border highlight navigation
- Tile-flipping coin flip animation with chaotic and lock-in phases
- Result display determines starting player based on player's choice
- Flash transition to game state (placeholder)

### Phase 6: Game Board Display (Static) ✓
- H-shaped board layout with empty, rosette, and border tiles
- Reserve areas for unplayed and finished pieces
- UI elements for prompts and dice display area
- Light and dark piece sprites loaded based on player assignment

### Phase 7: Dice Rolling ✓
- 4 binary dice implementation (total roll 0-4)
- Roll animation with dice cycling through values rapidly
- "Press A to roll" input flow with result display
- Roll result stored for move selection phase

### Phase 8a: Pause Screen ✓
- Turn counter tracking (increments each turn switch)
- Elapsed time tracking in frames (pauses when game is paused)
- Window layer overlay with smooth slide-up/down animation
- Pause screen displays: turn number, time (MM:SS), player scores, opponent name, difficulty
- START button toggles pause state
- Sprites hidden during pause to prevent rendering over window layer

### Phase 8b: Track Game State ✓
- Piece position tracking with arrays for both players (7 pieces each, positions 0-15)
- Board state visualization using tile-based pieces (white/black on various square types)
- Move validation system checking legal moves, captures, and rosette rules
- Random move execution for both players based on dice roll
- Automatic turn progression with rosette bonus turns and capture mechanics
- Board display updates after each move showing current game state

---