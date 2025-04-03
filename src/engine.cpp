#include "engine.h"

local Engine global_engine_object = {};

Engine* Global_Engine() {
    local bool initialized = false;

    if (!initialized) {
        global_engine_object.global_elapsed_time     = 0;
        global_engine_object.last_elapsed_delta_time = 0;
        global_engine_object.main_arena              = Memory_Arena((char*)"Main Arena", Megabyte(128));
        global_engine_object.scratch_arena           = Memory_Arena((char*)"Scratch Arena", Megabyte(128));
        // global_engine_object.scratch_arena           = global_engine_object.main_arena.sub_arena(Megabyte(2));
        global_engine_object.running                 = true;
        global_engine_object.global_timescale        = 1.0f;
        initialized = true;
    }

    return &global_engine_object;
}

void Engine::die() {
    running = false;
}

local char* memory_strings(size_t in_bytes) {
    return format_temp("%llu(B) (%llu)(KB) (%llu)(MB) (%llu)(GB)",
                       in_bytes,
                       in_bytes / Kilobyte(1),
                       in_bytes / Megabyte(1),
                       in_bytes / Gigabyte(1));
}

local char* biggest_valid_memory_string(size_t in_bytes) {
    size_t bytes     = in_bytes/Byte(1);
    size_t kilobytes = in_bytes/Kilobyte(1);
    size_t megabytes = in_bytes/Megabyte(1);
    size_t gigabytes = in_bytes/Gigabyte(1);

    if (gigabytes) {
        return format_temp("%llu (GB)", gigabytes);
    } else if (megabytes) {
        return format_temp("%llu (MB)", megabytes);
    } else if (kilobytes) {
        return format_temp("%llu (KB)", kilobytes);
    }

    return format_temp("%llu (B)", bytes);
}

string Engine::memory_usage_strings() {
    return string_from_cstring(format_temp(
        "(MainArena: %s / %s) (ScratchArena: %s / %s)",
        biggest_valid_memory_string(main_arena.used + main_arena.used_top),
        biggest_valid_memory_string(main_arena.capacity),
        biggest_valid_memory_string(scratch_arena.used + scratch_arena.used_top),
        biggest_valid_memory_string(scratch_arena.capacity)
    ));
}
