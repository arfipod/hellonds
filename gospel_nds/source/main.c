// SPDX-License-Identifier: CC0-1.0
//
// Gospel reader for Nintendo DS.

#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gospel_data.h"

#define TEXT_COLS 31
#define TEXT_ROWS 18
#define MAX_LINES 80
#define LINE_LEN 33
#define ENYE_CHAR 0x7f
#define CONSOLE_FONT_BYTES (96 * 8)

typedef enum
{
    SCREEN_MENU,
    SCREEN_READER,
    SCREEN_PICKER
} AppScreen;

typedef enum
{
    PICK_BOOK,
    PICK_CHAPTER,
    PICK_VERSE
} PickerField;

static PrintConsole top_console;
static PrintConsole bottom_console;

static AppScreen screen = SCREEN_MENU;
static PickerField picker_field = PICK_BOOK;

static int current_index = 0;
static int scroll_line = 0;
static int wrapped_count = 0;
static char wrapped_lines[MAX_LINES][LINE_LEN];
static int picker_book = 0;
static int picker_chapter = 1;
static int picker_verse = 1;
static bool dirty_top = true;
static bool dirty_bottom = true;
static uint32_t rng_state = 0xcee2026u;
static uint8_t console_font_tiles[CONSOLE_FONT_BYTES];

extern const uint8_t default_fontTiles[];

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

static uint32_t next_random(void)
{
    rng_state = rng_state * 1664525u + 1013904223u;
    return rng_state;
}

static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value)
        return min_value;
    if (value > max_value)
        return max_value;
    return value;
}

static int current_book_index(void)
{
    for (int i = 0; i < gospel_book_count; i++)
    {
        const GospelBook *book = &gospel_books[i];
        if (current_index >= book->first_verse &&
            current_index < book->first_verse + book->verse_count)
            return i;
    }
    return 0;
}

static void normalize_picker(void)
{
    picker_book = clamp_int(picker_book, 0, gospel_book_count - 1);

    const GospelBook *book = &gospel_books[picker_book];
    picker_chapter = clamp_int(picker_chapter, 1, book->chapter_count);
    picker_verse = clamp_int(picker_verse, 1, book->verses_per_chapter[picker_chapter - 1]);
}

static void picker_from_current(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];
    picker_book = current_book_index();
    picker_chapter = verse->chapter;
    picker_verse = verse->verse;
}

static void clear_console(PrintConsole *console)
{
    consoleSelect(console);
    printf("\x1b[2J\x1b[H");
}

static void add_wrapped_line(const char *line, int len)
{
    if (wrapped_count >= MAX_LINES)
        return;

    len = clamp_int(len, 0, LINE_LEN - 1);
    memcpy(wrapped_lines[wrapped_count], line, len);
    wrapped_lines[wrapped_count][len] = '\0';
    wrapped_count++;
}

static void wrap_text(const char *text)
{
    wrapped_count = 0;
    const char *line = text;
    int line_len = 0;
    int last_space = -1;

    for (const char *p = text;; p++)
    {
        char c = *p;
        bool end = c == '\0';
        bool newline = c == '\n';

        if (end || newline)
        {
            add_wrapped_line(line, line_len);
            if (end)
                break;
            line = p + 1;
            line_len = 0;
            last_space = -1;
            continue;
        }

        if (c == ' ')
            last_space = line_len;

        if (line_len >= TEXT_COLS)
        {
            int cut = last_space > 0 ? last_space : TEXT_COLS;
            add_wrapped_line(line, cut);

            line += cut;
            while (*line == ' ')
                line++;
            p = line - 1;
            line_len = 0;
            last_space = -1;
            continue;
        }

        line_len++;
    }

    if (wrapped_count == 0)
        add_wrapped_line("", 0);
}

static void set_current_index(int index)
{
    current_index = clamp_int(index, 0, gospel_verse_count - 1);
    scroll_line = 0;
    dirty_top = true;
    dirty_bottom = true;
}

static void go_random(void)
{
    int book_index = next_random() % gospel_book_count;
    const GospelBook *book = &gospel_books[book_index];
    int offset = next_random() % book->verse_count;

    set_current_index(book->first_verse + offset);
    screen = SCREEN_READER;
}

static void go_selected_citation(void)
{
    normalize_picker();
    int found = gospel_find_verse(picker_book, picker_chapter, picker_verse);
    if (found >= 0)
    {
        set_current_index(found);
        screen = SCREEN_READER;
    }
}

static void draw_menu_top(void)
{
    clear_console(&top_console);
    printf("EVANGELIO NDS\n");
    printf("Angel R.\n");
    printf("Version Conferencia\n");
    printf("Episcopal Espa\177ola\n\n");
    printf("Elige un modo en la\n");
    printf("pantalla inferior.\n\n");
    printf("Lectura seguida avanza\n");
    printf("por los cuatro evangelios.\n");
    printf("Buscar por cita usa un\n");
    printf("selector de libro, capitulo\n");
    printf("y versiculo.\n");
}

static void draw_menu_bottom(void)
{
    clear_console(&bottom_console);
    printf("A  Lectura seguida\n");
    printf("B  Buscar por cita\n");
    printf("X  Evangelio al azar\n\n");
    printf("START tambien empieza\n");
    printf("la lectura desde Mateo 1,1.");
}

static void draw_reader_top(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];
    char header[64];

    snprintf(header, sizeof(header), "%s %d,%d", verse->abbr, verse->chapter, verse->verse);
    wrap_text(verse->text);

    int max_scroll = wrapped_count > TEXT_ROWS ? wrapped_count - TEXT_ROWS : 0;
    scroll_line = clamp_int(scroll_line, 0, max_scroll);

    clear_console(&top_console);
    printf("%-31s\n", header);
    printf("-------------------------------\n");

    for (int row = 0; row < TEXT_ROWS; row++)
    {
        int index = scroll_line + row;
        if (index < wrapped_count)
            printf("%-31s\n", wrapped_lines[index]);
        else
            printf("\n");
    }

    if (max_scroll > 0)
        printf("Linea %02d/%02d", scroll_line + 1, wrapped_count);
}

