#include "virtual_file_system.h"
#include "bigfile.h"

local size_t          _bigfile_archive_count = 0;
local Bigfile_Archive _bigfile_archives[MAX_MOUNTABLE_BIGFILE_ARCHIVES];

void VFS_mount_archive(string pathname) {
    auto& current_archive = _bigfile_archives[_bigfile_archive_count++];
    if (!current_archive.open(pathname)) {
        _debugprintf("Failed to mount archive \"%.*s\"", pathname.length, pathname.data);
        _bigfile_archive_count--;
    } else {
        _debugprintf("Successfully mounted archive \"%.*s\" as archive %d", pathname.length, pathname.data, (_bigfile_archive_count-1));
    }
}

void VFS_finish(void) {
    _debugprintf("Unmount and deallocate all bigfiles");
    for (unsigned archive_index = 0; archive_index < _bigfile_archive_count; ++archive_index) {
        _bigfile_archives[archive_index].finish();
    }
}

local Bigfile_Entry* _VFS_look_for_entry_in_archives(
    string pathname,
    Bigfile_Archive** archive_result = nullptr
)
{
    Bigfile_Entry* result = nullptr;

    for (unsigned archive_index = 0; archive_index < _bigfile_archive_count; ++archive_index) {
        result = _bigfile_archives[archive_index].find(pathname);

        if (result) {
            _debugprintf("Found \"%.*s\" in archive %d",
                         pathname.length, pathname.data,
                         archive_index);
            if (archive_result) {
                (*archive_result) = &_bigfile_archives[archive_index];
            }
            return result;
        }
    }
    if (archive_result) {
        (*archive_result) = nullptr;
    }
    return result;
}

bool VFS_file_exists(string pathname) {
    auto archive_entry = _VFS_look_for_entry_in_archives(pathname);

    if (archive_entry) {
        return true;
    }

    _debugprintf("Checking actual file system for file");
    return OS_file_exists(pathname);
}

size_t VFS_file_length(string pathname, bool priority_to_archive) {
    auto archive_entry = _VFS_look_for_entry_in_archives(pathname);

    if (priority_to_archive) {
        if (archive_entry) {
            return archive_entry->length;
        }

        _debugprintf("Checking actual file system for file");
        return OS_file_length(pathname);
    } else {
        if (OS_file_exists(pathname)) {
            _debugprintf("Checking actual file system for file");
            return OS_file_length(pathname);
        } else {
            if (archive_entry) {
                return archive_entry->length;
            }
        }
    }

    return 0;
}

void VFS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length, bool priority_to_archive) {
    Bigfile_Archive* which_archive = nullptr;
    auto             archive_entry = _VFS_look_for_entry_in_archives(path, &which_archive);

#ifdef ONLY_LOAD_FROM_BIGFILES
    assert(archive_entry && "This is assumed to be a file that should exist. [DEBUG]");
    _debugprintf("Able to copy from bigfile archive");
    memory_copy(
        which_archive->get_blob_bytes(archive_entry->offset_to_start),
        buffer,
        buffer_length
    );
    return;
#endif

    if (priority_to_archive) {
        if (archive_entry) {
            _debugprintf("Able to copy from bigfile archive");
            memory_copy(
                which_archive->get_blob_bytes(archive_entry->offset_to_start),
                buffer,
                buffer_length
            );
        } else {
            _debugprintf("Had to read from real file");
            OS_read_entire_file_into_buffer(path, buffer, buffer_length);
        }
    } else {
        if (OS_file_exists(path)) {
            _debugprintf("Had to read from real file");
            OS_read_entire_file_into_buffer(path, buffer, buffer_length);
        } else {
            _debugprintf("Able to copy from bigfile archive");
            if (archive_entry) {
                memory_copy(
                    which_archive->get_blob_bytes(archive_entry->offset_to_start),
                    buffer,
                    buffer_length
                );
            }
        }
    }
}

struct file_buffer VFS_read_entire_file(IAllocator allocator, string path, bool priority_to_archive) {
    struct file_buffer result;
    size_t file_size = VFS_file_length(path, priority_to_archive);
    u8*    file_buffer = (u8*)allocator.alloc(&allocator, file_size+1);
    file_buffer[file_size] = 0;
    VFS_read_entire_file_into_buffer(path, file_buffer, file_size, priority_to_archive);

    result.allocator = allocator;
    result.buffer    = file_buffer;
    result.length    = file_size;
    return result;
}

