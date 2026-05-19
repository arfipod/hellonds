// SPDX-License-Identifier: CC0-1.0
//
// Flappy Bird-style Nintendo DS homebrew game.
// Top screen: framebuffer gameplay. Bottom screen: touch flap area and status.

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#define COLOR(r, g, b) ((uint16_t)(BIT(15) | RGB15((r), (g), (b))))

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

static PrintConsole bottom_console;
static int main_bg;
static uint32_t rng_state = 0x4d595df4;

static GameState state = GAME_TITLE;
static Pipe pipes[PIPE_COUNT];
static int bird_y;
static int bird_vy;
static int score;
static int best_score;
static int frame_count;
static int ground_scroll;
static int last_bottom_score = -1;
static GameState last_bottom_state = -1;

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

static uint32_t next_random(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static int random_gap_y(void)
{
    return PIPE_MIN_Y + (int)(next_random() % (PIPE_MAX_Y - PIPE_MIN_Y + 1));
}

static void reset_game(void)
{
    bird_y = 86 * FP_ONE;
    bird_vy = 0;
    score = 0;
    frame_count = 0;
    ground_scroll = 0;

    for (int i = 0; i < PIPE_COUNT; i++)
    {
        pipes[i].x = SCREEN_W + 34 + i * PIPE_SPACING;
        pipes[i].gap_y = random_gap_y();
        pipes[i].scored = false;
    }
}

static void recycle_pipe(int pipe_index)
{
    int farthest = pipes[0].x;

    for (int i = 1; i < PIPE_COUNT; i++)
    {
        if (pipes[i].x > farthest)
            farthest = pipes[i].x;
    }

    int next_x = farthest + PIPE_SPACING;
    int minimum_x = SCREEN_W + PIPE_RESPAWN_MARGIN;

    if (next_x < minimum_x)
        next_x = minimum_x;

    pipes[pipe_index].x = next_x;
    pipes[pipe_index].gap_y = random_gap_y();
    pipes[pipe_index].scored = false;
}

static void flap(void)
{
    bird_vy = FLAP_VELOCITY;
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

static void draw_score(uint16_t *fb)
{
    char text[12];
    snprintf(text, sizeof(text), "%d", score);

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

static void draw_bird(uint16_t *fb)
{
    int y = bird_y >> FP_SHIFT;
    int wing = (frame_count / 5) & 1;

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

static void draw_background(uint16_t *fb)
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

    draw_cloud(fb, 22 - (frame_count / 2) % 320, 25);
    draw_cloud(fb, 134 - (frame_count / 3) % 330, 54);
    draw_cloud(fb, 245 - (frame_count / 2) % 320, 20);

    fill_rect(fb, 0, GROUND_Y, SCREEN_W, 5, grass);
    fill_rect(fb, 0, GROUND_Y + 5, SCREEN_W, SCREEN_H - GROUND_Y - 5, ground);

    for (int x = -32 + ground_scroll; x < SCREEN_W; x += 16)
        fill_rect(fb, x, GROUND_Y + 12, 8, 4, stripe);
}

static bool bird_hits_pipe(const Pipe *pipe)
{
    int bird_top = bird_y >> FP_SHIFT;
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

static void update_playing(void)
{
    frame_count++;
    ground_scroll = (ground_scroll + PIPE_SPEED) & 15;

    bird_vy += GRAVITY;
    if (bird_vy > 960)
        bird_vy = 960;
    bird_y += bird_vy;

    for (int i = 0; i < PIPE_COUNT; i++)
    {
        pipes[i].x -= PIPE_SPEED;

        if (pipes[i].x + PIPE_W < 0)
            recycle_pipe(i);

        if (!pipes[i].scored && pipes[i].x + PIPE_W < BIRD_X)
        {
            pipes[i].scored = true;
            score++;
            if (score > best_score)
                best_score = score;
        }

        if (bird_hits_pipe(&pipes[i]))
            state = GAME_OVER;
    }

    int bird_top = bird_y >> FP_SHIFT;
    if (bird_top < 0 || bird_top + BIRD_H >= GROUND_Y)
        state = GAME_OVER;
}

static void render_game(void)
{
    uint16_t *fb = bgGetGfxPtr(main_bg);

    if (bgGetMapBase(main_bg) == 8)
        bgSetMapBase(main_bg, 0);
    else
        bgSetMapBase(main_bg, 8);

    draw_background(fb);

    for (int i = 0; i < PIPE_COUNT; i++)
        draw_pipe(fb, &pipes[i]);

    draw_bird(fb);
    draw_score(fb);

    if (state == GAME_TITLE)
    {
        draw_text_centered(fb, 48, 3, "FLAPPY", COLOR(31, 31, 31));
        draw_text_centered(fb, 76, 2, "TEST", COLOR(31, 28, 8));
        draw_text_centered(fb, 112, 1, "TAP TO START", COLOR(31, 31, 31));
    }
    else if (state == GAME_PAUSED)
    {
        fill_rect(fb, 47, 69, 162, 39, COLOR(5, 10, 13));
        draw_text_centered(fb, 78, 2, "PAUSED", COLOR(31, 31, 31));
    }
    else if (state == GAME_OVER)
    {
        fill_rect(fb, 37, 54, 182, 70, COLOR(5, 10, 13));
        draw_text_centered(fb, 64, 2, "GAME OVER", COLOR(31, 31, 31));
        draw_text_centered(fb, 93, 1, "TAP TO RETRY", COLOR(31, 28, 8));
    }
}

static void update_bottom_screen(void)
{
    if (state == last_bottom_state && score == last_bottom_score)
        return;

    last_bottom_state = state;
    last_bottom_score = score;

    consoleSelect(&bottom_console);
    consoleClear();

    printf("\n\n");
    printf("        FLAPPY TEST\n\n");
    printf("   Touch this screen to flap\n\n");
    printf("   Score: %d\n", score);
    printf("   Best:  %d\n\n", best_score);

    if (state == GAME_TITLE)
        printf("   Tap, A, or Up to start.\n");
    else if (state == GAME_PLAYING)
        printf("   START pauses the game.\n");
    else if (state == GAME_PAUSED)
        printf("   START resumes the game.\n");
    else
        printf("   Tap to retry. SELECT title.\n");
}

static void handle_input(uint16_t keys_down)
{
    bool flap_pressed = (keys_down & (KEY_TOUCH | KEY_A | KEY_UP)) != 0;

    if (state == GAME_TITLE)
    {
        if (flap_pressed)
        {
            reset_game();
            flap();
            state = GAME_PLAYING;
        }
    }
    else if (state == GAME_PLAYING)
    {
        if (flap_pressed)
            flap();

        if (keys_down & KEY_START)
            state = GAME_PAUSED;
    }
    else if (state == GAME_PAUSED)
    {
        if (keys_down & KEY_START)
            state = GAME_PLAYING;
    }
    else if (state == GAME_OVER)
    {
        if (flap_pressed)
        {
            reset_game();
            flap();
            state = GAME_PLAYING;
        }

        if (keys_down & KEY_SELECT)
        {
            reset_game();
            state = GAME_TITLE;
        }
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    defaultExceptionHandler();

    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_MAIN_BG_0x06000000, VRAM_B_MAIN_BG_0x06020000,
                        VRAM_C_SUB_BG, VRAM_D_LCD);

    main_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    consoleInit(&bottom_console, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0,
                false, true);

    reset_game();

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();
        handle_input(keysDown());

        if (state == GAME_PLAYING)
            update_playing();
        else
            frame_count++;

        render_game();
        update_bottom_screen();
    }

    return 0;
}
