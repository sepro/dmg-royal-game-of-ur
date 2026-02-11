# SOUND Integration Findings (DMG / GBDK-2020)

## Short answer
Yes — this project can integrate the sound effects you listed with low risk.

Given your requirement to reserve **pulse channels CH1 + CH2** for future background melody, you can implement SFX entirely on:
- **CH3 (wave channel):** tonal UI and move sounds
- **CH4 (noise channel):** dice / impact / win-lose accents

This maps well to your event list and keeps melodic channels free.

---

## What exists today (and what does not)

Current codebase status:
- No sound initialization or playback calls exist yet (no `NRxx` register usage and no sound utility module).
- Input and game flow are already centralized enough to add SFX hooks cleanly.

Good hook points already exist in screen update logic:
- Title menu navigation + confirm in `update_title()`. 
- Opponent/profile/difficulty navigation + confirm/cancel in their update handlers.
- Game roll / move selection / move execute / win transition in `game.c`.
- Win/lose presentation in `init_endgame()`.

This means integration can be done without architectural changes.

---

## Recommended integration approach

## 1) Add a tiny `audio_sfx` module
Create:
- `include/util/audio_sfx.h`
- `src/util/audio_sfx.c`

Core API suggestion:
- `void audio_init(void);`
- `void audio_update(void);` (called once per frame)
- `void sfx_play_ui_move(void);`
- `void sfx_play_ui_confirm(void);`
- `void sfx_play_ui_back(void);`
- `void sfx_play_move_select(void);`
- `void sfx_play_piece_move(void);`
- `void sfx_play_dice_roll_start(void);`
- `void sfx_play_dice_roll_tick(void);` (optional, throttled)
- `void sfx_play_win(void);`
- `void sfx_play_lose(void);`

Since Makefile already compiles all `src/util/*.c`, adding the module should require no build-system restructuring.

## 2) Initialize audio once in `main()`
Call `audio_init()` during boot (after display setup is fine), and `audio_update()` once per frame in the main loop.

## 3) Trigger SFX at event points
Use one-shot function calls exactly where input/state changes happen.

---

## DMG channel allocation plan

- **CH3 Wave** = tonal/"pitched" sounds
  - Menu cursor tick
  - Confirm/select chirp
  - Piece-move tone
  - Win/lose motifs (short)

- **CH4 Noise** = non-tonal/transient sounds
  - Dice rolling texture
  - Clicks/thuds
  - Optional low burst on capture / strong move

- **CH1 + CH2 Pulse** = reserved for future BGM
  - Do not use these channels in SFX implementation.

---

## Practical sound design workflow (how to create the sounds)

## A) Start with a tiny SFX preset table (recommended)
Represent each SFX as a small preset (or mini-sequence) mapped to CH3/CH4 register values.

Benefits:
- Fast iteration on real hardware/emulator.
- No external conversion pipeline needed for first pass.
- Easy to keep memory footprint small.

Example concept (not exact code):
- CH3 preset fields: output level, frequency, length/frames, wave-id.
- CH4 preset fields: envelope, polynomial (tone/noise flavor), length/frames.

## B) Author CH3 waveforms directly (32 samples)
CH3 wave RAM is 32 4-bit samples.

Create 3–4 reusable waveforms:
1. **Soft sine-ish** (menu/UI)
2. **Triangle-ish** (neutral utility)
3. **Square-ish grit** (action/move)
4. **Bright stepped wave** (win stinger)

Workflow:
1. Define 32 nybbles (0..15) waveform.
2. Pack two samples per byte into 16 bytes.
3. Load into wave RAM before triggering CH3 sounds.
4. Reuse same waveform set with different frequencies/envelopes.

## C) Build CH4 noise presets by role
Use 2–4 reusable noise "colors":
- **Short click** (UI back/cancel)
- **Light rattle** (dice tick)
- **Dense burst** (dice start)
- **Broad hit/noise tail** (lose accent)

For dice rolling, prefer:
- One start burst + occasional ticks during roll animation, not every frame.
- Trigger tick every N frames (e.g. every 3–5 frames) to avoid harsh spam.

