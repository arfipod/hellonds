// SPDX-License-Identifier: CC0-1.0
//
// Game rules and state for Flappy Test.

#ifndef FLAPPY_GAME_H
#define FLAPPY_GAME_H

#include <nds.h>
#include <stdbool.h>
#include <stdint.h>

#define SCREEN_W 256
#define SCREEN_H 192

#define FP_SHIFT 8
#define FP_ONE (1 << FP_SHIFT)

#define BIRD_X 58
#define BIRD_W 16
#define BIRD_H 12

#define PIPE_COUNT 3
#define PIPE_W 34
#define PIPE_SPACING 104
#define PIPE_GAP 64
#define PIPE_MIN_Y 34
#define PIPE_MAX_Y 132
#define PIPE_RESPAWN_MARGIN 18

#define GROUND_Y 168
#define GRAVITY 34
#define FLAP_VELOCITY (-760)
#define PIPE_SPEED 2

typedef enum
{
    GAME_TITLE,
    GAME_PLAYING,
    GAME_PAUSED,
    GAME_OVER
} GameState;

typedef struct
{
    int x;
    int gap_y;
    bool scored;
} Pipe;

typedef struct
{
    GameState state;
    Pipe pipes[PIPE_COUNT];
    int bird_y;
    int bird_vy;
    int score;
    int best_score;
    int frame_count;
    int ground_scroll;
    uint32_t rng_state;
} FlappyGame;

// Initializes persistent game state and prepares the title-screen layout.
void flappy_game_init(FlappyGame *game);

// Handles one frame of button/touch input.
void flappy_game_handle_input(FlappyGame *game, uint16_t keys_down);

// Advances gameplay or idle animation by one frame.
void flappy_game_tick(FlappyGame *game);

#endif
