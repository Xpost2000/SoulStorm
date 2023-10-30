#include "common.h"
#include "file_buffer.h"
#include "V2.h"

rectangle_f32 rectangle_f32_centered(rectangle_f32 center_region, f32 width, f32 height) {
    return rectangle_f32(
        center_region.x + (center_region.w/2) - (width/2),
        center_region.y + (center_region.h/2) - (height/2),
        width, height
    );
}

rectangle_f32 rectangle_f32_scale(rectangle_f32 a, f32 k) {
    a.x *= k;
    a.y *= k;
    a.w *= k;
    a.h *= k;
    return a;
}

bool rectangle_f32_intersect(rectangle_f32 a, rectangle_f32 b) {
    if (a.x < b.x + b.w && a.x + a.w > b.x &&
        a.y < b.y + b.h && a.y + a.h > b.y) {
        return true;
    }

    return false;
}

bool circle_f32_intersect(circle_f32 a, circle_f32 b) {
    V2 a_pos = V2(a.x, a.y);
    V2 b_pos = V2(b.x, b.y);

    V2 delta = b_pos - a_pos;
    f32 r_delta = b.r + a.r;

    return V2_dot(delta, delta) <= r_delta*r_delta;
}

void _debug_print_bitstring(u8* bytes, unsigned length) {
    unsigned bits = length * 8;
    _debugprintfhead();
    /* reverse print, higher addresses come first, lower addresses come last. To give obvious endian representation */
    for (s32 bit_index = bits-1; bit_index >= 0; --bit_index) {
        unsigned real_index   = (bit_index / 8);
        u8       current_byte = bytes[real_index];
        _debugprintf1("%d", (current_byte & BIT(bit_index % 8)) > 0);
        if (((bit_index) % 8) == 0) {
            _debugprintf1(" ");   
        }
    }
    _debugprintf1("\n");
}

string file_buffer_slice(struct file_buffer* buffer, u64 start, u64 end) {
    char* cstring_buffer = (char*)buffer->buffer;
    return string_from_cstring_length_counted(cstring_buffer + start, (end - start));
}

string file_buffer_as_string(struct file_buffer* buffer) {
    return file_buffer_slice(buffer, 0, buffer->length);
}

bool OS_file_exists(string path) {
    FILE* f = fopen(path.data, "r");

    if (f) {
        fclose(f);
        return true;
    }

    return false;
}

size_t OS_file_length(string path) {
    size_t result = 0;
    FILE*  file   = fopen(path.data, "rb+");

    if (file) {
        fseek(file, 0, SEEK_END);
        result = ftell(file);
        fclose(file);
    }

    return result;
}

void OS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length) {
    FILE* file = fopen(path.data, "rb+");
    fread(buffer, 1, buffer_length, file);
    fclose(file);
}

struct file_buffer OS_read_entire_file(IAllocator allocator, string path) {
    _debugprintf("file buffer create!");
    size_t file_size   = OS_file_length(path);
    u8*    file_buffer = (u8*)allocator.alloc(&allocator, file_size+1);
    OS_read_entire_file_into_buffer(path, file_buffer, file_size);
    file_buffer[file_size] = 0;

    struct file_buffer result;
    result.allocator = allocator;
    result.buffer = file_buffer;
    result.length = file_size;

    return result;
}

void file_buffer_free(struct file_buffer* file) {
    if (file->does_not_own_memory) {
        return;
    } else if (file->buffer) {
        file->allocator.free(&file->allocator, file->buffer);
    }
}