## D) Compose win/lose as tiny 2-layer stingers
- **Win:** CH3 ascending 2–3 notes + subtle CH4 sparkle noise.
- **Lose:** CH3 descending motif + short low noise burst.

Keep under ~500ms each for snappy UX.

---

## Suggested event-to-sound mapping

## UI navigation
- **Title selection move** (up/down): CH3 short soft tick.
- **Profile selection move** (grid moves): CH3 tick, slightly lower pitch than title if desired.
- **Difficulty move**: same family tick for consistency.
- **Confirm (A)**: CH3 short rising chirp.
- **Back (B)**: CH3 or CH4 short downward/click sound.

## Gameplay
- **Selecting a move** (cycling valid pieces): CH3 narrow tick.
- **Piece moved (commit)**: CH3 mid "blip" + tiny CH4 tap.
- **Dice rolling start**: CH4 burst.
- **Dice rolling ongoing**: CH4 rattle ticks throttled.
- **Roll result reveal**: optional CH3 single note keyed by roll total.

## End state
- **Win:** CH3 ascending motif (+ optional CH4 sparkle).
- **Lose:** CH3 descending motif (+ optional CH4 low noise).

---

## Concrete hook points in current source

These are the best insertion points for one-shot SFX calls:

- `src/screens/title.c`
  - When `selected_option` changes in `update_title()` → navigation SFX.
  - On `input_pressed(J_A)` branch → confirm SFX.

- `src/screens/opponent_select.c`
  - When `new_selection != selected_opponent` → navigation SFX.
  - On A confirm / B back branches → confirm/back SFX.

- `src/link/link_profile.c`
  - When grid selection changes in `update_selecting()` → navigation SFX.
  - On A confirm and B cancel branches → confirm/back SFX.

- `src/screens/difficulty_select.c`
  - On up/down selection moves → navigation SFX.
  - On A confirm / B back → confirm/back SFX.

- `src/screens/game.c`
  - `start_roll_animation()` → dice start SFX.
  - `update_roll_animation()` while rolling (throttled) → dice tick SFX.
  - `update_move_selection()` when left/right changes `selection_index` → move-select SFX.
  - `update_move_selection()` and CPU/link move execution path where `execute_move(...)` is called → piece-move SFX.
  - On win detection path (`check_win_condition()` true) before transition to endgame → queue win/lose based on `human_won`.

- `src/screens/endgame.c`
  - `init_endgame()` as fallback location for win/lose jingle if not already triggered.

---

## Sound register notes (GBDK / DMG)

In `audio_init()`:
- Enable APU master power and routing/volume (`NR52`, `NR50`, `NR51`).
- Configure CH3 defaults (`NR30..NR34`) and CH4 defaults (`NR41..NR44`) only when needed.

Important CH3 behavior:
- CH3 must be disabled before writing wave RAM, then re-enabled.
- Use output level control (`NR32`) to keep UI sounds subtle.

Important CH4 behavior:
- Envelope (`NR42`) and polynomial (`NR43`) shape the character dramatically.
- Keep durations short to reduce masking of UI readability.

---

## Mixing and priority recommendations

To keep sound clean and BGM-ready:
- Give SFX priority classes:
  1. critical (win/lose)
  2. gameplay (piece move, dice start)
  3. UI (cursor ticks)
- Drop or delay low-priority SFX if channel busy.
- Add per-SFX cooldown (e.g. 2–4 frames) to prevent chatter.

When BGM is added later:
- Continue reserving CH1/CH2 for melody/harmony.
- Optionally duck CH3 SFX level slightly while melody plays on CH1/CH2.

---

## Suggested initial SFX options (pick one style set)

## Option Set A: "Classic UI" (minimal)
- Menu move: soft CH3 sine tick.
- Confirm: short 2-note up chirp.
- Back: short down chirp.
- Piece move: single mid blip.
- Dice: CH4 light rattle.
- Win: 3-note rise.
- Lose: 2-note fall.

Pros: clean, unobtrusive, quick to tune.

## Option Set B: "Chunky board game"
- Menu move: slightly clicky CH3 triangle tick.
- Confirm: brighter CH3 + tiny CH4 click.
- Piece move: CH3 thunk + CH4 tap.
- Dice: denser CH4 burst and rattle.
- Win/Lose: stronger contrast with noise accents.

