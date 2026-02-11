# Codebase Review Findings (Senior Developer Pass)

This document highlights the highest-impact maintainability and correctness risks in the current codebase, why they matter, and pragmatic refactoring directions.

## 1) `game.c` is doing too much (high complexity hotspot)  -- Won't FIX (overkill for this project)

---

## 2) Heavy coupling via cross-module globals (`extern` state sharing) -- Won't FIX (overkill for this project)

---

## 3) Randomness usage is inconsistent and potentially biased -- FIXED

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

## 5) Repeated UI helper snippets suggest missing shared utilities --  FIXED

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