static void draw_reader_bottom(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];

    clear_console(&bottom_console);
    printf("%s %d,%d\n", verse->book, verse->chapter, verse->verse);
    printf("A/R/Derecha  siguiente\n");
    printf("L/Izquierda   anterior\n");
    printf("Arriba/Abajo  desplazar\n");
    printf("B             menu\n");
    printf("Y             buscar cita\n");
    printf("X             azar");
}

static void draw_picker_top(void)
{
    const GospelBook *book = &gospel_books[picker_book];

    clear_console(&top_console);
    printf("BUSCAR POR CITA\n\n");
    printf("%c Libro:     %s\n", picker_field == PICK_BOOK ? '>' : ' ', book->name);
    printf("%c Capitulo:  %d de %d\n", picker_field == PICK_CHAPTER ? '>' : ' ', picker_chapter, book->chapter_count);
    printf("%c Versiculo: %d de %d\n", picker_field == PICK_VERSE ? '>' : ' ', picker_verse, book->verses_per_chapter[picker_chapter - 1]);
    printf("\nReferencia: %s %d,%d\n\n", book->abbr, picker_chapter, picker_verse);
    printf("Pulsa A para abrirla.\n");
}

static void draw_picker_bottom(void)
{
    clear_console(&bottom_console);
    printf("Izq/Der  cambiar campo\n");
    printf("Arr/Abj  ajustar valor\n");
    printf("L/R      saltos rapidos\n");
    printf("A        leer cita\n");
    printf("B        menu\n");
    printf("X        azar");
}

static void redraw(void)
{
    if (dirty_top)
    {
        if (screen == SCREEN_MENU)
            draw_menu_top();
        else if (screen == SCREEN_READER)
            draw_reader_top();
        else
            draw_picker_top();
        dirty_top = false;
    }

    if (dirty_bottom)
    {
        if (screen == SCREEN_MENU)
            draw_menu_bottom();
        else if (screen == SCREEN_READER)
            draw_reader_bottom();
        else
            draw_picker_bottom();
        dirty_bottom = false;
    }
}

static void handle_menu(uint32_t down)
{
    if (down & (KEY_A | KEY_START))
    {
        set_current_index(0);
        screen = SCREEN_READER;
    }
    else if (down & KEY_B)
    {
        picker_from_current();
        screen = SCREEN_PICKER;
        dirty_top = true;
        dirty_bottom = true;
    }
    else if (down & KEY_X)
    {
        go_random();
    }
}

static void handle_reader(uint32_t down)
{
    if (down & KEY_B)
    {
        screen = SCREEN_MENU;
        dirty_top = true;
        dirty_bottom = true;
        return;
    }

    if (down & KEY_Y)
    {
        picker_from_current();
        screen = SCREEN_PICKER;
        dirty_top = true;
        dirty_bottom = true;
        return;
    }

    if (down & KEY_X)
    {
        go_random();
        return;
    }

    if (down & (KEY_A | KEY_RIGHT | KEY_R))
        set_current_index(gospel_next_verse_index(current_index));
    else if (down & (KEY_LEFT | KEY_L))
        set_current_index(gospel_prev_verse_index(current_index));
    else if (down & KEY_DOWN)
    {
        scroll_line++;
        dirty_top = true;
    }
    else if (down & KEY_UP)
    {
        scroll_line--;
        dirty_top = true;
    }
}

static void adjust_picker_value(int delta)
{
    if (picker_field == PICK_BOOK)
        picker_book += delta;
    else if (picker_field == PICK_CHAPTER)
        picker_chapter += delta;
    else
        picker_verse += delta;

    normalize_picker();
    dirty_top = true;
}

static void handle_picker(uint32_t down)
{
    if (down & KEY_B)
    {
        screen = SCREEN_MENU;
        dirty_top = true;
        dirty_bottom = true;
        return;
    }

    if (down & KEY_X)
    {
        go_random();
        return;
    }

    if (down & KEY_A)
    {
        go_selected_citation();
        return;
    }

    if (down & KEY_RIGHT)
    {
        picker_field = (picker_field + 1) % 3;
        dirty_top = true;
    }
    else if (down & KEY_LEFT)
    {
        picker_field = (picker_field + 2) % 3;
        dirty_top = true;
    }
    else if (down & KEY_UP)
        adjust_picker_value(1);
    else if (down & KEY_DOWN)
        adjust_picker_value(-1);
    else if (down & KEY_R)
        adjust_picker_value(10);
    else if (down & KEY_L)
        adjust_picker_value(-10);
}

int main(void)
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    consoleInit(&top_console, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottom_console, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    memcpy(console_font_tiles, default_fontTiles, CONSOLE_FONT_BYTES);
    memcpy(&console_font_tiles[(ENYE_CHAR - 32) * 8], enye_glyph, sizeof(enye_glyph));
    consoleSetFont(&top_console, &console_font);
    consoleSetFont(&bottom_console, &console_font);

    rng_state ^= (uint32_t)time(NULL);
    irqInit();
    irqEnable(IRQ_VBLANK);

    while (1)
    {
        scanKeys();
        uint32_t down = keysDown();

        if (screen == SCREEN_MENU)
            handle_menu(down);
        else if (screen == SCREEN_READER)
            handle_reader(down);
        else
            handle_picker(down);

        redraw();
        swiWaitForVBlank();
    }
}