Pros: more tactile personality.

## Option Set C: "Ancient mystic"
- Menu/UI: hollow CH3 waveform with lower frequencies.
- Piece move: resonant low-mid tone.
- Dice: dry sand-like CH4.
- Win: rising pentatonic-ish motif.
- Lose: descending minor-ish interval.

Pros: thematic identity; slightly more design effort.


---

## Testing/prototyping SFX with hUGETracker

Yes — you can absolutely use **hUGETracker** to prototype these sounds before hard-coding register presets.

Use it as a **sound sketchpad**, then copy the final CH3/CH4 characteristics into your `audio_sfx` presets.

### Important caveat
hUGETracker is primarily a music tracker, so one-shot SFX workflow is less direct than custom register code. For this project, that is okay:
- Keep SFX prototyping in hUGETracker.
- Implement final runtime playback with direct CH3/CH4 register writes.
- Continue reserving CH1/CH2 in the game code.

### Recommended hUGETracker setup for your constraints
1. Create a new `.uge` project.
2. Reserve **CH1 + CH2** by leaving them empty (or muted) for all test patterns.
3. Build SFX on:
   - **CH3** for tonal UI/move sounds.
   - **CH4** for dice/noise/impact sounds.
4. Use a short test pattern (e.g., 16 or 32 rows) with each SFX isolated and labeled by row region.

### How to create each requested sound in hUGETracker

### Concrete hUGETracker starter settings (specific values)

Use these as a first-pass baseline you can type in immediately:

- **Ticks/row speed:** start around 6.
- **Song tempo:** start around 150 BPM (or close default, then tune by ear).
- **CH1/CH2:** leave unused/muted in all patterns.
- **CH3 waveform set:** create 3 custom waves and reuse them.

Suggested CH3 waveforms (32 samples, values 0..15):

1. **Wave A (soft UI, sine-ish)**
   - `8,9,10,11,12,13,14,14,15,14,14,13,12,11,10,9,8,7,6,5,4,3,2,2,1,2,2,3,4,5,6,7`
2. **Wave B (triangle-ish, neutral/action)**
   - `0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0`
3. **Wave C (edgy/chunky)**
   - `2,2,2,2,13,13,13,13,2,2,2,2,13,13,13,13,3,3,3,3,12,12,12,12,3,3,3,3,12,12,12,12`

Suggested mapping:
- Menu navigation = Wave A
- Piece move / confirm = Wave B
- Strong accent (optional) = Wave C

Suggested CH3 note ranges:
- UI tick: `C-5` to `G-5`
- Confirm chirp: `D-5 -> G-5` or `E-5 -> A-5`
- Piece move: `A-4` to `D-5`
- Win motif: `C-5 -> E-5 -> G-5`
- Lose motif: `G-4 -> E-4 -> C-4`

Suggested CH4/noise character guide:
- **Dice tick:** short, bright noise (very short instrument/length)
- **Dice start burst:** denser noise with slightly longer decay
- **Lose accent:** lower/noisier burst with medium-short decay

---

### Manual starter patterns you can type (row-by-row)

Below are simple pattern sketches. Keep CH1/CH2 blank. Enter only CH3/CH4.

Conventions in examples:
- `..` means empty row on that channel.
- CH3 note names are literal tracker notes.
- CH4 uses generic noise note symbols (type the closest high/low noise note your theme uses; exact pitch on CH4 is less musical than texture).

#### Pattern A: UI navigation ticks (title/profile/difficulty)
Goal: very short, non-fatiguing cursor move sound.

- CH3 wave: **Wave A**
- CH3 instrument/volume: low-medium

Rows (16-row pattern):
- `00: CH3 C-5`
- `01: CH3 ..`
- `02: CH3 ..`
- `03: CH3 ..`
- `04: CH3 D-5`
- `05: CH3 ..`
- `06: CH3 ..`
- `07: CH3 ..`
- `08: CH3 E-5`
- `09-15: CH3 ..`

Use one of those rows as your menu "move" test trigger.

#### Pattern B: Confirm/select sound
Goal: short rising chirp.

