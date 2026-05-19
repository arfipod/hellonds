// SPDX-License-Identifier: CC0-1.0

#ifndef GOSPEL_DATA_H
#define GOSPEL_DATA_H

#include <stdint.h>

typedef struct
{
    const char *book;
    const char *abbr;
    uint8_t chapter;
    uint8_t verse;
    const char *text;
} GospelVerse;

typedef struct
{
    const char *name;
    const char *abbr;
    uint16_t first_verse;
    uint16_t verse_count;
    uint8_t chapter_count;
    const uint8_t *verses_per_chapter;
} GospelBook;

extern const GospelBook gospel_books[];
extern const GospelVerse gospel_verses[];
extern const uint16_t gospel_book_count;
extern const uint16_t gospel_verse_count;

int gospel_find_verse(int book_index, int chapter, int verse);
int gospel_first_verse_in_chapter(int book_index, int chapter);
int gospel_next_verse_index(int verse_index);
int gospel_prev_verse_index(int verse_index);

#endif
