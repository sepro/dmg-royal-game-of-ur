#include <gb/gb.h>
#include <stdint.h>
#include "util/music.h"

enum {
    NOTE_REST = -1,
    ROW_COUNT = 16,
    ROW_FRAMES = 10,
    CH1_NOTE_FRAMES = 4,
    CH2_NOTE_FRAMES = 6
};

// E Phrygian lead on CH1 (row-based tracker pattern)
static const int16_t ch1_pattern[ROW_COUNT] = {
    0x0672, NOTE_REST, 0x0689, NOTE_REST,
    0x06B2, NOTE_REST, 0x06D6, NOTE_REST,
    0x06B2, NOTE_REST, 0x0689, NOTE_REST,
    0x0672, NOTE_REST, 0x0642, 0x0672
};

// Sparse response motif on CH2
static const int16_t ch2_pattern[ROW_COUNT] = {
    NOTE_REST, NOTE_REST, NOTE_REST, NOTE_REST,
    0x05ED, NOTE_REST, NOTE_REST, NOTE_REST,
    0x05AC, NOTE_REST, NOTE_REST, NOTE_REST,
    0x0563, NOTE_REST, NOTE_REST, NOTE_REST
};

static uint8_t row_index;
static uint8_t frame_in_row;
static uint8_t ch1_frames_left;
static uint8_t ch2_frames_left;

static void trigger_ch1(uint16_t period) {
    NR10_REG = 0x00;                           // No sweep
    NR11_REG = 0x00 | 0x20;                    // 12.5% duty, short length
    NR12_REG = 0xF6;                           // High start volume, fast decay
    NR13_REG = (uint8_t)(period & 0xFF);
    NR14_REG = 0xC0 | ((period >> 8) & 0x07);  // Trigger + length enable
    ch1_frames_left = CH1_NOTE_FRAMES;
}

static void trigger_ch2(uint16_t period) {
    NR21_REG = 0x40 | 0x28;                    // 25% duty, short/medium length
    NR22_REG = 0x74;                           // Quieter, gentle decay
    NR23_REG = (uint8_t)(period & 0xFF);
    NR24_REG = 0xC0 | ((period >> 8) & 0x07);  // Trigger + length enable
    ch2_frames_left = CH2_NOTE_FRAMES;
}

static void trigger_row(uint8_t row) {
    int16_t ch1_note = ch1_pattern[row];
    int16_t ch2_note = ch2_pattern[row];

    if (ch1_note != NOTE_REST) {
        trigger_ch1((uint16_t)ch1_note);
    }

    if (ch2_note != NOTE_REST) {
        trigger_ch2((uint16_t)ch2_note);
    }
}

void init_music(void) {
    row_index = 0;
    frame_in_row = 0;
    ch1_frames_left = 0;
    ch2_frames_left = 0;

    trigger_row(row_index);
}

void update_music(void) {
    if (ch1_frames_left) {
        ch1_frames_left--;
        if (!ch1_frames_left) {
            NR12_REG = 0x00;
        }
    }

    if (ch2_frames_left) {
        ch2_frames_left--;
        if (!ch2_frames_left) {
            NR22_REG = 0x00;
        }
    }

    frame_in_row++;
    if (frame_in_row < ROW_FRAMES) {
        return;
    }

    frame_in_row = 0;
    row_index = (uint8_t)((row_index + 1) & 0x0F);
    trigger_row(row_index);
}
