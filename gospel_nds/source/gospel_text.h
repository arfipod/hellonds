// SPDX-License-Identifier: CC0-1.0
//
// Text wrapping and two-screen page layout for the Gospel reader.

#ifndef GOSPEL_TEXT_H
#define GOSPEL_TEXT_H

#define GOSPEL_TEXT_COLS 31
#define GOSPEL_TEXT_ROWS 18
#define GOSPEL_PAGE_ROWS 23
#define GOSPEL_PAGE_TEXT_ROWS (GOSPEL_PAGE_ROWS - 1)
#define GOSPEL_PAGE_TOTAL_ROWS (GOSPEL_PAGE_ROWS * 2)
#define GOSPEL_PAGE_TOTAL_TEXT_ROWS (GOSPEL_PAGE_TEXT_ROWS * 2)
#define GOSPEL_SIDEWAYS_COLS 23
#define GOSPEL_SIDEWAYS_ROWS 32
#define GOSPEL_SIDEWAYS_TEXT_ROWS (GOSPEL_SIDEWAYS_ROWS - 1)
#define GOSPEL_SIDEWAYS_TOTAL_TEXT_ROWS (GOSPEL_SIDEWAYS_TEXT_ROWS * 2)
#define GOSPEL_MAX_LINES 80
#define GOSPEL_LINE_LEN 33

typedef char GospelLine[GOSPEL_LINE_LEN];

// Bounds an integer. Kept here because UI and layout code both need it.
int gospel_clamp_int(int value, int min_value, int max_value);

// Copies one console line and always terminates it.
void gospel_copy_line(GospelLine dest, const char *source);

// Wraps plain console text into fixed-width lines. Returns the line count.
int gospel_wrap_text(const char *text, GospelLine lines[], int max_lines);

// Wraps plain console text into a caller-selected width.
int gospel_wrap_text_width(const char *text, GospelLine lines[], int max_lines, int cols);

// Builds a full book-mode page, returning the number of text lines produced.
void gospel_build_page_lines(int start_index, GospelLine lines[], int max_lines,
                             int *line_count, int *next_index);

// Builds a book page with caller-selected wrapping.
void gospel_build_page_lines_width(int start_index, GospelLine lines[], int max_lines,
                                   int cols, int *line_count, int *next_index);

// Finds the best previous starting verse for one book-mode page step.
int gospel_find_previous_page_start(int current_index);

// Finds the previous page start for a caller-selected wrapping.
int gospel_find_previous_page_start_width(int current_index, int cols, int page_text_rows);

#endif
