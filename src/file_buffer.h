#ifndef FILE_BUFFER_H
#define FILE_BUFFER_H

#include "allocators.h"
#include "string.h"

// NOTE: circular dependencies are pretty real I guess since I like having a massive common.h
// even though most of the reason I use it is for the typedefs.

// Implementation is in common.cpp

struct file_buffer {
    IAllocator allocator;
    u8* buffer;
    u64 length;
    u8  does_not_own_memory;
};

string             file_buffer_slice(struct file_buffer* buffer, u64 start, u64 end); 
string             file_buffer_as_string(struct file_buffer* buffer); 
bool               OS_file_exists(string path); 
size_t             OS_file_length(string path); 
void               OS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length); 
struct file_buffer OS_read_entire_file(IAllocator allocator, string path); 
void               file_buffer_free(struct file_buffer* file); 

#endif