- CH3 wave: **Wave B**

Rows:
- `00: CH3 D-5`
- `01: CH3 ..`
- `02: CH3 G-5`
- `03-15: CH3 ..`

Optional layer:
- Add CH4 very short click on row `00`.

#### Pattern C: Piece moved
Goal: tactile board-piece "blip/thunk".

- CH3 wave: **Wave B**
- Optional CH4 tap

Rows:
- `00: CH3 B-4`
- `01: CH3 ..`
- `02: CH3 F-4`
- `03: CH4 (short click/high noise)`
- `04-15: ..`

If too heavy, remove row `02` and keep single note.

#### Pattern D: Dice rolling
Goal: burst + intermittent rattle (not constant spam).

- CH4 only (or very light CH3 support)

Rows:
- `00: CH4 (dense burst)`
- `01: CH4 ..`
- `02: CH4 (short tick)`
- `03: CH4 ..`
- `04: CH4 ..`
- `05: CH4 (short tick)`
- `06: CH4 ..`
- `07: CH4 ..`
- `08: CH4 (short tick)`
- `09-15: CH4 ..`

In runtime code, mirror this idea: one start SFX + throttled tick calls every few frames.

#### Pattern E: Win stinger
Goal: quick positive cadence.

- CH3 wave: **Wave A** or **Wave B**
- Optional CH4 sparkle on final note

Rows:
- `00: CH3 C-5`
- `01: CH3 ..`
- `02: CH3 E-5`
- `03: CH3 ..`
- `04: CH3 G-5`
- `05: CH4 (tiny sparkle, optional)`
- `06-15: ..`

#### Pattern F: Lose stinger
Goal: clear downward cadence.

- CH3 wave: **Wave B**
- Optional low CH4 burst at start/end

Rows:
- `00: CH3 G-4`
- `01: CH3 ..`
- `02: CH3 E-4`
- `03: CH3 ..`
- `04: CH3 C-4`
- `05: CH4 (low short burst, optional)`
- `06-15: ..`

---

### Practical "type-this-first" workflow

1. Create 6 patterns (A..F) using the rows above.
2. Put each pattern on its own order position so you can audition quickly.
3. Tweak only one variable at a time:
   - first note pitch
   - note spacing
   - waveform (A/B/C)
   - CH4 decay length
4. Once each effect feels right, copy the final choices into your `audio_sfx` preset table (note interval, duration, wave, noise character).

### Export/transfer workflow from hUGETracker to game code
1. Prototype until the sound feels right in emulator playback.
2. For each SFX, write down:
   - channel used (CH3/CH4)
   - relative pitch steps / note length
   - loudness and decay feel
   - any layer timing offsets
3. Convert each into a compact preset/mini-sequence in `audio_sfx.c`.
4. Test on BGB/SameBoy and tune by ear against real gameplay cadence.

### Fast iteration template (recommended)
Make one hUGETracker test song with sections:
- Rows 00–0F: UI move variations
- Rows 10–1F: confirm/back
- Rows 20–2F: piece move
- Rows 30–4F: dice roll variants
- Rows 50–5F: win
- Rows 60–6F: lose

This gives you a repeatable "audition board" before implementing final register values.

---

## Risks and mitigations

- **Risk:** Too many rapid triggers from input repeat patterns.
  - **Mitigation:** Trigger only on edge-detected changes (already done by `input_pressed` + selection comparisons).

- **Risk:** Dice animation SFX overwhelms mix.
  - **Mitigation:** Throttle CH4 tick rate and keep envelope short.

- **Risk:** Future BGM conflicts.
  - **Mitigation:** Enforce CH3/CH4-only SFX policy now and codify in `audio_sfx.h` comments.

---

## Implementation order (fast path)

1. Add `audio_sfx` module with init + 4 core SFX (`ui_move`, `confirm`, `dice`, `piece_move`).
2. Hook title/opponent/profile/difficulty selection events.
3. Hook game roll + move-selection + execute move.
4. Add win/lose stingers.
5. Tune levels on hardware-accurate emulator (BGB/SameBoy), then real device if possible.

This gives immediate audible feedback now while preserving CH1/CH2 for later music.
