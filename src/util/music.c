#include <gb/gb.h>
#include <stdint.h>
#include "util/music.h"

enum {
    ROW_FRAMES = 9,
    STEP_COUNT = 9,
    RESP_COUNT = 3,
    SEQ_COUNT = 5
};

typedef enum {
    N_REST,
    N_F3,
    N_G3,
    N_A3,
    N_B3,
    N_C4,
    N_D4,
    N_E4,
    N_F4,
    N_G4,
    N_A4,
    N_B4
} NoteCode_t;

// Step order maps to rows: 00,02,04,06,08,0A,0C,0E,0F
static const uint8_t lead_patterns[4][STEP_COUNT] = {
    { N_E4, N_F4, N_G4, N_A4, N_G4, N_F4, N_E4, N_D4, N_E4 }, // Original
    { N_G4, N_A4, N_B4, N_A4, N_G4, N_F4, N_E4, N_F4, N_E4 }, // A
    { N_E4, N_D4, N_C4, N_D4, N_E4, N_G4, N_F4, N_E4, N_REST }, // B
    { N_F4, N_G4, N_F4, N_E4, N_D4, N_F4, N_G4, N_F4, N_E4 }  // C
};

// Rows: 04,08,0C
static const uint8_t response_patterns[4][RESP_COUNT] = {
    { N_B3, N_A3, N_G3 },
    { N_D4, N_C4, N_B3 },
    { N_A3, N_G3, N_F3 },
    { N_C4, N_B3, N_A3 }
};

// Loop variation: Original -> A -> Original -> B -> C
static const uint8_t pattern_sequence[SEQ_COUNT] = { 0, 1, 0, 2, 3 };

static uint8_t music_row;
static uint8_t music_tick;
static uint8_t sequence_index;
static uint8_t active_pattern;
static uint8_t music_enabled;

static uint16_t note_period(uint8_t code) {
    switch (code) {
        case N_F3: return 0x0511;
        case N_G3: return 0x0563;
        case N_A3: return 0x05AC;
        case N_B3: return 0x05ED;
        case N_C4: return 0x060B;
        case N_D4: return 0x0642;
        case N_E4: return 0x0672;
        case N_F4: return 0x0689;
        case N_G4: return 0x06B2;
        case N_A4: return 0x06D6;
        case N_B4: return 0x06F7;
        default: return 0;
    }
}

static void play_ch1(uint16_t period) {
    if (!period) return;
    NR10_REG = 0x00;
    NR11_REG = 0x20;                          // duty 12.5%
    NR12_REG = 0xF6;                          // pluck: loud + quick decay
    NR13_REG = (uint8_t)period;
    NR14_REG = 0xC0 | (uint8_t)(period >> 8); // trigger + length
}

static void play_ch2(uint16_t period) {
    if (!period) return;
    NR21_REG = 0x40;                          // duty 25%
    NR22_REG = 0x64;                          // quieter, slower decay
    NR23_REG = (uint8_t)period;
    NR24_REG = 0xC0 | (uint8_t)(period >> 8); // trigger + length
}

static void silence_music_channels(void) {
    // CH1 + CH2 off while keeping master sound and SFX channels active
    NR12_REG = 0x00;
    NR22_REG = 0x00;
}

static void play_row(uint8_t row) {
    uint8_t step;

    switch (row) {
        case 0x00: step = 0; break;
        case 0x02: step = 1; break;
        case 0x04: step = 2; break;
        case 0x06: step = 3; break;
        case 0x08: step = 4; break;
        case 0x0A: step = 5; break;
        case 0x0C: step = 6; break;
        case 0x0E: step = 7; break;
        case 0x0F: step = 8; break;
        default: return;
    }

    play_ch1(note_period(lead_patterns[active_pattern][step]));

    if (row == 0x04) {
        play_ch2(note_period(response_patterns[active_pattern][0]));
    } else if (row == 0x08) {
        play_ch2(note_period(response_patterns[active_pattern][1]));
    } else if (row == 0x0C) {
        play_ch2(note_period(response_patterns[active_pattern][2]));
    }
}

void init_music(void) {
    music_row = 0;
    music_tick = 0;
    sequence_index = 0;
    active_pattern = pattern_sequence[sequence_index];
    music_enabled = 1;
    play_row(0);
}

void set_music_enabled(uint8_t enabled) {
    music_enabled = enabled ? 1 : 0;
    if (!music_enabled) {
        silence_music_channels();
    } else {
        play_row(music_row);
    }
}

uint8_t is_music_enabled(void) {
    return music_enabled;
}

void update_music(void) {
    if (!music_enabled) {
        return;
    }
    music_tick++;
    if (music_tick < ROW_FRAMES) {
        return;
    }

    music_tick = 0;
    music_row = (music_row + 1) & 0x0F;

    if (music_row == 0x00) {
        sequence_index++;
        if (sequence_index >= SEQ_COUNT) {
            sequence_index = 0;
        }
        active_pattern = pattern_sequence[sequence_index];
    }

    play_row(music_row);
}
