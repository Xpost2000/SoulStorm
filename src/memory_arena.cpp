#include "memory_arena.h"

Memory_Arena::Memory_Arena(cstring name, u64 size) {
    this->name = (cstring)((name) ? (name) : MEMORY_ARENA_DEFAULT_NAME);
    this->capacity = size;
    this->used = this->used_top = 0;
    this->memory = virtual_memory_reserve(size);

#if 0
    zero_memory(this->memory, size);
#endif
}

Memory_Arena::Memory_Arena() {
    
}

Memory_Arena::~Memory_Arena() {
    
}

void Memory_Arena::finish() {
    //free(memory);
  clear();
  virtual_memory_free(memory, capacity);
  memory = nullptr;
}

void Memory_Arena::clear_top() {
    zero_memory((void*)((u8*)memory+capacity - used_top), used_top);
    used_top = 0;
    virtual_memory_uncommit((u8*)memory + capacity - committed_top, committed_top);
    committed_top = 0;
}

void Memory_Arena::clear_bottom() {
    zero_memory(memory, used);
    used = 0;
    virtual_memory_uncommit(memory, committed);
    committed = 0;
}

void Memory_Arena::clear() {
    clear_top();
    clear_bottom();
}

void* Memory_Arena::push_unaligned(u64 amount) {
    // no bounds checking 
    if (alloc_region == MEMORY_ARENA_ALLOCATION_REGION_BOTTOM) {
        void* base_pointer = (void*)((u8*)memory + used);
        used += amount;

        if (used > peak_bottom) {
            peak_bottom = used;
        }

        flags |= MEMORY_ARENA_TOUCHED_BOTTOM;

        while (committed < used) {
          u64 commit_desired = Kilobyte(4);
          if (committed + commit_desired + committed_top > capacity) {
            commit_desired = capacity - committed - committed_top;
          }
          virtual_memory_commit((u8*) memory + committed, commit_desired);
          committed += commit_desired;
        }

        zero_memory(base_pointer, amount);
        return base_pointer;
    } else {
        void* end_of_memory = (void*)((u8*)memory + capacity);
        used_top += amount;
        void* base_pointer  = (void*)((u8*)end_of_memory - used_top);

        if (used_top > peak_top) {
            peak_top = used_top;
        }

        flags |= MEMORY_ARENA_TOUCHED_TOP;

        while (committed < used_top) {
          u64 commit_desired = Kilobyte(4);
          if (committed_top + commit_desired + committed > capacity) {
            commit_desired = capacity - committed_top - committed;
          }
          committed_top += commit_desired;
          virtual_memory_commit((u8*)memory + capacity - committed_top, commit_desired);
        }

        zero_memory(base_pointer, amount);
        return base_pointer;
    }

    assertion(used+used_top <= capacity && "Over allocated memory");
    return nullptr;
}

Memory_Arena Memory_Arena::sub_arena(u64 amount) {
    Memory_Arena sub_arena = {};
    sub_arena.name = (cstring)"SUBARENA";
    sub_arena.memory = push_unaligned(amount);
    sub_arena.capacity = amount;
    return sub_arena;
}

u64 Memory_Arena::get_cursor() {
    if (alloc_region == MEMORY_ARENA_ALLOCATION_REGION_BOTTOM) {
        return used;
    } else {
        return used_top;
    }

    return 0;
}

void Memory_Arena::set_cursor(u64 where) {
    if (alloc_region == MEMORY_ARENA_ALLOCATION_REGION_BOTTOM) {
        used = where;
        assertion(where <= used && "Setting cursor to uncharted sights.");
    } else {
        used_top = where;
        assertion(where <= used_top && "Setting top cursor to uncharted sights.");
    }
}

// Temporary_Memory_Arena
Temporary_Memory::Temporary_Memory(Memory_Arena* parent) {
    this->parent = parent;
    parent_bottom_marker = parent->used;
    parent_top_marker = parent->used_top;
    memory = parent->push_unaligned(0); // just need pointer loc
}

Temporary_Memory::~Temporary_Memory() {
    reset_parent();
}

void Temporary_Memory::reset_parent() {
    if (flags & MEMORY_ARENA_TOUCHED_BOTTOM)
        parent->used     = parent_bottom_marker;
    if (flags & MEMORY_ARENA_TOUCHED_TOP)
        parent->used_top = parent_top_marker;
}
