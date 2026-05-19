// SPDX-License-Identifier: CC0-1.0
//
// Console setup shared by all Gospel NDS screens.

#ifndef GOSPEL_CONSOLE_H
#define GOSPEL_CONSOLE_H

#include <nds.h>

// Initializes both DS text consoles and installs the custom glyph used for ñ.
void gospel_console_init(PrintConsole *top, PrintConsole *bottom);

// Selects and clears one console. All rendering code uses this before drawing.
void gospel_console_clear(PrintConsole *console);

#endif
