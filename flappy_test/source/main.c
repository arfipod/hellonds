// SPDX-License-Identifier: CC0-1.0
//
// Platform entry point for Flappy Test.

#include <nds.h>

#include "flappy_game.h"
#include "flappy_render.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    PrintConsole bottom_console;
    FlappyGame game;
    FlappyHudCache hud_cache;

    defaultExceptionHandler();

    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
                        VRAM_C_SUB_BG, VRAM_D_LCD);

    int main_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    consoleInit(&bottom_console, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0,
                false, true);

    flappy_game_init(&game);
    flappy_hud_cache_init(&hud_cache);

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        flappy_game_handle_input(&game, keysDown());
        flappy_game_tick(&game);

        flappy_render_game(main_bg, &game);
        flappy_render_bottom(&bottom_console, &game, &hud_cache);
    }

    return 0;
}
