// SPDX-License-Identifier: CC0-1.0
//
// Menus, navigation, and rendering for the Gospel NDS app.

#include "gospel_app.h"

#include <stdio.h>

#include "gospel_console.h"
#include "gospel_data.h"
#include "gospel_random.h"
#include "gospel_text.h"

typedef enum
{
    SCREEN_MENU,
    SCREEN_READER,
    SCREEN_BOOK,
    SCREEN_PICKER
} AppScreen;

typedef enum
{
    PICK_BOOK,
    PICK_CHAPTER,
    PICK_VERSE
} PickerField;

static PrintConsole *top_console;
static PrintConsole *bottom_console;

static AppScreen screen = SCREEN_MENU;
static PickerField picker_field = PICK_BOOK;

static int current_index = 0;
static int scroll_line = 0;
static int next_page_index = 0;
static int picker_book = 0;
static int picker_chapter = 1;
static int picker_verse = 1;
static bool dirty_top = true;
static bool dirty_bottom = true;

void gospel_app_init(PrintConsole *top, PrintConsole *bottom)
{
    top_console = top;
    bottom_console = bottom;
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
    picker_book = gospel_clamp_int(picker_book, 0, gospel_book_count - 1);

    const GospelBook *book = &gospel_books[picker_book];
    picker_chapter = gospel_clamp_int(picker_chapter, 1, book->chapter_count);
    picker_verse = gospel_clamp_int(picker_verse, 1,
                                    book->verses_per_chapter[picker_chapter - 1]);
}

static void picker_from_current(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];
    picker_book = current_book_index();
    picker_chapter = verse->chapter;
    picker_verse = verse->verse;
}

static void set_current_index(int index)
{
    current_index = gospel_clamp_int(index, 0, gospel_verse_count - 1);
    scroll_line = 0;
    next_page_index = current_index;
    dirty_top = true;
    dirty_bottom = true;
}

static void go_random(void)
{
    gospel_random_mix(((uint32_t)timerTick(0) << 16) ^ timerTick(1));

    int book_index = (int)((gospel_random_next() >> 16) % gospel_book_count);
    const GospelBook *book = &gospel_books[book_index];
    int offset = (int)((gospel_random_next() >> 8) % book->verse_count);

    set_current_index(book->first_verse + offset);
    if (screen != SCREEN_BOOK)
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
    gospel_console_clear(top_console);
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
    gospel_console_clear(bottom_console);
    printf("A  Lectura seguida\n");
    printf("B  Buscar por cita\n");
    printf("X  Evangelio al azar\n");
    printf("Y  Modo libro\n\n");
    printf("START tambien empieza\n");
    printf("la lectura desde Mateo 1,1.");
}

static void draw_reader_top(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];
    GospelLine wrapped[GOSPEL_MAX_LINES];
    char header[64];

    snprintf(header, sizeof(header), "%s %d,%d", verse->abbr, verse->chapter, verse->verse);
    int wrapped_count = gospel_wrap_text(verse->text, wrapped, GOSPEL_MAX_LINES);

    int max_scroll = wrapped_count > GOSPEL_TEXT_ROWS ? wrapped_count - GOSPEL_TEXT_ROWS : 0;
    scroll_line = gospel_clamp_int(scroll_line, 0, max_scroll);

    gospel_console_clear(top_console);
    printf("%-31s\n", header);
    printf("-------------------------------\n");

    for (int row = 0; row < GOSPEL_TEXT_ROWS; row++)
    {
        int index = scroll_line + row;
        if (index < wrapped_count)
            printf("%-31s\n", wrapped[index]);
        else
            printf("\n");
    }

    if (max_scroll > 0)
        printf("Linea %02d/%02d", scroll_line + 1, wrapped_count);
}

static void draw_reader_bottom(void)
{
    const GospelVerse *verse = &gospel_verses[current_index];

    gospel_console_clear(bottom_console);
    printf("%s %d,%d\n", verse->book, verse->chapter, verse->verse);
    printf("A/R/Derecha  siguiente\n");
    printf("L/Izquierda   anterior\n");
    printf("Arriba/Abajo  desplazar\n");
    printf("B             menu\n");
    printf("SELECT        buscar cita\n");
    printf("Y             modo libro\n");
    printf("X             azar");
}

static void print_page_half(PrintConsole *console, GospelLine lines[], int offset)
{
    gospel_console_clear(console);

    for (int row = 0; row < GOSPEL_PAGE_ROWS; row++)
        printf("%-31s\n", lines[offset + row]);
}

