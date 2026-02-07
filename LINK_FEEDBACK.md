# Link Cable Implementation - Remaining Issues

An analysis of the link cable multiplayer code across `link.c`, `link_connect.c`, `link_profile.c`, `game.c`, and `endgame.c`. Each issue includes the root cause, affected code locations, and a concrete fix path.

## Previously Fixed Issues (for reference)

The following issues from the original analysis have been resolved:

- **Data loss when sender transmits before receiver is listening** -- Fixed by `link_pump_recv()` which keeps the slave's serial port armed every frame, eliminating deaf windows during all game phases. Combined with `LINK_SEND_RETRIES=180` (~3 seconds of master retries with busy-wait spin).
- **Slave-side `link_game_recv()` blocks the game loop** -- Fixed by persistent armed state (`slave_recv_armed`) and non-blocking `is_transfer_done()` checks. Game loop runs at full 60fps during recv polling.
- **No validation of received move legality** -- Fixed by bounds-checking `piece_idx` and validating against `get_valid_moves()` in `PHASE_LINK_RECV_MOVE` before calling `execute_move()`.
- **Timing gap between slave receive attempts** -- Fixed by `link_pump_recv()` and persistent `slave_recv_armed` flag. Slave only disarms when a transfer completes and re-arms immediately.
- **`PHASE_SHOW_RESULT` overloaded with recv logic** -- Fixed by dedicated `PHASE_LINK_RECV_MOVE` phase with "WAITING..." drawn once on entry.
- **30-second disconnect timeout** -- Reduced `LINK_RECV_TIMEOUT` from 1800 to 600 frames (~10 seconds). Now accurate since recv is non-blocking.
- **`draw_prompt("WAITING...")` called every frame** -- Fixed by drawing once on transition to `PHASE_LINK_RECV_MOVE`.

---

## Open Issues

### 1. Slave-side `link_game_send()` blocks for up to 255 frames (~4.25 seconds)

**Severity:** High - completely freezes the game

