#include <gb/gb.h>
#include <stdint.h>
#include "util/music.h"

enum {
    ROW_FRAMES = 12
};

static uint8_t music_row;
static uint8_t music_tick;

static void play_ch1(uint16_t period) {
    NR10_REG = 0x00;
    NR11_REG = 0x20;                          // duty 12.5%
    NR12_REG = 0xF6;                          // pluck: loud + quick decay
    NR13_REG = (uint8_t)period;
    NR14_REG = 0xC0 | (uint8_t)(period >> 8); // trigger + length
}

static void play_ch2(uint16_t period) {
    NR21_REG = 0x40;                          // duty 25%
    NR22_REG = 0x64;                          // quieter, slower decay
    NR23_REG = (uint8_t)period;
    NR24_REG = 0xC0 | (uint8_t)(period >> 8); // trigger + length
}

static void play_row(uint8_t row) {
    switch (row) {
        case 0x00: play_ch1(0x0672); break; // E4
        case 0x02: play_ch1(0x0689); break; // F4
        case 0x04: play_ch1(0x06B2); play_ch2(0x05ED); break; // G4 + B3
        case 0x06: play_ch1(0x06D6); break; // A4
        case 0x08: play_ch1(0x06B2); play_ch2(0x05AC); break; // G4 + A3
        case 0x0A: play_ch1(0x0689); break; // F4
        case 0x0C: play_ch1(0x0672); play_ch2(0x0563); break; // E4 + G3
        case 0x0E: play_ch1(0x0642); break; // D4
        case 0x0F: play_ch1(0x0672); break; // E4 (quick turn)
        default: break;
    }
}

void init_music(void) {
    music_row = 0;
    music_tick = 0;
    play_row(0);
}

void update_music(void) {
    music_tick++;
    if (music_tick < ROW_FRAMES) {
        return;
    }

    music_tick = 0;
    music_row = (music_row + 1) & 0x0F;
    play_row(music_row);
}
