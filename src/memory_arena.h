#ifndef MEMORY_ARENA_H
#define MEMORY_ARENA_H

#include "common.h"
/*
 * Stack memory arena with two ends
 *
 * ported from legends-jrpg
 */

#define MEMORY_ARENA_DEFAULT_NAME ("(no-name)")

enum memory_arena_flags {
    /* used for smarter temporary memory */
    MEMORY_ARENA_TOUCHED_BOTTOM = BIT(1),
    MEMORY_ARENA_TOUCHED_TOP    = BIT(2),
};

enum memory_arena_allocation_region {
    MEMORY_ARENA_ALLOCATION_REGION_BOTTOM,
    MEMORY_ARENA_ALLOCATION_REGION_TOP,
};

struct Temporary_Memory;

// This is just in case I ever need them in this
// engine, although since I may use std::array and std::vector
// at least for this engine, the arenas are kind of gone...
struct Memory_Arena {
    Memory_Arena(cstring name, u64 size);
    Memory_Arena();
    ~Memory_Arena();

    cstring name;
    void*   memory;
    u64     capacity;
    u64     used = 0;
    u64     used_top = 0;


    // private usage information
    u8      flags = 0;
    u8      alloc_region = MEMORY_ARENA_ALLOCATION_REGION_BOTTOM;
    u64     peak_top = 0;
    u64     peak_bottom = 0;

    void allocate_from_top() {
        alloc_region = MEMORY_ARENA_ALLOCATION_REGION_TOP;
    }

    void allocate_from_bottom() {
        alloc_region = MEMORY_ARENA_ALLOCATION_REGION_BOTTOM;
    }

    u64 get_cursor();
    void set_cursor(u64 where);

    void finish();
    void clear_top();
    void clear_bottom();
    void clear();

    void* push_unaligned(u64 amount);
    // NOTE: sub arenas are never deallocated
    Memory_Arena sub_arena(u64 amount);
};

// NOTE:
// only intended to be used in scopes... Although in practice
// this literally never gets used.
// At least it's not that much code...
struct Temporary_Memory : public Memory_Arena {
    Temporary_Memory(Memory_Arena* parent);
    ~Temporary_Memory();
    void reset_parent();
    Memory_Arena* parent = nullptr;
    u64           parent_top_marker = 0;
    u64           parent_bottom_marker = 0;
};

#endif
