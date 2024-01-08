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
#include "graphics_driver.h"
struct Engine {
    f32 global_elapsed_time;
    f32 last_elapsed_delta_time;

    //NOTE: use this as a modifier when I need it
    // I don't need to affect the timescale of all updates.
    f32 global_timescale;

    Memory_Arena main_arena;
    Memory_Arena scratch_arena;

    bool running;
    Graphics_Driver* driver;

    int real_screen_width;
    int real_screen_height;

    void die();
    string memory_usage_strings();
};

Engine* Global_Engine();

#endif
