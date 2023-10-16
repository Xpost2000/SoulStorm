#include "allocators.h"
#include "memory_arena.h"

Allocator_Allocate_Function(stub_allocator_allocate) { return NULL; }
Allocator_Reallocate_Function(stub_allocator_reallocate) { return ptr; }
Allocator_Deallocate_Function(stub_allocator_deallocate) {}

Allocator_Allocate_Function(heap_allocator_allocate) {
    return malloc(amount); 
}

Allocator_Reallocate_Function(heap_allocator_reallocate) {
    return realloc(ptr, amount); 
}

Allocator_Deallocate_Function(heap_allocator_deallocate) {
    free(ptr); 
}

IAllocator heap_allocator(void) {
    return (IAllocator) {
        .userdata = nullptr,
        .alloc    = heap_allocator_allocate,
        .free     = heap_allocator_deallocate,
        .realloc  = heap_allocator_reallocate,
    };
}

Allocator_Allocate_Function(memory_arena_allocator_allocate) {
    Memory_Arena* arena = (Memory_Arena*) allocator->userdata;
    return arena->push_unaligned(amount);
}

/* memory arenas aren't expected for realloc purposes sorry! */
IAllocator memory_arena_allocator(Memory_Arena* allocator) {
    return (IAllocator) {
        .userdata = allocator,
        .alloc    = memory_arena_allocator_allocate,
        .free     = stub_allocator_deallocate,
        .realloc  = stub_allocator_reallocate,
    };
}


