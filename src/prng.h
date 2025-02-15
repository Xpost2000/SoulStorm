#ifndef PRNG_H
#define PRNG_H

#include "common.h"

#define RANDOM_MAX ((1 << 30) - 1)
struct random_state {
    s32 constant;
    s32 multiplier;
    s32 state;
    s32 seed;
    s32 modulus;
};

#define random_state_ext(CONSTANT, MULTIPLIER, SEED, MODULUS) \
    random_state {                                            \
        CONSTANT,                                             \
            MULTIPLIER,                                       \
            SEED,                                             \
            SEED,                                             \
            MODULUS,                                          \
            }

#define random_state_ex(constant, multiplier) random_state_ext(constant, multiplier, (s32)time(NULL), (RANDOM_MAX))
#define random_state() random_state_ex(12345, 1123515245)

s32 random_state_next(random_state* state);
void random_state_reset(random_state* state);
f32 random_float(random_state* state);
f32 random_ranged_float(random_state* state, f32 a, f32 b);
s32 random_ranged_integer(random_state* state, s32 minimum, s32 maximum);
s32 random_weighted_selection(random_state* random, f32* weights, u64 item_count);

#endif