**Problem:** `link_game_send()` on the slave side calls `link_exchange_slave(data, NULL, LINK_GAME_TIMEOUT)` where `LINK_GAME_TIMEOUT` is 255 frames. The slave loads its data and waits for the master to clock the transfer. If the master is not currently polling (e.g. it's in a dice roll animation, processing a UI update, or in a transition between phases), the slave blocks for the entire timeout.

During this ~4.25 second block:
- The screen is completely frozen
- No input is processed
- No animations play
- The user has no feedback that anything is happening

This occurs whenever the slave player (local) makes a move or rolls dice while the master (remote) isn't actively polling for receive.

**Affected code:**
- `link.c` -- `link_game_send()` slave path calls `link_exchange_slave()` with long timeout
- `game.c` -- dice send and move send block up to 4.25s on slave

**Fix:** Split `link_game_send()` into a non-blocking state machine:
1. `link_game_send_start(data)` -- loads `SB_REG` and sets `SC_REG`, returns immediately.
2. `link_game_send_poll()` -- checks `is_transfer_done()`, returns 1 if complete, 0 if still waiting.
3. Add a new game phase (e.g. `PHASE_LINK_SENDING`) that calls `link_game_send_poll()` each frame while showing "SENDING..." to the user. On completion, resume normal game flow.

---

### 2. No board state synchronization check

**Severity:** Medium - silent game corruption

**Problem:** Both Game Boys maintain independent board states that should mirror each other. If a single byte is corrupted or lost, the boards silently diverge. All subsequent moves will be executed on different board states, leading to:
- Pieces appearing on different squares on each device
- Captures that happen on one device but not the other
- One device reaching a win condition while the other doesn't

There is currently no mechanism to detect or correct this divergence. (Move validation now catches some desync cases early by disconnecting when a received move is illegal locally.)

**Affected code:**
- All of `game.c` move execution -- no sync verification anywhere

**Fix:** Implement periodic board state checksums. After every N moves (e.g. every 4 moves or every turn switch), both sides exchange a checksum of their board state:

```c
uint8_t board_checksum(void) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < PIECES_PER_PLAYER; i++) {
        sum ^= human_pieces[i];
        sum ^= cpu_pieces[i] << 4;
    }
    return sum;
}
```

If checksums don't match, show "SYNC ERROR" and disconnect. This at least prevents the game from continuing in a corrupted state, and helps distinguish "link lost" from "data corruption".

---

### 3. Ready-sync timeout too tight relative to side-reveal duration

**Severity:** Medium - intermittent connection failure during setup

**Problem:** In `CONNECT_PHASE_SIDE_REVEAL` (`link_connect.c`), the player sees the side assignment screen for up to 120 frames (2 seconds) or until they press A. They then enter `CONNECT_PHASE_SYNCING` which has a 180-frame (3 second) timeout.

If Player A presses A immediately (frame 0) and Player B waits the full 120 frames, Player A enters syncing at frame 0 and Player B at frame 120. Player A's sync timeout fires at frame 180, but Player B only started syncing at frame 120. The ready-sync attempts between frames 120-180 must succeed, giving only a 60-frame (1 second) window.

On hardware, the ready-sync exchanges can fail on individual frames (master sends READY, slave not yet listening), so it takes several attempts. A 1-second window may be insufficient on real hardware with frame alignment issues.

**Affected code:**
- `link_connect.c` -- side reveal with 120-frame duration
- `link_connect.c` -- syncing with 180-frame timeout
- `link_profile.c` -- same pattern in profile syncing

**Fix:** Either:
- Increase `CONNECT_SYNC_TIMEOUT` to 360+ frames (6 seconds), or
- Make the side reveal a fixed duration (no A-skip) so both players always enter sync at the same frame, or
- Have the side reveal auto-sync: even during the reveal, start calling `link_ready_sync()` in the background. Transition when both the timer expires AND sync succeeds.

---

### 4. LINK_ERROR status is never set -- dead error handling code

**Severity:** Low - dead code / missing error path

**Problem:** `link_connect.c` checks for `link_status == LINK_ERROR`, but `link_connect_step()` in `link.c` never sets `link_status` to `LINK_ERROR`. The handshake state machine only transitions between `LINK_DISCONNECTED`, `LINK_CONNECTING`, and `LINK_CONNECTED`. The `LINK_ERROR` enum value exists in `link.h` but is unreachable.

This means the "CONNECTION FAILED" / "PRESS A TO RETRY" UI in `link_connect.c` is dead code. If a real connection failure occurs (e.g. hardware fault), the handshake just loops forever between master and slave tries.

**Affected code:**
- `link.h` -- `LINK_ERROR` defined but never assigned
- `link.c` -- `link_connect_step()` never sets LINK_ERROR
- `link_connect.c` -- dead error handling code

**Fix:** Add a maximum retry count to the handshake state machine. After N full master+slave cycles without connecting (e.g. 10 cycles = ~15 seconds), set `link_status = LINK_ERROR`. This makes the existing error handling UI reachable and gives users feedback when no other Game Boy is connected.

---

### 5. `link_cancel()` may not reach the peer

**Severity:** Low - best-effort by design but can leave peer hanging

**Problem:** `link_cancel()` sends `LINK_CANCEL_BYTE` once and then calls `link_reset()` regardless. The cancel byte is sent role-appropriately (master with internal clock, slave with external clock), but if the other side is not currently listening, the cancel byte is lost.

For the slave-side cancel specifically: `link_exchange_slave(LINK_CANCEL_BYTE, &dummy, LINK_TRANSFER_WAIT)` waits up to 4 frames for the master to clock. If the master is not clocking idle bytes at that moment, the cancel is never sent.

After the failed cancel, `link_reset()` clears all state. The peer continues waiting until its own timeout fires (up to ~10 seconds in-game).

**Affected code:**
- `link.c` -- `link_cancel()`
- `link_connect.c` -- B-cancel during connection
- `link_profile.c` -- B-cancel during profile select

**Fix:** Send the cancel byte multiple times across several frames before resetting. For example, send CANCEL 3 times with 1-frame gaps between attempts. This increases the chance the peer catches at least one. Alternatively, after calling `link_cancel()`, transition to a brief "CANCELLING..." state that attempts the cancel over multiple frames before returning to title.

---

## Architectural Recommendations

### A. Move to an interrupt-driven serial handler

The polling approach with `link_pump_recv()` has largely mitigated the deaf-window problem, but an interrupt-driven handler would be more robust:
- Receive bytes the instant they arrive (no per-frame polling delay)
- Not block the game loop
- Allow a proper FIFO buffer for incoming data

```c
// Sketch of interrupt-driven approach:
volatile uint8_t link_rx_buf;
volatile uint8_t link_rx_ready;

void serial_isr(void) __interrupt {
    link_rx_buf = SB_REG;
    link_rx_ready = 1;
    // Re-arm for next receive
    SB_REG = LINK_IDLE_BYTE;
    SC_REG = SC_START | SC_CLOCK_EXT;  // For slave
}
```

This would make `link_game_recv()` a simple flag check and would also fix Issue #1 (slave send blocking) since the master could receive via interrupt while the slave blocks.

### B. Add a protocol framing layer

The current protocol sends raw tagged bytes with no framing. A minimal framing layer would add reliability:

```
[TAG] [DATA] [CHECKSUM]
```

Where checksum = TAG ^ DATA. This catches single-bit errors. For critical messages (moves), add an ACK response:

```
Sender:  [MOVE_TAG] [piece_idx] [checksum]
Receiver: validates checksum, executes move, sends [ACK]
Sender:  receives ACK, proceeds
```

This prevents lost data and corrupted data at the cost of 2-3 exchange cycles per move instead of 1.

### C. Separate link I/O from game phase logic

Currently, `link_game_send()` and `link_game_recv()` are called inline within the game phase state machine (`game.c`). This tightly couples the I/O timing to the game phase transitions. A cleaner architecture would be:

1. A `link_io` module that manages a send queue and receive buffer.
2. The game logic pushes outgoing messages and pulls incoming messages.
3. The I/O module handles retries, timeouts, and framing independently.

This separation would make it easier to add features like sync checks, ACK/NAK, and reconnection without modifying game logic.
