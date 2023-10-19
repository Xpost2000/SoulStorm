#include "engine.h"

local Engine global_engine_object = {};

Engine* Global_Engine() {
    local bool initialized = false;

    if (!initialized) {
        global_engine_object.global_elapsed_time     = 0;
        global_engine_object.last_elapsed_delta_time = 0;
        global_engine_object.main_arena              = Memory_Arena((char*)"Main Arena", Megabyte(16));
        global_engine_object.scratch_arena           = Memory_Arena((char*)"Scratch Arena", Megabyte(16));
        global_engine_object.running                 = true;
        global_engine_object.global_timescale        = 1.0f;
        initialized = true;
    }

    return &global_engine_object;
}

void Engine::die() {
    running = false;
}
