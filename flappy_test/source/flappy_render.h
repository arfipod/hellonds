// SPDX-License-Identifier: CC0-1.0
//
// Framebuffer and bottom-console rendering for Flappy Test.

#ifndef FLAPPY_RENDER_H
#define FLAPPY_RENDER_H

#include <nds.h>

#include "flappy_game.h"

typedef struct
{
    int score;
    GameState state;
} FlappyHudCache;

// Invalidates the cached bottom-screen HUD state.
void flappy_hud_cache_init(FlappyHudCache *cache);

// Draws one top-screen framebuffer frame.
void flappy_render_game(int main_bg, const FlappyGame *game);

// Updates the bottom console only when the visible HUD values changed.
void flappy_render_bottom(PrintConsole *console, const FlappyGame *game,
                          FlappyHudCache *cache);

#endif
