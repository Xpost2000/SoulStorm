#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "common.h"
#include "allocators.h"
#include "string.h"

/* NOTE copied and pasted exactly from previous project. NOTE: this is still true! Since it's pretty clean! */

// NOTE: I should probably have a way to check the "error" mode.

/* Thankfully when I do the archive related stuff, it's going to be SERIALIZER_MEMORY types... */
enum binary_serializer_type {
    BINARY_SERIALIZER_FILE,
    BINARY_SERIALIZER_MEMORY,
};
enum binary_serializer_mode {
    BINARY_SERIALIZER_READ,
    BINARY_SERIALIZER_WRITE,
};
struct binary_serializer_memory_node {
    struct binary_serializer_memory_node* next;
    size_t size;
    uint8_t buffer[]; /*Probably GNU C only right? */
};

struct binary_serializer {
    enum binary_serializer_type type;
    enum binary_serializer_mode mode;
    enum endianess              expected_endianess;

    union {
        /*or use the file, not too picky on sources*/
        FILE* file_handle;

        struct {
            struct binary_serializer_memory_node* head;
            struct binary_serializer_memory_node* tail;
        } memory_nodes;

        struct {
            size_t size;
            uint8_t* buffer;

            size_t already_read;
        } memory_buffer;
    };
};
struct binary_serializer open_write_file_serializer(string filename);
struct binary_serializer open_read_file_serializer(string filename);
struct binary_serializer open_write_memory_serializer(void);
struct binary_serializer open_read_memory_serializer(void* buffer, size_t buffer_size);
void serializer_set_endianess(struct binary_serializer* serializer, enum endianess expected_endianess);
void serializer_finish(struct binary_serializer* serializer);
void* serializer_flatten_memory(struct binary_serializer* serializer, size_t* size);
void serializer_push_memory_node(struct binary_serializer* serializer, void* bytes, size_t size);
void serialize_bytes(struct binary_serializer* serializer, void* bytes, size_t size);
void serialize_string(IAllocator* allocator, struct binary_serializer* serializer, string* s);
void serialize_format(struct binary_serializer* serializer, char* format_string, ...);

#define Define_Serializer_Function(Typename, Type) void serialize_##Typename(struct binary_serializer* serializer, Type* obj);
Define_Serializer_Function(u64, u64);
Define_Serializer_Function(s64, s64);
Define_Serializer_Function(u32, u32);
Define_Serializer_Function(s32, s32);
Define_Serializer_Function(u16, u16);
Define_Serializer_Function(s16, s16);
Define_Serializer_Function(u8 , u8);
Define_Serializer_Function(s8 , s8);
Define_Serializer_Function(f32, f32);
Define_Serializer_Function(f64, f64);
#undef Define_Serializer_Function

#define Serialize_Fixed_Array_And_Allocate_From_Arena(serializer, arena, type, counter, array) \
    do {                                                                \
        serialize_##type(serializer, &counter);                         \
        _debugprintf("allocating: %d %s objects", counter, #array);      \
        if (serializer->mode == BINARY_SERIALIZER_READ)                 \
            array = memory_arena_push(arena, counter * sizeof(*array)); \
        serialize_bytes(serializer, array, counter * sizeof(*array));   \
    } while (0)
#define Serialize_Fixed_Array(serializer, type, counter, array) \
    do {                                                                \
        serialize_##type(serializer, &counter);                         \
        _debugprintf("allocating: %d (%s) objects", counter, #array);      \
        serialize_bytes(serializer, array, counter * sizeof(*array));   \
    } while (0)
#define Serialize_Structure(serializer, structure) \
    do {                                                                \
        serialize_bytes(serializer, &structure, sizeof(structure));     \
    } while (0)

#endif
