# CLAUDE.md - Royal Game of Ur (Game Boy DMG)

## Project Overview

A Game Boy DMG ROM implementing the Royal Game of Ur, built with GBDK-2020. This is a single-player strategy board game with AI opponents.

## Development Approach

**Follow PLAN.md phases strictly.** Each phase builds on the previous and results in a testable ROM. Do not skip ahead or combine phases without explicit instruction.

## Build System

### Toolchain
- **GBDK-2020** (ensure it's installed and `$GBDK_HOME` is set, or adjust Makefile paths)
- `lcc` for compilation
- `png2asset` for graphics conversion

### Build using the build-validator agent

Always use the build-validator agent to test building the rom! The agent will run the commands below and report if the build was successfull. If not it will point out the issue that needs to be tackled.

```bash
make          # Build the ROM
make clean    # Remove build artifacts
make run      # Build and launch in emulator (if configured)
```

### Testing

Ask the user the run the rom, they will use SameBoy to test

## Code Style

### General Principles
- Prioritize **readability over cleverness**
- Use descriptive variable and function names
- Comments explain *why*, code shows *what* and *how*

### Naming Conventions
```c
// Functions: verb_noun, lowercase with underscores
void load_title_screen(void);
uint8_t get_valid_moves(uint8_t roll);

// Variables: lowercase with underscores
uint8_t current_player;
uint8_t selected_piece_index;

// Constants/Defines: UPPERCASE with underscores
#define MAX_PIECES 7
#define ROSETTE_BONUS 1

// Types: PascalCase with _t suffix
typedef struct GameState_t { ... } GameState_t;
typedef enum ScreenState_t { ... } ScreenState_t;
```

### File Organization
- One module per logical unit (see PLAN.md file structure)
- Header files contain declarations, `.c` files contain implementations
- Keep functions short and focused (under 50 lines when possible)

### Game Boy Specific
```c
// Always wait for VBlank before VRAM writes
void vblank_wait(void) {
    while (LY_REG != 144);  // Wait for VBlank
    while (LY_REG == 144);  // Wait for it to end (ensures full VBlank)
}

// Use GBDK types for size clarity
uint8_t small_value;    // 0-255
int8_t signed_small;    // -128 to 127
uint16_t larger_value;  // 0-65535
```

## Hardware Constraints

### Memory Limits
- **VRAM Tiles:** 256 background tiles + 256 sprite tiles
- **OAM:** 40 sprites maximum, 10 per scanline
- **WRAM:** ~8KB available
- **ROM:** Starts at 32KB, can use MBC for more if needed

### Performance
- CPU is ~1MHz effective; avoid heavy computation during active display
- Do AI calculations during VBlank or spread across frames
- Use lookup tables instead of runtime calculation where possible

### Graphics
- 4 colors only (or 4 shades of green on DMG)
- Tiles are 8x8 pixels
- Sprites can be 8x8 or 8x16 (mode set globally)

## Asset Pipeline

### Directory Structure
```
assets/
├── tiles/           # Source PNGs for background tiles
├── sprites/         # Source PNGs for sprite graphics
├── maps/            # Tilemap data (if hand-authored)
└── generated/       # Output from png2asset (DO NOT EDIT)
```

### Converting Graphics
```bash
# Example: Convert title screen
png2asset assets/tiles/title.png -o assets/generated/title.c -map -tiles_only

# Example: Convert sprite sheet
png2asset assets/sprites/pieces.png -o assets/generated/pieces.c -sw 8 -sh 8
```

Document exact `png2asset` commands used in comments at the top of generated files or in a separate `assets/README.md`.

## State Machine

The game uses a central state machine. All screens are states:

```c
typedef enum {
    STATE_TITLE,
    STATE_OPPONENT_SELECT,
    STATE_DIFFICULTY_SELECT,
    STATE_COINFLIP,
    STATE_GAME,
    STATE_ENDGAME
} ScreenState_t;
```

Each state has:
- `init_*()` - Called once when entering the state
- `update_*()` - Called every frame while in the state
- `cleanup_*()` - Called when leaving the state (optional)

## Game Rules Reference

### Royal Game of Ur Basics
- 2 players, 7 pieces each
- 4 binary dice (tetrahedral), roll 0-4
- First to bear off all 7 pieces wins
- **Rosette squares:** Landing grants another turn; pieces are safe from capture
- **Capture:** Landing on opponent's piece sends it back to start (not on rosettes)
- **Exact bearing off:** Must roll exact or higher to bear off from final squares

### Board Layout (Path)
```
Player 1 start  →  [1][2][3][4]        [13][14]  →  Player 1 finish
                        ↓                  ↑
              [5][6][7][8*][9][10][11][12]
                        ↓                  ↑
Player 2 start  →  [1][2][3][4]        [13][14]  →  Player 2 finish

* = Rosette (squares 4, 8, 14 are rosettes in standard rules)
```

## AI Implementation Notes

The AI will be ported from Python. Key considerations:

- **Easy:** Random valid move
- **Medium:** 1-ply evaluation (pick best immediate outcome)
- **Hard:** Expectiminimax with depth limit (account for dice probability)

### Evaluation Heuristics
- Piece advancement (further = better)
- Rosette control
- Capture opportunities
- Blocking opponent
- Bearing off progress

### Performance Budget
- AI must complete within ~0.5 seconds perceived time
- Use iterative deepening if needed
- Precompute what you can

## Common Pitfalls

1. **Writing to VRAM outside VBlank** - Causes corruption. Always wait.
2. **Too many sprites on one line** - Only 10 visible; others disappear.
3. **Forgetting to call `wait_vbl_done()`** - Use GBDK's helper or manual wait.
4. **Large switch statements** - Can generate slow code; use function pointers or lookup tables.
5. **Signed vs unsigned math** - Be explicit; Game Boy is 8-bit and overflow matters.

## Phase Checklist

When completing each phase, verify:

- [ ] ROM compiles without warnings
- [ ] Feature works in emulator
- [ ] No visual glitches or tearing
- [ ] Input is responsive
- [ ] Transitions to next state work correctly
- [ ] Code follows style guidelines
- [ ] New assets documented in asset pipeline section

## Resources

- [GBDK-2020 Documentation](https://gbdk-2020.github.io/gbdk-2020/docs/api/)
- [Pan Docs (GB Hardware Reference)](https://gbdev.io/pandocs/)
- [GB ASM Tutorial (concepts apply)](https://gbdev.io/gb-asm-tutorial/)
- [png2asset Usage](https://gbdk-2020.github.io/gbdk-2020/docs/api/docs_toolchain_settings.html)

## Questions for Human

Before starting implementation, clarify with the human:

1. Do they have GBDK-2020 installed? What's the path?
2. Can they provide the title screen image?
3. What are the 4 opponent characters (names, play styles)?
4. Can they provide the Python AI code for porting?
5. Preferred emulator for testing?
