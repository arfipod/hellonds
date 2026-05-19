// SPDX-License-Identifier: CC0-1.0
//
// All drawing code for Flappy Test lives here. Gameplay never touches pixels.

#include "flappy_render.h"

#include <stdio.h>
#include <string.h>

#define COLOR(r, g, b) ((uint16_t)(BIT(15) | RGB15((r), (g), (b))))

static const uint8_t font_digits[10][7] = {
    {0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e},
    {0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e},
    {0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f},
    {0x1e, 0x01, 0x01, 0x0e, 0x01, 0x01, 0x1e},
    {0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02},
    {0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e},
    {0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e},
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08},
    {0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e},
    {0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c},
};

static const uint8_t font_letters[26][7] = {
    {0x0e, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
    {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e},
    {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e},
    {0x1e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1e},
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f},
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10},
    {0x0f, 0x10, 0x10, 0x13, 0x11, 0x11, 0x0f},
    {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11},
    {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e},
    {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0e},
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11},
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f},
    {0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11},
    {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11},
    {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
    {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10},
    {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d},
    {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11},
    {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e},
    {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e},
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04},
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11},
    {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11},
    {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04},
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f},
};

void flappy_hud_cache_init(FlappyHudCache *cache)
{
    cache->score = -1;
    cache->state = (GameState)-1;
}

static void put_pixel(uint16_t *fb, int x, int y, uint16_t color)
{
    if ((unsigned)x < SCREEN_W && (unsigned)y < SCREEN_H)
        fb[y * SCREEN_W + x] = color;
}

static void fill_rect(uint16_t *fb, int x, int y, int w, int h, uint16_t color)
{
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = x + w > SCREEN_W ? SCREEN_W : x + w;
    int y1 = y + h > SCREEN_H ? SCREEN_H : y + h;

    for (int py = y0; py < y1; py++)
    {
        uint16_t *row = &fb[py * SCREEN_W];
        for (int px = x0; px < x1; px++)
            row[px] = color;
    }
}

static void draw_circle(uint16_t *fb, int cx, int cy, int r, uint16_t color)
{
    int r2 = r * r;
    for (int y = -r; y <= r; y++)
    {
        for (int x = -r; x <= r; x++)
        {
            if (x * x + y * y <= r2)
                put_pixel(fb, cx + x, cy + y, color);
        }
    }
}

static void draw_cloud(uint16_t *fb, int x, int y)
{
    uint16_t shade = COLOR(26, 30, 31);
    draw_circle(fb, x + 8, y + 8, 8, shade);
    draw_circle(fb, x + 18, y + 5, 10, shade);
    draw_circle(fb, x + 30, y + 9, 8, shade);
    fill_rect(fb, x + 8, y + 8, 24, 8, shade);
}

static void draw_char(uint16_t *fb, int x, int y, int scale, char c,
                      uint16_t color)
{
    const uint8_t *glyph = NULL;

    if (c >= '0' && c <= '9')
        glyph = font_digits[c - '0'];
    else if (c >= 'A' && c <= 'Z')
        glyph = font_letters[c - 'A'];
    else if (c >= 'a' && c <= 'z')
        glyph = font_letters[c - 'a'];
    else if (c == ' ')
        return;

    if (!glyph)
        return;

    for (int row = 0; row < 7; row++)
    {
        for (int col = 0; col < 5; col++)
        {
            if (glyph[row] & (1 << (4 - col)))
                fill_rect(fb, x + col * scale, y + row * scale, scale, scale, color);
        }
    }
}

static int text_width(const char *text, int scale)
{
    return (int)strlen(text) * 6 * scale;
}

static void draw_text(uint16_t *fb, int x, int y, int scale, const char *text,
                      uint16_t color)
{
    while (*text)
    {
        draw_char(fb, x, y, scale, *text, color);
        x += 6 * scale;
        text++;
    }
}

static void draw_text_centered(uint16_t *fb, int y, int scale, const char *text,
                               uint16_t color)
{
    draw_text(fb, (SCREEN_W - text_width(text, scale)) / 2, y, scale, text, color);
}

static void draw_score(uint16_t *fb, const FlappyGame *game)
{
    char text[12];
    snprintf(text, sizeof(text), "%d", game->score);

    uint16_t shadow = COLOR(4, 10, 12);
    uint16_t white = COLOR(31, 31, 31);
    int x = (SCREEN_W - text_width(text, 3)) / 2;

    draw_text(fb, x + 2, 13, 3, text, shadow);
    draw_text(fb, x, 11, 3, text, white);
}

static void draw_pipe(uint16_t *fb, const Pipe *pipe)
{
    int top_h = pipe->gap_y - PIPE_GAP / 2;
    int bottom_y = pipe->gap_y + PIPE_GAP / 2;

    uint16_t dark = COLOR(7, 18, 5);
    uint16_t mid = COLOR(10, 25, 7);
    uint16_t light = COLOR(19, 31, 10);

    fill_rect(fb, pipe->x, 0, PIPE_W, top_h, mid);
    fill_rect(fb, pipe->x - 3, top_h - 10, PIPE_W + 6, 10, light);
    fill_rect(fb, pipe->x + PIPE_W - 7, 0, 5, top_h, dark);
    fill_rect(fb, pipe->x + 4, 0, 4, top_h, light);

    fill_rect(fb, pipe->x, bottom_y, PIPE_W, GROUND_Y - bottom_y, mid);
    fill_rect(fb, pipe->x - 3, bottom_y, PIPE_W + 6, 10, light);
    fill_rect(fb, pipe->x + PIPE_W - 7, bottom_y, 5, GROUND_Y - bottom_y, dark);
    fill_rect(fb, pipe->x + 4, bottom_y, 4, GROUND_Y - bottom_y, light);
}

