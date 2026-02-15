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

static const uint8_t wave_grit[] = {
    0x0F, 0x1F, 0x2E, 0x3D, 0x4B, 0x59, 0x67, 0x75,
    0x84, 0x93, 0xA3, 0xB4, 0xC6, 0xD9, 0xEC, 0xFF
};

// Chime note frequencies (ch3 period register values)
// Victory: C5 -> E5 -> G5 -> C6 (ascending major arpeggio)
static const uint16_t victory_freqs[4] = { 0x0783, 0x079D, 0x07AC, 0x07C1 };
static const uint8_t victory_durations[4] = { 12, 12, 12, 20 };

// Loss: E5 -> C5 -> A4 -> F4 (descending)
static const uint16_t loss_freqs[4] = { 0x079D, 0x0783, 0x076B, 0x0744 };
static const uint8_t loss_durations[4] = { 15, 15, 15, 25 };

// Confirm and move-select are short two-note motifs.
static const uint16_t confirm_freqs[2] = { 0x06E0, 0x0750 };
static const uint8_t confirm_durations[2] = { 5, 9 };
static const uint16_t move_select_freqs[2] = { 0x0480, 0x0560 };
static const uint8_t move_select_durations[2] = { 4, 8 };

// Noise texture options for an evolving dice roll.
static const uint8_t dice_poly_values[] = {
    0x13, 0x17, 0x1A, 0x23, 0x27, 0x2B, 0x33, 0x36
};

// Multi-note chime sequencer state
static uint8_t chime_active;
static uint8_t chime_note_index;
static uint8_t chime_frame_counter;
static uint8_t chime_note_count;
static uint8_t vibrato_depth;
static uint8_t vibrato_phase;
static const uint16_t *chime_freqs;
static const uint8_t *chime_durations;

// Dice roll sequencer state.
static uint8_t dice_active;
static uint8_t dice_frame_counter;
static uint8_t dice_frames_remaining;

static void trigger_noise(uint8_t envelope, uint8_t polynomial) {
    NR41_REG = 0x08;      // Short length
    NR42_REG = envelope;  // Initial volume + envelope sweep
    NR43_REG = polynomial;
    NR44_REG = 0x80;      // Trigger
}

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
    NR32_REG = 0x20;                              // 100% wave output
    NR33_REG = (uint8_t)(period & 0xFF);          // Frequency low
    NR34_REG = 0xC0 | ((period >> 8) & 0x07);     // Trigger + length enable + freq high
}

static void start_chime(const uint8_t *wave,
                        const uint16_t *freqs,
                        const uint8_t *durations,
                        uint8_t note_count,
                        uint8_t vibrato) {
    load_wave(wave);
    chime_freqs = freqs;
    chime_durations = durations;
    chime_note_count = note_count;
    chime_note_index = 0;
    chime_frame_counter = 0;
    vibrato_depth = vibrato;
    vibrato_phase = 0;
    chime_active = 1;
    trigger_wave_note(chime_freqs[0]);
}

void init_sound(void) {
    // Enable master sound
    NR52_REG = AUDENA_ON;

    // Max volume both speakers
    NR50_REG = AUDVOL_VOL_LEFT(7) | AUDVOL_VOL_RIGHT(7);

    // Route all channels to both speakers
    NR51_REG = AUDTERM_1_LEFT | AUDTERM_1_RIGHT |
               AUDTERM_2_LEFT | AUDTERM_2_RIGHT |
               AUDTERM_3_LEFT | AUDTERM_3_RIGHT |
               AUDTERM_4_LEFT | AUDTERM_4_RIGHT;

    chime_active = 0;
    dice_active = 0;
}

void play_sfx(SoundEffect_t sfx) {
    switch (sfx) {
        case SFX_CURSOR:
            // Sharper two-layer click for menu movement.
            trigger_noise(0xF1, 0x31);
            trigger_noise(0xA1, 0x51);
            break;

        case SFX_CONFIRM:
            start_chime(wave_sine, confirm_freqs, confirm_durations, 2, 1);
            break;

        case SFX_MOVE_CHANGE:
            // Softer tactile tick.
            trigger_noise(0xD2, 0x49);
            break;

        case SFX_MOVE_SELECT:
            start_chime(wave_grit, move_select_freqs, move_select_durations, 2, 2);
            break;

        case SFX_DICE_ROLL:
            // Kick off an evolving 22-frame rattle with changing color.
            dice_active = 1;
            dice_frame_counter = 0;
            dice_frames_remaining = 22;
            trigger_noise(0xF2, 0x23);
            break;

        case SFX_VICTORY:
            // Ascending celebratory arpeggio with gentle vibrato.
            start_chime(wave_sine, victory_freqs, victory_durations, 4, 2);
            break;

        case SFX_LOSS:
            // Descending darker phrase.
            start_chime(wave_triangle, loss_freqs, loss_durations, 4, 1);
            break;
    }
}

void update_sound(void) {
    if (dice_active) {
        dice_frame_counter++;

        if ((dice_frame_counter & 0x01) == 0) {
            uint8_t index = (DIV_REG ^ dice_frame_counter) & 0x07;
            uint8_t envelope = 0xF1;

            if (dice_frames_remaining < 10) {
                envelope = 0xB2;
            }

            trigger_noise(envelope, dice_poly_values[index]);
            dice_frames_remaining--;
            if (!dice_frames_remaining) {
                dice_active = 0;
            }
        }
    }

    if (!chime_active) {
        return;
    }

    chime_frame_counter++;

    if (vibrato_depth) {
        uint16_t base = chime_freqs[chime_note_index];
        int16_t mod = (vibrato_phase & 0x01) ? (int16_t)vibrato_depth : -(int16_t)vibrato_depth;
        uint16_t modulated = (uint16_t)((int16_t)base + mod);

        NR33_REG = (uint8_t)(modulated & 0xFF);
        NR34_REG = 0x40 | ((modulated >> 8) & 0x07);
        vibrato_phase++;
    }

    if (chime_frame_counter >= chime_durations[chime_note_index]) {
        chime_frame_counter = 0;
        chime_note_index++;

        if (chime_note_index >= chime_note_count) {
            // Chime complete - silence wave channel
            NR30_REG = 0x00;
            chime_active = 0;
            return;
        }

        // Trigger next note
        trigger_wave_note(chime_freqs[chime_note_index]);
    }
}
