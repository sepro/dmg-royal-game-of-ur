#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

typedef enum {
    SFX_CURSOR,       // Navigation tick (noise ch4)
    SFX_CONFIRM,      // Menu confirm (wave ch3)
    SFX_MOVE_CHANGE,  // Change piece selection (noise ch4)
    SFX_MOVE_SELECT,  // Confirm game move (wave ch3)
    SFX_DICE_ROLL,    // Dice rattle (noise ch4)
    SFX_VICTORY,      // Win chime - ascending arpeggio (wave ch3, multi-frame)
    SFX_LOSS          // Lose chime - descending (wave ch3, multi-frame)
} SoundEffect_t;

/** Enable master sound, route ch3+ch4 to both speakers */
void init_sound(void);

/** Trigger a sound effect (one-shot or start chime sequence) */
void play_sfx(SoundEffect_t sfx);

/** Advance chime sequencer - call every frame from main loop */
void update_sound(void);

#endif
