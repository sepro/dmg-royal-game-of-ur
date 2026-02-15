#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>

/** Initialize and start background music on CH1 + CH2. */
void init_music(void);

/** Enable/disable background music playback. */
void set_music_enabled(uint8_t enabled);

/** Return non-zero if background music playback is enabled. */
uint8_t is_music_enabled(void);

/** Advance music sequencer once per frame. */
void update_music(void);

#endif
