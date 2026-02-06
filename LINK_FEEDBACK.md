# Link Cable Implementation - Critical Evaluation

An analysis of the link cable multiplayer code across `link.c`, `link_connect.c`, `link_profile.c`, `game.c`, and `endgame.c`. Each issue includes the root cause, affected code locations, and a concrete fix path.

---

## Critical Issues

### 1. Data loss when sender transmits before receiver is listening

**Severity:** Critical - causes board desync and eventual disconnect

**Problem:** The Game Boy serial port has no hardware buffering. When the master clocks a transfer, if the slave does not have `SC_REG` bit 7 set at that exact moment, the byte is silently lost. The master's `link_game_send()` returns success because the hardware transfer "completes" (bit 7 clears), but `SB_REG` reads `0xFF` -- the slave never saw the data.

This happens in practice after rosette bonuses and turn transitions. Consider:
1. Master executes a move that lands on a rosette (extra turn).
2. Both sides enter `PHASE_ROSETTE_BONUS` and wait `RESULT_PAUSE_FRAMES`.
3. Master's timer expires first (due to frame alignment differences), transitions to `PHASE_WAIT_ROLL`, player rolls, sends `LINK_DICE_TAG + total`.
4. Slave is still in `PHASE_ROSETTE_BONUS` -- not calling `link_game_recv()`, so `SC_REG` bit 7 is clear.
5. Master's `link_exchange()` completes (master always completes since it controls the clock), returns success.
6. Slave finally enters `PHASE_WAIT_ROLL`, starts calling `link_game_recv()` -- but the dice byte is gone.
7. Slave waits 1800 frames (~30 seconds) then disconnects with "LINK LOST".

**Affected code:**
- `game.c:914-918` -- dice send after roll animation
- `game.c:525-528` -- no-moves send
- `game.c:604-608` -- piece move send
- `game.c:1170-1174` -- zero-roll no-moves send

**Fix:** Implement a send-with-acknowledgment pattern. After sending a game data byte, the sender must wait for the receiver to echo or ACK the byte before proceeding. For example:

```
Sender:  loads game_data into SB_REG, waits for exchange
Receiver: loads ACK_GAME (e.g. 0xAA) into SB_REG, waits for exchange
Both:    exchange completes -- sender confirms receiver got the data
```

Alternatively, add a ready-sync step before each data exchange: the sender polls idle bytes until the receiver responds with a "ready for data" byte, then the sender transmits the actual data. This ensures the slave is always listening before the master clocks real data.

---

### 2. Slave-side `link_game_recv()` blocks the game loop for up to 4 frames

**Severity:** Critical - causes visible UI stutter and input lag

**Problem:** `link_game_recv()` on the slave side calls `link_exchange_slave(LINK_IDLE_BYTE, &recv, LINK_TRANSFER_WAIT)` with a 4-frame timeout (`link.c:227`). Inside `link_exchange_slave()`, the function enters a `while` loop calling `wait_vbl_done()` up to 4 times (`link.c:146-157`). This means the game loop is blocked for up to 4 frames (~67ms) on every call.

During the remote player's turn, `link_game_recv()` is called every game loop iteration (`game.c:1133`, `game.c:1187`). Since the function blocks for 4 frames each time, the effective frame rate drops to ~15fps. During this time:
- `input_update()` is not called -- button presses can be missed
- `elapsed_frames` does not increment -- the game timer freezes
- `frame_counter` does not increment -- randomness entropy stalls
- No transition or UI animations run
- The "WAITING..." text/dots animation (if present) freezes

**Affected code:**
- `link.c:141-161` -- `link_exchange_slave()` blocks with `wait_vbl_done()` loop
- `link.c:222-243` -- `link_game_recv()` calls the blocking function
- `game.c:1129-1150` -- PHASE_WAIT_ROLL recv polling
- `game.c:1182-1217` -- PHASE_SHOW_RESULT recv polling

**Fix:** Make `link_game_recv()` truly non-blocking. Instead of calling `link_exchange_slave()` with a multi-frame timeout, use a 0-frame or 1-frame check:

1. On entry to the recv polling loop, set `SB_REG = LINK_IDLE_BYTE` and `SC_REG = 0x80` once.
2. Each frame, call `is_transfer_done()`. If done, read `SB_REG` and restart the listen. If not done, return 0 immediately (no blocking).
3. This way the game loop runs at full 60fps and the slave just checks once per frame whether the master clocked a byte.

The listen state should persist across frames rather than being set up and torn down every call.

---

### 3. Slave-side `link_game_send()` blocks for up to 255 frames (~4.25 seconds)

**Severity:** High - completely freezes the game

