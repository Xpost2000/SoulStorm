#ifndef VIRTUAL_FILE_SYSTEM_H
#define VIRTUAL_FILE_SYSTEM_H

#include "common.h"
#include "file_buffer.h"
#define MAX_MOUNTABLE_BIGFILE_ARCHIVES (32)


/* #define ONLY_LOAD_FROM_BIGFILES */

#if 1
#ifdef RELEASE
    #define PRIORITIZE_BIGFILE_ARCHIVE_FOR_DEFAULT_LOAD
#endif
#else
// Default Debug
    #define PRIORITIZE_BIGFILE_ARCHIVE_FOR_DEFAULT_LOAD
#endif

/*
 * NOTE:
 *
 * This VFS interface is dependent on the bigfile_archive
 *
 * which basically means, it's only designed to read files.
 * It cannot enumerate directories or write to anything because the bigfile_archive
 * is read only.
 */

void               VFS_mount_archive(string pathname);
void               VFS_finish(void);
bool               VFS_file_exists(string pathname); // this works the same no matter what.

// NOTE: priority_to_archive is implicitly true since I want to enforce
// priority to be looking at the archive first.
#ifdef PRIORITIZE_BIGFILE_ARCHIVE_FOR_DEFAULT_LOAD
size_t             VFS_file_length(string pathname, bool priority_to_archive=true);
void               VFS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length, bool priority_to_archive=true);
struct file_buffer VFS_read_entire_file(IAllocator allocator, string path, bool priority_to_archive=true);
#else
// In debug, default priority should be to a real file.
size_t             VFS_file_length(string pathname, bool priority_to_archive=false);
void               VFS_read_entire_file_into_buffer(string path, u8* buffer, size_t buffer_length, bool priority_to_archive=false);
struct file_buffer VFS_read_entire_file(IAllocator allocator, string path, bool priority_to_archive=false);
#endif


#endif