static void draw_book(void)
{
    GospelLine text_lines[GOSPEL_PAGE_TOTAL_TEXT_ROWS];
    GospelLine lines[GOSPEL_PAGE_TOTAL_ROWS];
    int count = 0;

    gospel_build_page_lines(current_index, text_lines, GOSPEL_PAGE_TOTAL_TEXT_ROWS,
                            &count, &next_page_index);

    for (int i = 0; i < GOSPEL_PAGE_TOTAL_ROWS; i++)
        lines[i][0] = '\0';

    for (int i = 0; i < GOSPEL_PAGE_TEXT_ROWS; i++)
    {
        if (i < count)
            gospel_copy_line(lines[i], text_lines[i]);

        int second_page_index = GOSPEL_PAGE_TEXT_ROWS + i;
        if (second_page_index < count)
            gospel_copy_line(lines[GOSPEL_PAGE_ROWS + i], text_lines[second_page_index]);
    }

    snprintf(lines[GOSPEL_PAGE_ROWS - 1], GOSPEL_LINE_LEN, "Y normal  A pag  SEL cita");
    snprintf(lines[GOSPEL_PAGE_TOTAL_ROWS - 1], GOSPEL_LINE_LEN, "B menu   X azar   L atras");

    print_page_half(top_console, lines, 0);
    print_page_half(bottom_console, lines, GOSPEL_PAGE_ROWS);
}

static void draw_picker_top(void)
{
    const GospelBook *book = &gospel_books[picker_book];

    gospel_console_clear(top_console);
    printf("BUSCAR POR CITA\n\n");
    printf("%c Libro:     %s\n", picker_field == PICK_BOOK ? '>' : ' ', book->name);
    printf("%c Capitulo:  %d de %d\n", picker_field == PICK_CHAPTER ? '>' : ' ',
           picker_chapter, book->chapter_count);
    printf("%c Versiculo: %d de %d\n", picker_field == PICK_VERSE ? '>' : ' ',
           picker_verse, book->verses_per_chapter[picker_chapter - 1]);
    printf("\nReferencia: %s %d,%d\n\n", book->abbr, picker_chapter, picker_verse);
    printf("Pulsa A para abrirla.\n");
}

static void draw_picker_bottom(void)
{
    gospel_console_clear(bottom_console);
    printf("Izq/Der  cambiar campo\n");
    printf("Arr/Abj  ajustar valor\n");
    printf("L/R      saltos rapidos\n");
    printf("A        leer cita\n");
    printf("Y        leer como libro\n");
    printf("B        menu\n");
    printf("X        azar");
}

void gospel_app_redraw(void)
{
    if (dirty_top)
    {
        if (screen == SCREEN_MENU)
            draw_menu_top();
        else if (screen == SCREEN_READER)
            draw_reader_top();
        else if (screen == SCREEN_BOOK)
            draw_book();
        else
            draw_picker_top();
        dirty_top = false;
        if (screen == SCREEN_BOOK)
            dirty_bottom = false;
    }

    if (dirty_bottom)
    {
        if (screen == SCREEN_MENU)
            draw_menu_bottom();
        else if (screen == SCREEN_READER)
            draw_reader_bottom();
        else if (screen == SCREEN_BOOK)
            draw_book();
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
    else if (down & KEY_Y)
    {
        set_current_index(current_index);
        screen = SCREEN_BOOK;
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
        screen = SCREEN_BOOK;
        dirty_top = true;
        dirty_bottom = true;
        return;
    }

    if (down & KEY_SELECT)
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

static void handle_book(uint32_t down)
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
        screen = SCREEN_READER;
        dirty_top = true;
        dirty_bottom = true;
        return;
    }

    if (down & KEY_SELECT)
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
        set_current_index(next_page_index);
    else if (down & (KEY_LEFT | KEY_L))
        set_current_index(gospel_find_previous_page_start(current_index));
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

    if (down & KEY_Y)
    {
        go_selected_citation();
        if (screen == SCREEN_READER)
            screen = SCREEN_BOOK;
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

void gospel_app_handle_keys(uint32_t down)
{
    if (screen == SCREEN_MENU)
        handle_menu(down);
    else if (screen == SCREEN_READER)
        handle_reader(down);
    else if (screen == SCREEN_BOOK)
        handle_book(down);
    else
        handle_picker(down);
}
