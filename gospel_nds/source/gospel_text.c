// SPDX-License-Identifier: CC0-1.0
//
// Layout helpers. The renderer deals in already-wrapped console lines.

#include "gospel_text.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "gospel_data.h"

int gospel_clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value)
        return min_value;
    if (value > max_value)
        return max_value;
    return value;
}

void gospel_copy_line(GospelLine dest, const char *source)
{
    int len = 0;

    while (len < GOSPEL_LINE_LEN - 1 && source[len] != '\0')
    {
        dest[len] = source[len];
        len++;
    }

    dest[len] = '\0';
}

static void add_wrapped_line(GospelLine lines[], int *count, int max_lines,
                             const char *line, int len)
{
    if (*count >= max_lines)
        return;

    len = gospel_clamp_int(len, 0, GOSPEL_LINE_LEN - 1);
    memcpy(lines[*count], line, len);
    lines[*count][len] = '\0';
    (*count)++;
}

int gospel_wrap_text(const char *text, GospelLine lines[], int max_lines)
{
    int count = 0;
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
            add_wrapped_line(lines, &count, max_lines, line, line_len);
            if (end)
                break;
            line = p + 1;
            line_len = 0;
            last_space = -1;
            continue;
        }

        if (c == ' ')
            last_space = line_len;

        if (line_len >= GOSPEL_TEXT_COLS)
        {
            int cut = last_space > 0 ? last_space : GOSPEL_TEXT_COLS;
            add_wrapped_line(lines, &count, max_lines, line, cut);

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

    if (count == 0)
        add_wrapped_line(lines, &count, max_lines, "", 0);

    return count;
}

static int append_verse_lines(int verse_index, GospelLine lines[], int count, int limit)
{
    const GospelVerse *verse = &gospel_verses[verse_index];
    GospelLine title;
    GospelLine wrapped[GOSPEL_MAX_LINES];

    snprintf(title, sizeof(title), "%s %d,%d", verse->abbr, verse->chapter, verse->verse);
    if (count < limit)
        gospel_copy_line(lines[count++], title);

    int wrapped_count = gospel_wrap_text(verse->text, wrapped, GOSPEL_MAX_LINES);
    for (int i = 0; i < wrapped_count && count < limit; i++)
        gospel_copy_line(lines[count++], wrapped[i]);

    if (count < limit)
        lines[count++][0] = '\0';

    return count;
}

void gospel_build_page_lines(int start_index, GospelLine lines[], int max_lines,
                             int *line_count, int *next_index)
{
    int count = 0;
    int index = gospel_clamp_int(start_index, 0, gospel_verse_count - 1);

    while (index < gospel_verse_count)
    {
        GospelLine verse_lines[GOSPEL_MAX_LINES];
        int verse_line_count = append_verse_lines(index, verse_lines, 0, GOSPEL_MAX_LINES);

        if (count > 0 && count + verse_line_count > max_lines)
            break;

        for (int i = 0; i < verse_line_count && count < max_lines; i++)
            gospel_copy_line(lines[count++], verse_lines[i]);

        index++;
        if (count >= max_lines)
            break;
    }

    *line_count = count;
    *next_index = index >= gospel_verse_count ? 0 : index;
}

static int page_end_for_start(int start_index)
{
    GospelLine scratch[GOSPEL_PAGE_TOTAL_TEXT_ROWS];
    int line_count = 0;
    int next_index = start_index;

    gospel_build_page_lines(start_index, scratch, GOSPEL_PAGE_TOTAL_TEXT_ROWS,
                            &line_count, &next_index);
    return next_index;
}

int gospel_find_previous_page_start(int current_index)
{
    if (current_index <= 0)
        return 0;

    int best = gospel_prev_verse_index(current_index);

    for (int candidate = best; candidate >= 0 && current_index - candidate < 24; candidate--)
    {
        int end = page_end_for_start(candidate);
        if (end == current_index || end > current_index)
            best = candidate;
        else
            break;
    }

    return best;
}
