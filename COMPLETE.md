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

### Phase 8c: Move Selection (Human Player) ✓
- Valid move calculation determining which pieces can legally move based on dice roll
- Selection sprite (square border) positioned over selectable pieces
- Left/right navigation cycling through valid pieces only
- Destination preview showing blinking piece sprite on target square
- A button confirms and executes the selected move
- Support for entering pieces from reserve with visual indicator
- Handles entering board, normal movement, capturing, rosette rules, and bearing off

### Phase 10: Game Loop ✓
- Turn management alternating between human and CPU players
- Rosette bonus turns handled correctly (landing on rosette grants extra turn)
- Roll of 0 automatically passes turn with "NO MOVES" message
- Win detection tracking pieces borne off (first to 7 wins)
- End game sequence triggered on victory (transitions to endgame screen)
- Move legality enforced: cannot land on own piece, captures allowed except on rosettes
- Rosette squares provide safety from capture

### Phase 11: End Game Screen ✓
- Win/lose detection and state communication from game screen
- White background screen with inverted font text
- Opponent portrait display (5x5 tiles) with decorative border frame
- Win screen displays: "YOU WON !", opponent portrait, "YOU BEAT [NAME]"
- Lose screen displays: "YOU LOST !", opponent portrait
- Border reuses tiles from pause screen and selection screens (7x7 frame)
- A button returns to opponent selection for new game
- Layout: result text at row 1 (offset right), portrait at rows 4-8 with border at rows 3-9

---