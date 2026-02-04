# Royal Game of Ur - Game Boy DMG Development Plan

## Overview

This plan breaks down the development into manageable phases, each building on the previous one. Each phase results in a testable milestone. The plan assumes familiarity with GBDK-2020 and Game Boy hardware constraints (4 colors, 8x8 tiles, 40 sprites max, etc.).

## Phase 12: Link Cable Support

Add two-player link cable support. Players connect via Game Boy link cable, get assigned colors (light/dark), select profile pictures visible to the opponent, then play against each other.

### Phase 12A: Serial Communication Foundation

**New files:** `src/link.c`, `include/link.h`

**Tasks:**
1. Initialize serial hardware (SC and SB registers)
2. Implement master/slave detection (first to send sync byte becomes master)
3. Create send/receive byte functions with timeout handling
4. Implement handshake protocol for connection establishment
5. Add connection status tracking (disconnected, connecting, connected, error)

**Protocol constants:**
```c
#define LINK_SYNC_BYTE    0xAA
#define LINK_ACK_BYTE     0x55
#define LINK_TIMEOUT      60    // Frames (~1 second)
```

### Phase 12B: Connection Screen

**New files:** `src/link_connect.c`, `include/link_connect.h`

**New state:** `STATE_LINK_CONNECT`

**Screen flow:**
1. Display "WAITING FOR CONNECTION..." with animated dots
2. Both players press A to initiate handshake
3. Master/slave determined by timing (first sender = master)
4. Master assigned LIGHT pieces, slave assigned DARK pieces
5. Exchange confirmation bytes
6. Display "CONNECTED!" briefly, then transition to profile selection

**UI layout:**
```
    LINK CABLE MODE

    WAITING FOR
    CONNECTION...

    PRESS A TO CONNECT

    [Cancel: B]
```

### Phase 12C: Link Profile Selection

**New files:** `src/link_profile.c`, `include/link_profile.h`

**New state:** `STATE_LINK_PROFILE_SELECT`

**Screen flow:**
1. Reuse opponent select screen layout (2x2 portrait grid with 4 profiles)
2. Each player independently selects their profile picture
3. Send selected profile index (0-3) to other player when A pressed
4. Wait for opponent's selection (show "WAITING..." after own selection)
5. Display both selections briefly ("YOU: [name]" / "OTHER: [name]")
6. Transition to game state

### Phase 12D: Link Game Mode

**Modified files:** `src/game.c`, `include/game.h`, `src/board_state.c`

**Changes:**
1. Add game mode enum:
   ```c
   typedef enum { GAME_MODE_SINGLE, GAME_MODE_LINK } GameMode_t;
   ```

2. Replace "CPU" label with "OTHER" when `game_mode == GAME_MODE_LINK`
   - Position stays at tile (1, 11)

3. Turn management:
   - Local player controls their assigned color
   - Remote player's turn: wait for move data via link
   - Send dice roll result to opponent
   - Send selected move (piece_index, destination) to opponent

4. Each player sees board with their pieces on bottom row (same as single player perspective)

5. Load and display remote player's selected profile portrait

6. Disable AI when in link mode

**Synchronization per turn:**
```
Active player:
  1. Roll dice locally
  2. Send dice result to opponent
  3. Select move
  4. Send piece_index and destination
  5. Wait for acknowledgment

Passive player:
  1. Receive dice result, display it
  2. Receive move data
  3. Apply move to local board
  4. Send acknowledgment
```

### Phase 12E: Disconnection Handling & Game End

**Modified files:** `src/endgame.c`, `src/link.c`

**Disconnection handling:**
- Timeout detection (no response within 60 frames)
- Display "CONNECTION LOST" message
- Return to title screen automatically

**Game end:**
- Winner sends WIN signal, loser sends LOSE signal
- Both confirm game end synchronously
- Display "YOU WON!" / "YOU LOST!" accordingly
- "PRESS A FOR NEW GAME" returns both to title

### Link Protocol Byte Format

| Byte Range | Meaning |
|------------|---------|
| 0xAA | Sync/handshake request |
| 0x55 | Acknowledge |
| 0x00-0x03 | Profile selection (index 0-3) |
| 0x10-0x14 | Dice roll result (value 0-4) |
| 0x20-0x2F | Move piece index (0x20 + idx) |
| 0x30-0x3F | Move destination (0x30 + pos) |
| 0xF0 | No valid moves signal |
| 0xFE | Game end - I won |
| 0xFF | Game end - I lost |

### State Flow (Link Mode)

```
[TITLE] → "LINK CABLE"
    ↓
[LINK_CONNECT] → connection established
    ↓
[LINK_PROFILE_SELECT] → both profiles exchanged
    ↓
[GAME] (game_mode = GAME_MODE_LINK)
    ↓
[ENDGAME] → Press A
    ↓
[TITLE]
```

### File Changes Summary

**New files:**
- `src/link.c` / `include/link.h` - Serial communication driver
- `src/link_connect.c` / `include/link_connect.h` - Connection screen
- `src/link_profile.c` / `include/link_profile.h` - Link profile selection

**Modified files:**
- `include/game_types.h` - Add STATE_LINK_CONNECT, STATE_LINK_PROFILE_SELECT, GameMode_t
- `src/main.c` - Add new state handlers in main loop
- `src/title.c` - Update LINK_CABLE to transition to STATE_LINK_CONNECT
- `src/game.c` / `include/game.h` - Link mode logic, "OTHER" label
- `src/endgame.c` - Handle link mode end game
- `Makefile` - Add new source files and dependencies

### Testing with Emulators

#### BGB (Recommended)

1. **Setup two instances:**
   - Open BGB → Options → Link → "Listen" (note port, default 8765)
   - Open second BGB → Options → Link → "Connect" → localhost:8765

2. **Test workflow:**
   - Load `royal-ur.gb` in both
   - Select "LINK CABLE" on both title screens
   - Press A on both to connect
   - Select profiles, play game
   - Test disconnection by closing one instance

#### SameBoy

1. **Two instances:**
   - First: File → Connect Link Cable → Listen
   - Second: File → Connect Link Cable → Connect to localhost

2. **Or use Buddy Mode:**
   - File → Open Buddy ROM → Load same ROM
   - Both Game Boys in split view

### Test Checklist

- [ ] Connection establishes when both press A
- [ ] Master gets LIGHT, slave gets DARK
- [ ] B button cancels and returns to title
- [ ] Profile selection syncs correctly
- [ ] Dice rolls display on both Game Boys
- [ ] Moves apply correctly on both sides
- [ ] Captures and rosette bonuses work
- [ ] Game end displays correct winner/loser
- [ ] Disconnection detected and handled gracefully


## Phase 13: Add Sound

Background music needs to be added.

Sound effects need to be added to dice rolls, selecting menu items (difficulty, opponent)

A happy or sad chime should sound upon winning or losing.

### Assets

- Sounds and background music made with hUGEtracker


## Phase 14: Happy - Sad profile animations

We'll add some animations to the opponents profile, if they capture a piece or finish with a piece they should
briefly look happy. If a piece of them is captured by the human player or the human finishes a piece they should briefly look sad. 

On the endgame screen, if the opponent has won their profile image should altenate between normal and happy. If they lost it should alternate between normal and sad

### Assets

- 4 opponent happy variants
- 4 opponent sad variants

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
