#ifndef ENGINE_H
#define ENGINE_H
/*
 * This is mostly engine global stuff
 *
 * which mostly include the shared memory arenas and the timers.
 *
 * However most of the main engine setup is in the main loop
 * and the separate components.
 */
#include "common.h"
#include "memory_arena.h"

struct Engine {
    f32 global_elapsed_time;
    f32 last_elapsed_delta_time;

    Memory_Arena main_arena;
    Memory_Arena scratch_arena;

    bool running;
};

Engine* Global_Engine();

#endif
