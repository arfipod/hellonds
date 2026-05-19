// SPDX-License-Identifier: CC0-1.0
//
// Nintendo DS console bootstrap plus a tiny custom font patch for Spanish ñ.

#include "gospel_console.h"

#include <stdio.h>
#include <string.h>

#define ENYE_CHAR 0x7f
#define CONSOLE_FONT_BYTES (96 * 8)

extern const uint8_t default_fontTiles[];

static uint8_t console_font_tiles[CONSOLE_FONT_BYTES];

// The libnds default console font contains ASCII 32..127 only. The generated
// Gospel text maps ñ/Ñ to character 0x7f, so this tile replaces that slot.
static const uint8_t enye_glyph[8] = {
    0x0a,
    0x14,
    0x00,
    0x16,
    0x19,
    0x11,
    0x11,
    0x00,
};

static ConsoleFont console_font = {
    .gfx = console_font_tiles,
    .pal = NULL,
    .numColors = 0,
    .bpp = 1,
    .asciiOffset = 32,
    .numChars = 96,
};

void gospel_console_init(PrintConsole *top, PrintConsole *bottom)
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    consoleInit(top, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    memcpy(console_font_tiles, default_fontTiles, CONSOLE_FONT_BYTES);
    memcpy(&console_font_tiles[(ENYE_CHAR - 32) * 8], enye_glyph, sizeof(enye_glyph));
    consoleSetFont(top, &console_font);
    consoleSetFont(bottom, &console_font);
}

void gospel_console_clear(PrintConsole *console)
{
    consoleSelect(console);
    printf("\x1b[2J\x1b[H");
}
