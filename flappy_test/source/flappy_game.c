// SPDX-License-Identifier: CC0-1.0
//
// Pure gameplay code: no video memory, no console, only state transitions.

#include "flappy_game.h"

static uint32_t next_random(FlappyGame *game)
{
    game->rng_state = game->rng_state * 1664525u + 1013904223u;
    return game->rng_state;
}

static int random_gap_y(FlappyGame *game)
{
    return PIPE_MIN_Y + (int)(next_random(game) % (PIPE_MAX_Y - PIPE_MIN_Y + 1));
}

static void reset_round(FlappyGame *game)
{
    game->bird_y = 86 * FP_ONE;
    game->bird_vy = 0;
    game->score = 0;
    game->frame_count = 0;
    game->ground_scroll = 0;

    for (int i = 0; i < PIPE_COUNT; i++)
    {
        game->pipes[i].x = SCREEN_W + 34 + i * PIPE_SPACING;
        game->pipes[i].gap_y = random_gap_y(game);
        game->pipes[i].scored = false;
    }
}

void flappy_game_init(FlappyGame *game)
{
    game->state = GAME_TITLE;
    game->best_score = 0;
    game->rng_state = 0x4d595df4u;
    reset_round(game);
}

static void recycle_pipe(FlappyGame *game, int pipe_index)
{
    int farthest = game->pipes[0].x;

    for (int i = 1; i < PIPE_COUNT; i++)
    {
        if (game->pipes[i].x > farthest)
            farthest = game->pipes[i].x;
    }

    int next_x = farthest + PIPE_SPACING;
    int minimum_x = SCREEN_W + PIPE_RESPAWN_MARGIN;

    if (next_x < minimum_x)
        next_x = minimum_x;

    game->pipes[pipe_index].x = next_x;
    game->pipes[pipe_index].gap_y = random_gap_y(game);
    game->pipes[pipe_index].scored = false;
}

static void flap(FlappyGame *game)
{
    game->bird_vy = FLAP_VELOCITY;
}

static bool bird_hits_pipe(const FlappyGame *game, const Pipe *pipe)
{
    int bird_top = game->bird_y >> FP_SHIFT;
    int bird_bottom = bird_top + BIRD_H;
    int bird_left = BIRD_X + 2;
    int bird_right = BIRD_X + BIRD_W - 2;

    int pipe_left = pipe->x;
    int pipe_right = pipe->x + PIPE_W;

    if (bird_right < pipe_left || bird_left > pipe_right)
        return false;

    int gap_top = pipe->gap_y - PIPE_GAP / 2;
    int gap_bottom = pipe->gap_y + PIPE_GAP / 2;

    return bird_top < gap_top || bird_bottom > gap_bottom;
}

static void update_playing(FlappyGame *game)
{
    game->frame_count++;
    game->ground_scroll = (game->ground_scroll + PIPE_SPEED) & 15;

    game->bird_vy += GRAVITY;
    if (game->bird_vy > 960)
        game->bird_vy = 960;
    game->bird_y += game->bird_vy;

    for (int i = 0; i < PIPE_COUNT; i++)
    {
        game->pipes[i].x -= PIPE_SPEED;

        if (game->pipes[i].x + PIPE_W < 0)
            recycle_pipe(game, i);

        if (!game->pipes[i].scored && game->pipes[i].x + PIPE_W < BIRD_X)
        {
            game->pipes[i].scored = true;
            game->score++;
            if (game->score > game->best_score)
                game->best_score = game->score;
        }

        if (bird_hits_pipe(game, &game->pipes[i]))
            game->state = GAME_OVER;
    }

    int bird_top = game->bird_y >> FP_SHIFT;
    if (bird_top < 0 || bird_top + BIRD_H >= GROUND_Y)
        game->state = GAME_OVER;
}

void flappy_game_handle_input(FlappyGame *game, uint16_t keys_down)
{
    bool flap_pressed = (keys_down & (KEY_TOUCH | KEY_A | KEY_UP)) != 0;

    if (game->state == GAME_TITLE)
    {
        if (flap_pressed)
        {
            reset_round(game);
            flap(game);
            game->state = GAME_PLAYING;
        }
    }
    else if (game->state == GAME_PLAYING)
    {
        if (flap_pressed)
            flap(game);

        if (keys_down & KEY_START)
            game->state = GAME_PAUSED;
    }
    else if (game->state == GAME_PAUSED)
    {
        if (keys_down & KEY_START)
            game->state = GAME_PLAYING;
    }
    else if (game->state == GAME_OVER)
    {
        if (flap_pressed)
        {
            reset_round(game);
            flap(game);
            game->state = GAME_PLAYING;
        }

        if (keys_down & KEY_SELECT)
        {
            reset_round(game);
            game->state = GAME_TITLE;
        }
    }
}

void flappy_game_tick(FlappyGame *game)
{
    if (game->state == GAME_PLAYING)
        update_playing(game);
    else
        game->frame_count++;
}
