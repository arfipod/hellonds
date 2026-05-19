// SPDX-License-Identifier: CC0-1.0
//
// xorshift32 is tiny, fast, and good enough for choosing random citations.

#include "gospel_random.h"

static uint32_t rng_state = 0xcee2026u;

uint32_t gospel_random_next(void)
{
    if (rng_state == 0)
        rng_state = 0x6d2b79f5u;

    rng_state ^= rng_state << 13;
    rng_state ^= rng_state >> 17;
    rng_state ^= rng_state << 5;
    return rng_state;
}

void gospel_random_mix(uint32_t value)
{
    rng_state ^= value + 0x9e3779b9u + (rng_state << 6) + (rng_state >> 2);
    gospel_random_next();
}
