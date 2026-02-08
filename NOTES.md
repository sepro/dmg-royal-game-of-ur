# Notes and todo-items for later

  * add Pil or Pillow to the dockerfile and rebuild

  * Update the image for the titlescreen
  * Define places where the sparkle sprite can appear
  * Update names and description for players
  * Revise opponent profiles
  * Design and add sgb border https://gbdev.io/guides/sgb_border.html
  

## SGB border
Below is a minimal C example showing how you might integrate a Super Game Boy custom border into a gbdev-style Game Boy project. It uses the SGB transfer packet API from GBDK (if you’re using plain C with gbdev tooling you can adapt the packet sending and detection parts manually).

This example assumes you’ve already generated border data (tiles, map, palettes) into C arrays (e.g., via png2asset with pack_mode sgb) and want to send it to the SGB at startup.

For a Super Game Boy (SGB) custom border, the input PNG should be 256 pixels wide and 224 pixels tall (that’s the standard SGB border canvas size) so it matches the tilemap size the SGB expects when converting it into tiles/maps/palettes.

1) Data headers (generated via png2asset)
// border_data.h
extern const unsigned char border_tiles[];
extern const unsigned int border_tiles_len;
extern const unsigned char border_map_pal[];
extern const unsigned int border_map_pal_len;


Generate these with png2asset input.png -map -bpp 4 -max_palettes 4 -pack_mode sgb -c border_data.c as documented in GBDK docs.

2) SGB init + border transfer (C code)
#include <gb/gb.h>
#include <gb/sgb.h>
#include "border_data.h"

void send_sgb_packets(const uint8_t *data, uint16_t size) {
    const uint8_t *p = data;
    uint16_t remaining = size;

    while (remaining) {
        sgb_transfer((uint8_t*)p);
        // delay a few VBLs (roughly 4 frames recommended)
        for (uint8_t i=0; i<4; i++) wait_vbl_done();
        p += 16;
        remaining = (remaining > 16) ? (remaining - 16) : 0;
    }
}

void main() {
    // Wait a bit so SGB BIOS is ready to accept packets
    for (uint8_t i = 0; i < 8; i++) wait_vbl_done();

    // Check if we are running under Super Game Boy
    if (sgb_check()) {
        // Send tile data (CHR_TRN)
        send_sgb_packets(border_tiles, border_tiles_len);

        // Send map + palette data (PCT_TRN)
        send_sgb_packets(border_map_pal, border_map_pal_len);
    }

    // Now run your game loop
    while(1) {
        // normal game code here
        wait_vbl_done();
    }
}


This uses:

sgb_check() to detect SGB presence (returns non-zero on SGB)

sgb_transfer() to send 16-byte SGB packets sequentially over the joypad interface

delays between packets for reliable reception.

3) Build flags and header requirements

To ensure the SGB BIOS engages correctly:

Set SGB flag and old licensee code in the header (so your ROM is recognized as SGB-aware). With RGBFIX, that’s --sgb-compatible --old-licensee 0x33.

The border assets must follow the SGB specs: 255 tiles + 1 transparent tile, 3 palettes, and a 256×224 tilemap.

4) Notes for gbdev (manual packet)

If you’re not using GBDK’s sgb_transfer, you must implement sending SGB packets manually by bit-banging JOYP (P1) register per the SGB packet protocol: start pulse, data pulses, inter-packet delays, etc. The gbdev guide covers this in detail.