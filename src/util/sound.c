/**
 * sound.c
 * Sound effects using Channel 3 (wave) and Channel 4 (noise)
 * Channels 1 & 2 are reserved for future music.
 */

#include <gb/gb.h>
#include <stdint.h>
#include "util/sound.h"

// Wave patterns (16 bytes each, loaded into AUD3WAVE before ch3 trigger)
static const uint8_t wave_sine[] = {
    0x02, 0x46, 0x8A, 0xCE, 0xFE, 0xDC, 0xA8, 0x64,
    0x20, 0x02, 0x46, 0x8A, 0xCE, 0xFE, 0xDC, 0xA8
};

static const uint8_t wave_triangle[] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
};

// Chime note frequencies (ch3 period register values)
// Victory: C5 -> E5 -> G5 -> C6 (ascending major arpeggio)
static const uint16_t victory_freqs[4] = { 0x0783, 0x079D, 0x07AC, 0x07C1 };
static const uint8_t victory_durations[4] = { 12, 12, 12, 20 };

// Loss: E5 -> C5 -> A4 -> F4 (descending)
static const uint16_t loss_freqs[4] = { 0x079D, 0x0783, 0x076B, 0x0744 };
static const uint8_t loss_durations[4] = { 15, 15, 15, 25 };

// Multi-note chime sequencer state
static uint8_t chime_active;
static uint8_t chime_note_index;
static uint8_t chime_frame_counter;
static const uint16_t *chime_freqs;
static const uint8_t *chime_durations;

/**
 * Load 16 bytes into wave RAM
 * Must disable ch3 DAC before writing
 */
static void load_wave(const uint8_t *data) {
    NR30_REG = 0x00;  // Disable DAC to allow wave RAM writes
    for (uint8_t i = 0; i < 16; i++) {
        AUD3WAVE[i] = data[i];
    }
    NR30_REG = 0x80;  // Re-enable DAC
}

/**
 * Trigger wave channel at given period value
 */
static void trigger_wave_note(uint16_t period) {
    NR31_REG = 0xEF;                              // Short length
    NR32_REG = 0x20;                              // 100% volume (bits 5-6 = 01)
    NR33_REG = (uint8_t)(period & 0xFF);          // Frequency low
    NR34_REG = 0xC0 | ((period >> 8) & 0x07);     // Trigger + length enable + freq high
}

void init_sound(void) {
    // Enable master sound
    NR52_REG = AUDENA_ON;

    // Max volume both speakers
    NR50_REG = AUDVOL_VOL_LEFT(7) | AUDVOL_VOL_RIGHT(7);

    // Route ch3 and ch4 to both speakers (leave ch1+ch2 unrouted)
    NR51_REG = AUDTERM_3_LEFT | AUDTERM_3_RIGHT |
               AUDTERM_4_LEFT | AUDTERM_4_RIGHT;

    chime_active = 0;
}

void play_sfx(SoundEffect_t sfx) {
    switch (sfx) {
        case SFX_CURSOR:
            // Short sharp click on noise channel
            NR42_REG = 0xF1;  // Vol 15, envelope down 1 step
            NR43_REG = 0x41;  // Clock shift 4, divisor 1
            NR44_REG = 0x80;  // Trigger
            break;

        case SFX_CONFIRM:
            chime_active = 0;
            load_wave(wave_sine);
            trigger_wave_note(0x0500);
            break;

        case SFX_MOVE_CHANGE:
            // Softer tick on noise channel
            NR42_REG = 0xC1;  // Vol 12, envelope down 1 step
            NR43_REG = 0x51;  // Clock shift 5, divisor 1
            NR44_REG = 0x80;  // Trigger
            break;

        case SFX_MOVE_SELECT:
            chime_active = 0;
            load_wave(wave_sine);
            trigger_wave_note(0x0400);
            break;

        case SFX_DICE_ROLL:
            // Rattle texture on noise channel
            NR42_REG = 0xF3;  // Vol 15, envelope down 3 steps
            NR43_REG = 0x33;  // Clock shift 3, divisor 3
            NR44_REG = 0x80;  // Trigger
            break;

        case SFX_VICTORY:
            // Ascending chime (sine wave)
            load_wave(wave_sine);
            chime_freqs = victory_freqs;
            chime_durations = victory_durations;
            chime_note_index = 0;
            chime_frame_counter = 0;
            chime_active = 1;
            trigger_wave_note(chime_freqs[0]);
            break;

        case SFX_LOSS:
            // Descending chime (triangle wave for darker tone)
            load_wave(wave_triangle);
            chime_freqs = loss_freqs;
            chime_durations = loss_durations;
            chime_note_index = 0;
            chime_frame_counter = 0;
            chime_active = 1;
            trigger_wave_note(chime_freqs[0]);
            break;
    }
}

void update_sound(void) {
    if (!chime_active) return;

    chime_frame_counter++;

    if (chime_frame_counter >= chime_durations[chime_note_index]) {
        chime_frame_counter = 0;
        chime_note_index++;

        if (chime_note_index >= 4) {
            // Chime complete - silence wave channel
            NR30_REG = 0x00;
            chime_active = 0;
            return;
        }

        // Trigger next note
        trigger_wave_note(chime_freqs[chime_note_index]);
    }
}
