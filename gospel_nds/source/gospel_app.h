// SPDX-License-Identifier: CC0-1.0
//
// High-level Gospel reader state machine.

#ifndef GOSPEL_APP_H
#define GOSPEL_APP_H

#include <nds.h>
#include <stdint.h>

// Binds the app to the two consoles created during platform startup.
void gospel_app_init(PrintConsole *top, PrintConsole *bottom);

// Applies one frame of input to the current screen.
void gospel_app_handle_keys(uint32_t down);

// Redraws dirty screens. Rendering is skipped when nothing changed.
void gospel_app_redraw(void);

#endif
