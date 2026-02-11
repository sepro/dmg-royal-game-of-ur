# Codebase Review Findings (Senior Developer Pass)

This document highlights the highest-impact maintainability and correctness risks in the current codebase, why they matter, and pragmatic refactoring directions.

## 1) `game.c` is doing too much (high complexity hotspot)

**What I observed**
- `src/screens/game.c` is very large (about 1.5k lines) and owns UI rendering, input flow, turn state transitions, link logic integration, pause menu, portrait animation, dice animation, and win/loss transitions.

**Why this is bad**
- A single high-traffic file becomes a regression magnet: every change has broad blast radius.
- It is hard to reason about behavior and hard to test in isolation.
- Cognitive overhead slows feature work and increases defect probability.

**How to improve**
- Split by domain responsibilities:
  - `game_state_machine.c` (phase transitions)
  - `game_ui.c` (text, HUD, pause window drawing)
  - `game_dice.c` (roll lifecycle and animation)
  - `game_selection.c` (move selection/destination preview)
  - `game_link_adapter.c` (link-specific move transport)
- Keep a thin orchestrator in `game.c`.
- Add a `GameContext` struct passed between modules instead of relying on many globals.

---

## 2) Heavy coupling via cross-module globals (`extern` state sharing)

**What I observed**
- Core game state arrays and flags are defined in one module and consumed through `extern` in others (e.g. piece arrays and colors used by board and AI modules).

**Why this is bad**
- Hidden dependencies make behavior brittle and non-local.
- Modules cannot be unit tested independently.
- Refactoring data layout is expensive because every consumer reaches into internals.

**How to improve**
- Replace direct global access with an explicit state contract:
  - Introduce a `GameState` struct with positions, colors, mode, turn metadata.
  - Pass `const GameState*` for read-only logic (AI, board rendering decisions).
  - Expose mutations via explicit APIs (`apply_move`, `set_turn`, `capture_piece`).
- DMG performance note: this does **not** require costly copying. Keep exactly one global/static `GameState` instance and pass a pointer/reference to modules. That keeps RAM/layout equivalent to the current scattered globals while improving code boundaries.
- If needed, split into hot/cold structs (frequently-updated gameplay fields vs rarely-changed metadata) and use `const` tables for immutable data so the runtime overhead stays negligible.
- This yields stronger boundaries and easier deterministic tests without meaningful frame-time impact on DMG.

---

## 3) Randomness usage is inconsistent and potentially biased

**What I observed**
- `get_random_range()` uses modulo arithmetic.
- Some code paths bypass the shared RNG and use `DIV_REG % n` directly.
- Range API does not guard against invalid bounds (`min > max`).

**Why this is bad**
- Modulo range reduction introduces distribution bias unless divisor is a power of two.
- Mixed entropy sources create inconsistent behavior and make AI/gameplay less reproducible.
- Invalid range parameters can underflow and produce undefined gameplay behavior.

**How to improve**
- Standardize on one RNG interface for all gameplay randomness.
- Implement rejection sampling for unbiased bounded random values.
- Add defensive validation (or assertions) in `get_random_range(min, max)`.
- Optionally add a deterministic seed mode for repeatable debugging.

---

## 4) Blocking waits in gameplay/link paths can freeze responsiveness

**What I observed**
- Link exchange functions include busy-spin and frame-blocking loops.
- Some screen flows use explicit frame delays in-place.

**Why this is bad**
- Blocking logic can stall rendering/input handling and create perceptible jank.
- Timeouts become spread across modules, making behavior hard to tune and reason about.
- Failures in link scenarios can degrade UX sharply.

**How to improve**
- Move link transfer to a non-blocking, frame-pumped state machine everywhere.
- Convert blocking delays into phase timers handled in the normal update loop.
- Centralize timeout constants and expose them through one config header.

---

## 5) Repeated UI helper snippets suggest missing shared utilities

**What I observed**
- Identical or near-identical helper data/functions are repeated across files (e.g. blank/white tile arrays, local rectangle-clearing helpers).

**Why this is bad**
- Duplication causes drift: one fix often misses other copies.
- Increases maintenance cost and code size.

**How to improve**
- Promote common utilities to `src/util/screen_utils.c` / `include/util/screen_utils.h`:
  - `load_blank_tile(index)`
  - `clear_rect(tile, x, y, w, h)`
  - reusable border/text helper wrappers
- Keep per-screen files focused on screen-specific behavior.

---

## 6) Testing strategy appears manual-only

**What I observed**
- No clear automated test harness is present for core logic modules.

**Why this is bad**
- Logic-heavy code (move validation, capture rules, AI scoring) is vulnerable to regressions.
- Refactoring safely becomes difficult.

**How to improve**
- Add host-side tests for pure logic modules (`board_state`, AI scoring helpers).
- Start with deterministic table-driven tests:
  - move validity edge cases
  - capture/no-capture on rosettes
  - bear-off exact-roll behavior
  - AI decision sanity in known board states
- Add at least one CI target that runs these tests on every PR.

---

## DMG ROM automated testing procedure (practical implementation plan)

### A. Split tests by level
1. **Host-side logic tests (fast, deterministic)**
   - Compile logic modules with a host compiler (`gcc`/`clang`) using a small compatibility layer that stubs GB-only APIs.
   - Scope: `board_state` rules, AI evaluation/decision, RNG properties.
   - Benefit: very fast feedback and great branch coverage.

2. **ROM integration tests in emulator (hardware-accurate behavior)**
   - Build dedicated test ROM targets (e.g., `make test-rom`).
   - Run in a scriptable emulator (e.g., SameBoy headless mode, or BGB with automation) in CI.
   - Assert pass/fail via serial output and emulator exit code.

3. **Golden-frame/snapshot tests (UI regressions)**
   - For stable scenes (title, opponent select, board setup), boot ROM with deterministic seed/input and capture frame buffers.
   - Compare against checked-in golden PNGs with small tolerance.

### B. Add a tiny in-ROM test harness
- Create `tests/rom_test_main.c` that:
  - runs test cases in sequence,
  - reports progress and failures over serial (or writes to known WRAM addresses),
  - finishes with a single PASS/FAIL marker.
- Prefer table-driven tests so adding cases is data-only.

### C. Deterministic control hooks
- Introduce `#ifdef TEST` hooks for:
  - deterministic RNG seed,
  - fixed joypad input playback,
  - frame-step utilities.
- Keep these hooks isolated behind a `test_support` header so production code paths stay clean.

### D. CI pipeline shape
1. Build main ROM.
2. Build test ROM(s).
3. Run host-side unit tests.
4. Run emulator-based ROM tests headlessly.
5. (Optional) Run snapshot comparisons for key screens.
6. Fail pipeline on first regression; upload logs/artifacts (serial logs, screenshots, diffs).

### E. First concrete test set to implement
- Move validation matrix for each roll (0–4), including reserve/finish edge cases.
- Capture rules on shared path and rosette immunity.
- Extra-turn behavior after rosette landing.
- Link timeout/retry state transitions (module-level, mocked timing).
- RNG range tests to detect bias regressions for bounded values.

This layered approach gives quick feedback from host tests while still validating real DMG runtime behavior through emulator-driven ROM tests.

---

## Suggested execution order

1. **Stabilize interfaces**: introduce `GameState` and reduce `extern` coupling.
2. **Extract complexity**: split `game.c` into focused modules.
3. **Harden randomness + timing**: unify RNG and remove blocking waits.
4. **Reduce duplication**: centralize screen utilities.
5. **Lock quality**: add automated logic tests and CI gate.

This sequence minimizes risk while delivering compounding maintainability gains.