static void draw_bird(uint16_t *fb, const FlappyGame *game)
{
    int y = game->bird_y >> FP_SHIFT;
    int wing = (game->frame_count / 5) & 1;

    uint16_t outline = COLOR(7, 6, 5);
    uint16_t body = COLOR(31, 25, 5);
    uint16_t belly = COLOR(31, 30, 16);
    uint16_t wing_color = COLOR(28, 15, 4);
    uint16_t beak = COLOR(31, 12, 2);
    uint16_t white = COLOR(31, 31, 31);
    uint16_t black = COLOR(0, 0, 0);

    fill_rect(fb, BIRD_X + 2, y + 2, 12, 8, outline);
    fill_rect(fb, BIRD_X + 3, y + 1, 10, 10, body);
    fill_rect(fb, BIRD_X + 6, y + 7, 7, 3, belly);
    fill_rect(fb, BIRD_X + 1, y + 5 + wing * 2, 7, 4, wing_color);
    fill_rect(fb, BIRD_X + 13, y + 5, 6, 3, beak);
    fill_rect(fb, BIRD_X + 10, y + 3, 4, 4, white);
    put_pixel(fb, BIRD_X + 12, y + 4, black);
}

static void draw_background(uint16_t *fb, const FlappyGame *game)
{
    uint16_t sky_top = COLOR(13, 25, 31);
    uint16_t sky_bottom = COLOR(18, 29, 31);
    uint16_t ground = COLOR(21, 17, 7);
    uint16_t grass = COLOR(8, 24, 8);
    uint16_t stripe = COLOR(27, 22, 10);

    for (int y = 0; y < GROUND_Y; y++)
    {
        uint16_t color = y < 84 ? sky_top : sky_bottom;
        for (int x = 0; x < SCREEN_W; x++)
            fb[y * SCREEN_W + x] = color;
    }

    draw_cloud(fb, 22 - (game->frame_count / 2) % 320, 25);
    draw_cloud(fb, 134 - (game->frame_count / 3) % 330, 54);
    draw_cloud(fb, 245 - (game->frame_count / 2) % 320, 20);

    fill_rect(fb, 0, GROUND_Y, SCREEN_W, 5, grass);
    fill_rect(fb, 0, GROUND_Y + 5, SCREEN_W, SCREEN_H - GROUND_Y - 5, ground);

    for (int x = -32 + game->ground_scroll; x < SCREEN_W; x += 16)
        fill_rect(fb, x, GROUND_Y + 12, 8, 4, stripe);
}

void flappy_render_game(int main_bg, const FlappyGame *game)
{
    uint16_t *fb = bgGetGfxPtr(main_bg);

    if (bgGetMapBase(main_bg) == 8)
        bgSetMapBase(main_bg, 0);
    else
        bgSetMapBase(main_bg, 8);

    draw_background(fb, game);

    for (int i = 0; i < PIPE_COUNT; i++)
        draw_pipe(fb, &game->pipes[i]);

    draw_bird(fb, game);
    draw_score(fb, game);

    if (game->state == GAME_TITLE)
    {
        draw_text_centered(fb, 48, 3, "FLAPPY", COLOR(31, 31, 31));
        draw_text_centered(fb, 76, 2, "TEST", COLOR(31, 28, 8));
        draw_text_centered(fb, 112, 1, "TAP TO START", COLOR(31, 31, 31));
    }
    else if (game->state == GAME_PAUSED)
    {
        fill_rect(fb, 47, 69, 162, 39, COLOR(5, 10, 13));
        draw_text_centered(fb, 78, 2, "PAUSED", COLOR(31, 31, 31));
    }
    else if (game->state == GAME_OVER)
    {
        fill_rect(fb, 37, 54, 182, 70, COLOR(5, 10, 13));
        draw_text_centered(fb, 64, 2, "GAME OVER", COLOR(31, 31, 31));
        draw_text_centered(fb, 93, 1, "TAP TO RETRY", COLOR(31, 28, 8));
    }
}

void flappy_render_bottom(PrintConsole *console, const FlappyGame *game,
                          FlappyHudCache *cache)
{
    if (game->state == cache->state && game->score == cache->score)
        return;

    cache->state = game->state;
    cache->score = game->score;

    consoleSelect(console);
    consoleClear();

    printf("\n\n");
    printf("        FLAPPY TEST\n\n");
    printf("   Touch this screen to flap\n\n");
    printf("   Score: %d\n", game->score);
    printf("   Best:  %d\n\n", game->best_score);

    if (game->state == GAME_TITLE)
        printf("   Tap, A, or Up to start.\n");
    else if (game->state == GAME_PLAYING)
        printf("   START pauses the game.\n");
    else if (game->state == GAME_PAUSED)
        printf("   START resumes the game.\n");
    else
        printf("   Tap to retry. SELECT title.\n");
}
