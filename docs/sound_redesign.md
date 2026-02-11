# Sound redesign proposal (character-focused SFX)

This project currently synthesizes all effects directly on DMG hardware (CH3 wave + CH4 noise), which is the right technical direction for low ROM and no-sample playback.

To add more character, use online references and rebuild them as small parametric patches:

## Online references worth mining

- **jsfxr / sfxr patch galleries** for punchy retro UI and game action SFX patterns:
  - https://sfxr.me/
  - https://www.bfxr.net/
- **Freesound** for foley references (dice cup shake, ceramic clicks, wooden impacts):
  - https://freesound.org/
- **Kenney UI / game audio packs** for reusable style references and layering ideas:
  - https://kenney.nl/assets

## Engineering approach for DMG implementation

1. Design target sounds in jsfxr/bfxr first (fast iteration).
2. Extract envelope + pitch contour from references.
3. Re-synthesize on DMG channels:
   - **CH4** for transient/noise body (cursor, dice, tactile ticks)
   - **CH3** for tonal identity (confirm, select, victory/loss motifs)
4. Add lightweight modulation:
   - Multi-frame sequencing for noise color evolution
   - 2-note motifs for semantic UI actions
   - Subtle vibrato for emotional chimes
5. Keep effects under ~250ms for UI responsiveness.

## Immediate next tuning pass

- Cursor: add very short pre-click + brighter tail.
- Confirm: tune motif intervals for stronger "resolution" feel.
- Dice roll: shape decay curve so it starts chaotic then tightens.
- Victory/Loss: add tiny rests between notes to increase phrasing.

