#include "common.h"
#include "file_buffer.h"
#include "v2.h"
#include "memory_arena.h"

#undef UNICODE
#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <unistd.h>
#endif
#endif

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
    FILE* f = fopen(format_temp("%.*s", path.length, path.data), "r");

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
        _debugprintf("Success in reading \"%.*s\"!", path.length, path.data);
        fclose(file);
    }

    _debugprintf("Reading file length of \"%.*s\" (%lld bytes)", path.length, path.data, result);
    return result;
}

void OS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length) {
    FILE* file = fopen(path.data, "rb+");
    if (file) {
        fread(buffer, 1, buffer_length, file);
        fclose(file);
    }
}

struct file_buffer OS_read_entire_file(IAllocator allocator, string path) {
    _debugprintf("file buffer create (\"%.*s\")", path.length, path.data);
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

u64 system_get_current_time(void) {
    return time(0);
}

bool OS_create_directory(string location) {
    _debugprintf("Making directory %.*s", location.length, location.data);
    string s = string_from_cstring(format_temp("%.*s", location.length, location.data));
#ifdef _WIN32 
    DWORD error_code;
    if (!CreateDirectory(s.data, NULL)) {
      error_code = GetLastError();

      if (error_code == ERROR_ALREADY_EXISTS) {
        return true;
      }

      return false;
    }

    return true;
#else
    int error_code = mkdir(s.data, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (error_code == EEXIST) {
      return true;
    }

    return (error_code == 0);
#endif
}

Calendar_Time calendar_time_from(s64 timestamp) {
    Calendar_Time result = { 0 };
    time_t     current_time = timestamp; 
    struct tm* time_info    = localtime(&current_time);

    _debugprintf("%lld\n", timestamp);

    if (time_info) {
        result.year = time_info->tm_year + 1900;
        result.month = time_info->tm_mon;
        result.hours = time_info->tm_hour;
        result.minutes = time_info->tm_min;
        result.seconds = time_info->tm_sec;
        result.day = time_info->tm_mday;
        result.day_of_the_week = time_info->tm_wday;
        return result;
    } else {
        return result;
    }
}

Calendar_Time current_calendar_time(void) {
    return calendar_time_from(system_get_current_time());
}

local void normalize_to_unix_path(char* buffer, size_t buffer_size) {
    for (size_t index = 0; index < buffer_size; ++index) {
        if (buffer[index] == '\\') {
            buffer[index] = '/';
        }
    }
}

bool path_exists(string location) {
#ifndef __EMSCRIPTEN__
    char tmp_copy[260] = {};
    for (s32 i = 0; i < location.length; ++i) {
        tmp_copy[i] = location.data[i];
    }
    if (location.data[location.length-1] == '/' || location.data[location.length-1] == '\\') {
        tmp_copy[location.length-1] = 0;
    }

#ifdef _WIN32
    WIN32_FIND_DATA find_data = {};
    HANDLE handle = FindFirstFile(tmp_copy, &find_data);


    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    return true;
#else
    // unsafe
    normalize_to_unix_path(tmp_copy, array_count(tmp_copy));
    DIR* directory_information;
    directory_information = opendir(tmp_copy);

    if (directory_information) {
        closedir(directory_information);
        return true;
    }

    return false;
#endif
#else
    return false;
#endif
}

bool is_path_directory(string location) {
#ifndef __EMSCRIPTEN__
    char tmp_copy[260] = {};
    for (s32 i = 0; i < location.length; ++i) {
        tmp_copy[i] = location.data[i];
    }
    if (location.data[location.length-1] == '/' || location.data[location.length-1] == '\\') {
        tmp_copy[location.length-1] = 0;
    }

#ifdef _WIN32
    WIN32_FIND_DATA find_data = {};
    HANDLE handle = FindFirstFile(tmp_copy, &find_data);

    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return true;
    }

    return false;
#else                                          
    normalize_to_unix_path(tmp_copy, array_count(tmp_copy));

    struct stat file_stat_info;
    int stat_result = stat(tmp_copy, &file_stat_info);
    if(stat_result){
        // error
        _debugprintf("Stat error: (%s) %d", tmp_copy, stat_result);
    }else{
    }

    if( S_ISDIR(file_stat_info.st_mode) ) {
        return true;
    }
    return false;
#endif
#endif
    return false;
}

Directory_Listing directory_listing_list_all_files_in(Memory_Arena* arena, string location) {
    Directory_Listing result = {};
    cstring_copy(location.data, result.basename, 260);

    if (!is_path_directory(location)) {
        return result;
    }
#ifndef __EMSCRIPTEN__
#ifdef _WIN32
    WIN32_FIND_DATA find_data = {};
    HANDLE handle = FindFirstFile(string_concatenate(arena, location, string_literal("/*")).data, &find_data);

    if (handle == INVALID_HANDLE_VALUE) {
        return result;
    }

    result.files = (Directory_File*)arena->push_unaligned(sizeof(*result.files));

    do {
        {
            string entry_name = string_from_cstring(find_data.cFileName);
            if (string_equal(entry_name, string_literal(".")) ||
                string_equal(entry_name, string_literal(".."))) 
                continue;
        }

        Directory_File* current_file = &result.files[result.count++];
        current_file->is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

        cstring_copy(find_data.cFileName, current_file->name, array_count(current_file->name));
        current_file->filesize = (find_data.nFileSizeHigh * (MAXDWORD+1)) + find_data.nFileSizeLow;

        arena->push_unaligned(sizeof(*result.files));
    } while (FindNextFile(handle, &find_data));
#else
    // NOTE: this is broken!
    // linux
    normalize_to_unix_path(result.basename, array_count(result.basename));

    /* string acceptable_filepath = string_concatenate(arena, string_literal("./"), location); */
    string acceptable_filepath = string_literal(result.basename);
    _debugprintf("%s\n", acceptable_filepath.data);
    result.files = (Directory_File*)arena->push_unaligned(sizeof(*result.files));
    DIR* directory_information = opendir(acceptable_filepath.data);

    if (directory_information) {
        struct dirent* directory_entry;
        while((directory_entry = readdir(directory_information))){
	    string entry_name = string_from_cstring(directory_entry->d_name);

	    if (string_equal(entry_name, string_literal(".")) ||
		string_equal(entry_name, string_literal(".."))) 
                continue;

            Directory_File* current_file = &result.files[result.count++];
            {
                cstring_copy(directory_entry->d_name, current_file->name, array_count(current_file->name));

                struct stat file_stat_info;
                string full_path_name = string_from_cstring(format_temp("%s/%s", acceptable_filepath.data, current_file->name));
                int stat_result = stat(full_path_name.data, &file_stat_info);

                if(stat_result){
                    // error
                    _debugprintf("Stat error: %d", stat_result);
                }else{
                    current_file->filesize = file_stat_info.st_size;
                }

                if( S_ISDIR(file_stat_info.st_mode) ) {
                    current_file->is_directory = true;
                }else{
                    current_file->is_directory = false;
                }
            }

            arena->push_unaligned(sizeof(*result.files));
        }
        closedir(directory_information);
    }
#endif
#endif

    return result;
}

#if _WIN32
void* virtual_memory_reserve(size_t sz)
{
  return VirtualAlloc(0, sz, MEM_RESERVE, PAGE_READWRITE);
}

void* virtual_memory_commit(void* ptr, size_t sz)
{
  if (sz == 0) {
    return 0;
  }
  return VirtualAlloc(ptr, sz, MEM_COMMIT, PAGE_READWRITE);
}

void virtual_memory_uncommit(void* ptr, size_t sz)
{
  if (sz == 0) {
    return;
  }
  VirtualFree(ptr, sz, MEM_DECOMMIT);
}

// eh.
void virtual_memory_free(void* ptr, size_t sz)
{
  VirtualFree(ptr, sz, MEM_RELEASE);
}
#else
// mmap/munmap does not have reserve/unreserve style behavior.
void* virtual_memory_reserve(size_t sz)
{
  return mmap(NULL, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

// NO-OP on posix.
void* virtual_memory_commit(void* ptr, size_t sz)
{
  return 0;
}

void virtual_memory_uncommit(void* ptr, size_t sz)
{
  return;
}

void virtual_memory_free(void* ptr, size_t sz)
{
  munmap(ptr, sz);
}
#endif