**Problem:** `link_game_send()` on the slave side calls `link_exchange_slave(data, NULL, LINK_GAME_TIMEOUT)` where `LINK_GAME_TIMEOUT` is 255 frames (`link.c:212`). The slave loads its data and waits for the master to clock the transfer. If the master is not currently polling (e.g. it's in a dice roll animation, processing a UI update, or in a transition between phases), the slave blocks for the entire timeout.

During this ~4.25 second block:
- The screen is completely frozen
- No input is processed
- No animations play
- The user has no feedback that anything is happening

This occurs whenever the slave player (local) makes a move or rolls dice while the master (remote) isn't actively polling for receive.

**Affected code:**
- `link.c:208-213` -- `link_game_send()` slave path
- `game.c:914-918` -- dice send blocks up to 4.25s on slave
- `game.c:604-608` -- move send blocks up to 4.25s on slave

**Fix:** Split `link_game_send()` into a non-blocking state machine:
1. `link_game_send_start(data)` -- loads `SB_REG` and sets `SC_REG`, returns immediately.
2. `link_game_send_poll()` -- checks `is_transfer_done()`, returns 1 if complete, 0 if still waiting.
3. Add a new game phase (e.g. `PHASE_LINK_SENDING`) that calls `link_game_send_poll()` each frame while showing "SENDING..." to the user. On completion, resume normal game flow.

---

### 4. No validation of received move legality

**Severity:** High - can corrupt board state or crash

**Problem:** When a remote move is received (`game.c:1193-1198`), the code extracts `piece_idx` and directly calls `execute_move(PLAYER_CPU, piece_idx, dice_total)` without checking:
- Whether `piece_idx` is valid (0-6). A corrupted byte could produce index 7+.
- Whether the piece at that index actually has a valid move with the current `dice_total`.
- Whether the destination square is occupied by a friendly piece.

A single corrupted byte means one device executes a different move than intended, and the two boards silently diverge. From that point on, every subsequent move will appear invalid on one side.

**Affected code:**
- `game.c:1193-1198` -- remote move execution, no validation
- `game.c:1135` -- dice tag range check exists but doesn't catch adjacent-range corruption

**Fix:** Before calling `execute_move()`, validate the received move:

```c
// Validate piece_idx is in range
if (piece_idx >= PIECES_PER_PLAYER) {
    handle_link_disconnect();
    return;
}

// Validate the move is actually legal
uint8_t valid[PIECES_PER_PLAYER];
uint8_t n = get_valid_moves(PLAYER_CPU, dice_total, valid);
uint8_t found = 0;
for (uint8_t i = 0; i < n; i++) {
    if (valid[i] == piece_idx) { found = 1; break; }
}
if (!found) {
    handle_link_disconnect();  // Boards are desynced
    return;
}
```

This catches corruption AND silent desync early, rather than letting the boards drift apart.

---

### 5. Timing gap between slave receive attempts allows missed bytes

**Severity:** High - intermittent data loss

**Problem:** The slave's `link_game_recv()` calls `link_exchange_slave()` which sets `SC_REG = 0x80` and polls for 4 frames. When it times out, it sets `SC_REG = 0` (`link.c:159`) and returns. The game loop then returns, the main loop runs `wait_vbl_done()`, and on the next iteration `link_game_recv()` is called again, setting `SC_REG = 0x80` once more.

Between the timeout clearing `SC_REG` and the next call setting it, there is a window of approximately 1 frame where the slave is not listening. If the master clocks a transfer during this gap, the slave's `SC_REG` bit 7 is clear, and the byte is lost. The master receives `0xFF` and its `link_game_send()` returns success.

This is an intermittent failure that depends on exact frame alignment between the two Game Boys, making it hard to reproduce consistently but very real in practice.

**Affected code:**
- `link.c:158-160` -- SC_REG cleared on timeout, creating a deaf window
- `link.c:227` -- `link_game_recv()` slave path re-initializes listen each call

**Fix:** Keep the slave listening continuously. Instead of clearing `SC_REG` on timeout, leave it set and only clear/re-set it when a transfer actually completes. The slave's receive function should:
1. If `SC_REG` bit 7 is clear (not listening): set `SB_REG` and `SC_REG` to start listening.
2. If `SC_REG` bit 7 is set (still listening): check `is_transfer_done()`. If done, read data and return it. If not done, return 0 immediately.
3. Never clear `SC_REG` on timeout -- only clear it after reading received data.

This eliminates the deaf window entirely.

---

### 6. `PHASE_SHOW_RESULT` receives move data with the wrong phase structure

**Severity:** High - architectural issue causing unreliable receive

**Problem:** When `current_turn == 1` and `game_mode == GAME_MODE_LINK`, the code to receive the remote player's move is embedded inside `PHASE_SHOW_RESULT` at `game.c:1182-1217`. This code path is only entered when `result_timer` reaches 0, meaning it runs on exactly one frame before transitioning to a different phase on success.

But if `link_game_recv()` returns 0 (no data yet), the code increments `link_recv_frames` and... stays in `PHASE_SHOW_RESULT` with `result_timer == 0`. On the next frame, it re-enters the `result_timer == 0` branch and tries again. This works as a polling loop, but it has a subtle bug: the "WAITING..." prompt is redrawn every frame (`game.c:1186`), causing unnecessary VRAM writes during active display. More importantly, this code shares `PHASE_SHOW_RESULT` with the single-player result display logic, making the phase semantically overloaded and error-prone.

The code also draws "WAITING..." at `game.c:1186` on every frame the receive fails, which writes to VRAM potentially outside VBlank.

**Affected code:**
- `game.c:1163-1224` -- PHASE_SHOW_RESULT handles both result display AND remote move receive

**Fix:** Add a dedicated `PHASE_LINK_RECV_MOVE` phase that cleanly handles the receive polling:
1. After showing the roll result and `result_timer` expires, transition to `PHASE_LINK_RECV_MOVE`.
2. On entering the new phase, draw "WAITING..." once.
3. Each frame, call `link_game_recv()` once. On success, process the move. On failure, increment timeout counter.
4. This separates the receive logic from the result display logic.

---

## Medium Issues

### 7. 30-second disconnect timeout is excessively long

**Severity:** Medium - poor user experience

**Problem:** `LINK_RECV_TIMEOUT` is 1800 frames (~30 seconds at 60fps, declared at `game.c:107`). If the link cable is physically disconnected during play, the player stares at a frozen "WAITING..." screen for 30 seconds before seeing "LINK LOST". This is compounded by Issue #2 (game loop blocking), which makes the timeout take even longer in real time.

Additionally, due to Issue #2, the slave's 4-frame blocking in `link_game_recv()` means `link_recv_frames` only increments once every 4-5 frames, so the effective timeout is 1800 * 4-5 = 7200-9000 frames (120-150 seconds real time!).

**Affected code:**
- `game.c:107` -- `#define LINK_RECV_TIMEOUT 1800`
- `game.c:1145-1148` -- timeout check in PHASE_WAIT_ROLL
- `game.c:1212-1216` -- timeout check in PHASE_SHOW_RESULT

**Fix:** Reduce `LINK_RECV_TIMEOUT` to 300-600 frames (5-10 seconds). After fixing Issue #2 (non-blocking recv), the counter will increment once per real frame, making the timeout accurate. Also consider adding a visual countdown ("WAITING... 10", "WAITING... 9") so the player knows the game hasn't frozen.

---

### 8. No board state synchronization check

**Severity:** Medium - silent game corruption

**Problem:** Both Game Boys maintain independent board states that should mirror each other. If a single byte is corrupted or lost (see Issues #1, #5), the boards silently diverge. All subsequent moves will be executed on different board states, leading to:
- Pieces appearing on different squares on each device
- Captures that happen on one device but not the other
- One device reaching a win condition while the other doesn't

There is currently no mechanism to detect or correct this divergence.

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

### 9. Ready-sync timeout too tight relative to side-reveal duration

**Severity:** Medium - intermittent connection failure during setup

**Problem:** In `CONNECT_PHASE_SIDE_REVEAL` (`link_connect.c:193-211`), the player sees the side assignment screen for up to 120 frames (2 seconds) or until they press A. They then enter `CONNECT_PHASE_SYNCING` which has a 180-frame (3 second) timeout (`link_connect.h` defines `CONNECT_SYNC_TIMEOUT`).

If Player A presses A immediately (frame 0) and Player B waits the full 120 frames, Player A enters syncing at frame 0 and Player B at frame 120. Player A's sync timeout fires at frame 180, but Player B only started syncing at frame 120. The ready-sync attempts between frames 120-180 must succeed, giving only a 60-frame (1 second) window.

On hardware, the ready-sync exchanges can fail on individual frames (master sends READY, slave not yet listening), so it takes several attempts. A 1-second window may be insufficient on real hardware with frame alignment issues.

**Affected code:**
- `link_connect.c:193-211` -- side reveal with 120-frame duration
- `link_connect.c:214-234` -- syncing with 180-frame timeout
- `link_profile.c:398-419` -- same pattern in profile syncing

**Fix:** Either:
- Increase `CONNECT_SYNC_TIMEOUT` to 360+ frames (6 seconds), or
- Make the side reveal a fixed duration (no A-skip) so both players always enter sync at the same frame, or
- Have the side reveal auto-sync: even during the reveal, start calling `link_ready_sync()` in the background. Transition when both the timer expires AND sync succeeds.

---

### 10. LINK_ERROR status is never set -- dead error handling code

**Severity:** Low - dead code / missing error path

**Problem:** `link_connect.c:163` checks for `link_status == LINK_ERROR`, but `link_connect_step()` in `link.c` never sets `link_status` to `LINK_ERROR`. The handshake state machine only transitions between `LINK_DISCONNECTED`, `LINK_CONNECTING`, and `LINK_CONNECTED`. The `LINK_ERROR` enum value exists in `link.h:35` but is unreachable.

This means the "CONNECTION FAILED" / "PRESS A TO RETRY" UI at `link_connect.c:165-183` is dead code. If a real connection failure occurs (e.g. hardware fault), the handshake just loops forever between master and slave tries.

**Affected code:**
- `link.h:35` -- `LINK_ERROR` defined but never assigned
- `link.c:258-360` -- `link_connect_step()` never sets LINK_ERROR
- `link_connect.c:163-184` -- dead error handling code

**Fix:** Add a maximum retry count to the handshake state machine. After N full master+slave cycles without connecting (e.g. 10 cycles = ~15 seconds), set `link_status = LINK_ERROR`. This makes the existing error handling UI reachable and gives users feedback when no other Game Boy is connected.

---

### 11. `link_cancel()` may not reach the peer

**Severity:** Low - best-effort by design but can leave peer hanging

**Problem:** `link_cancel()` (`link.c:194-202`) sends `LINK_CANCEL_BYTE` once and then calls `link_reset()` regardless. The cancel byte is sent role-appropriately (master with internal clock, slave with external clock), but if the other side is not currently listening (e.g. it's in a blocking send, or between receive calls), the cancel byte is lost.

For the slave-side cancel specifically: `link_exchange_slave(LINK_CANCEL_BYTE, &dummy, LINK_TRANSFER_WAIT)` waits up to 4 frames for the master to clock. If the master is not clocking idle bytes at that moment, the cancel is never sent.

After the failed cancel, `link_reset()` clears all state. The peer continues waiting until its own timeout fires (up to 30 seconds in-game).

**Affected code:**
- `link.c:194-202` -- `link_cancel()`
- `link_connect.c:187-190` -- B-cancel during connection
- `link_profile.c:338-341` -- B-cancel during profile select

**Fix:** Send the cancel byte multiple times across several frames before resetting. For example, send CANCEL 3 times with 1-frame gaps between attempts. This increases the chance the peer catches at least one. Alternatively, after calling `link_cancel()`, transition to a brief "CANCELLING..." state that attempts the cancel over multiple frames before returning to title.

---

### 12. `draw_prompt("WAITING...")` called on every receive-fail frame

**Severity:** Low - unnecessary VRAM writes, potential visual glitch

**Problem:** At `game.c:1186`, inside the remote-move receive loop, `draw_prompt("WAITING...")` is called every frame that `link_game_recv()` returns 0. This writes to VRAM on every frame. While GBDK's `set_bkg_tiles()` is VBlank-safe when called during active display on the DMG (it just may cause tearing), it's unnecessary work.

Combined with Issue #2 (slave blocking), the timing of these VRAM writes becomes unpredictable.

**Affected code:**
- `game.c:1186` -- draw_prompt inside per-frame recv loop

**Fix:** Draw "WAITING..." once when entering the receive-wait state (or when transitioning to a dedicated `PHASE_LINK_RECV_MOVE`), not on every frame.

---

## Architectural Recommendations

### A. Move to an interrupt-driven serial handler

The current polling approach is the root cause of most timing issues. A serial interrupt handler would:
- Receive bytes the instant they arrive (no deaf windows)
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

This would make `link_game_recv()` a simple flag check (truly non-blocking) and eliminate Issues #2, #3, and #5 entirely.

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

This prevents Issue #1 (lost data) and Issue #4 (corrupted data) at the cost of 2-3 exchange cycles per move instead of 1.

### C. Separate link I/O from game phase logic

Currently, `link_game_send()` and `link_game_recv()` are called inline within the game phase state machine (`game.c`). This tightly couples the I/O timing to the game phase transitions. A cleaner architecture would be:

1. A `link_io` module that manages a send queue and receive buffer.
2. The game logic pushes outgoing messages and pulls incoming messages.
3. The I/O module handles retries, timeouts, and framing independently.

This separation would make it easier to add features like sync checks, ACK/NAK, and reconnection without modifying game logic.
