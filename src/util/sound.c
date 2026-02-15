/**
 * sound.c
 * Compact SFX engine to keep ROM footprint low.
 * CH1/CH2 are used by music, SFX stay on CH3/CH4.
 */

#include <gb/gb.h>
#include <stdint.h>
#include "util/sound.h"

static const uint8_t wave_basic[16] = {
    0x02, 0x46, 0x8A, 0xCE, 0xFE, 0xDC, 0xA8, 0x64,
    0x20, 0x02, 0x46, 0x8A, 0xCE, 0xFE, 0xDC, 0xA8
};

static void load_wave(void) {
    uint8_t i;
    NR30_REG = 0x00;
    for (i = 0; i < 16; i++) {
        AUD3WAVE[i] = wave_basic[i];
    }
    NR30_REG = 0x80;
}

static void trigger_wave(uint16_t period, uint8_t volume) {
    NR31_REG = 0xEF;
    NR32_REG = volume;
    NR33_REG = (uint8_t)period;
    NR34_REG = 0xC0 | (uint8_t)(period >> 8);
}

static void trigger_noise(uint8_t envelope, uint8_t polynomial) {
    NR41_REG = 0x08;
    NR42_REG = envelope;
    NR43_REG = polynomial;
    NR44_REG = 0x80;
}

void init_sound(void) {
    NR52_REG = AUDENA_ON;
    NR50_REG = AUDVOL_VOL_LEFT(7) | AUDVOL_VOL_RIGHT(7);
    NR51_REG = AUDTERM_1_LEFT | AUDTERM_1_RIGHT |
               AUDTERM_2_LEFT | AUDTERM_2_RIGHT |
               AUDTERM_3_LEFT | AUDTERM_3_RIGHT |
               AUDTERM_4_LEFT | AUDTERM_4_RIGHT;

    load_wave();
}

void play_sfx(SoundEffect_t sfx) {
    switch (sfx) {
        case SFX_CURSOR:
            trigger_noise(0xE1, 0x31);
            break;

        case SFX_CONFIRM:
            trigger_wave(0x0750, 0x40);
            break;

        case SFX_MOVE_CHANGE:
            trigger_noise(0xC1, 0x49);
            break;

        case SFX_MOVE_SELECT:
            trigger_wave(0x0560, 0x40);
            break;

        case SFX_DICE_ROLL:
            trigger_noise(0xF2, 0x23);
            break;

        case SFX_VICTORY:
            trigger_wave(0x07AC, 0x20);
            break;

        case SFX_LOSS:
            trigger_wave(0x0744, 0x20);
            break;
    }
}

void update_sound(void) {
    // Intentionally empty: one-shot SFX only for smaller code size.
}
