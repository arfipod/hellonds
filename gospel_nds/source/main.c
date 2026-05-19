// SPDX-License-Identifier: CC0-1.0
//
// Platform entry point for Gospel NDS.

#include <nds.h>
#include <time.h>

#include "gospel_app.h"
#include "gospel_console.h"
#include "gospel_random.h"

int main(void)
{
    PrintConsole top_console;
    PrintConsole bottom_console;
    uint32_t frame_count = 0;

    defaultExceptionHandler();
    gospel_console_init(&top_console, &bottom_console);
    gospel_app_init(&top_console, &bottom_console);

    timerStart(0, ClockDivider_1, 0, NULL);
    timerStart(1, ClockDivider_1024, 0, NULL);
    keysSetRepeat(24, 8);

    gospel_random_mix((uint32_t)time(NULL) ^ ((uint32_t)timerTick(0) << 16) ^ timerTick(1));

    while (1)
    {
        scanKeys();
        uint32_t down = keysDown() | keysDownRepeat();
        uint32_t held = keysHeld();

        frame_count++;
        gospel_random_mix(frame_count ^ (held << 8) ^ down ^
                          ((uint32_t)timerTick(0) << 16) ^ timerTick(1));

        gospel_app_handle_keys(down);
        gospel_app_redraw();
        swiWaitForVBlank();
    }
}
