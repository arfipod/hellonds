// SPDX-License-Identifier: CC0-1.0
//
// Small deterministic RNG with explicit entropy mixing points.

#ifndef GOSPEL_RANDOM_H
#define GOSPEL_RANDOM_H

#include <stdint.h>

// Mixes timing or input entropy into the current random state.
void gospel_random_mix(uint32_t value);

// Returns the next pseudo-random 32-bit value.
uint32_t gospel_random_next(void);

#endif
