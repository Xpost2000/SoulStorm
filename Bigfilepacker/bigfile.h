#ifndef BIGFILE_H
#define BIGFILE_H

#include "../src/common.h"
#include "../src/string.h"
#include "../src/allocators.h"
#include "../src/file_buffer.h"
#include "../src/serializer.h"

/*
  Flat binary concatenation archive,

  mainly only to make final distribution easier by only sending one file,
  no compression is done.

  Simpler than the previous format used in legends as that format intended to be more
  general purpose.
*/
#define MAX_BIGFILE_PATHNAME_LENGTH (128)
struct Bigfile_Entry {
    char name[MAX_BIGFILE_PATHNAME_LENGTH];
    u64  offset_to_start; // relative to the blob_data. Not from the raw_data.
    u64  length;
    char pad[4];          // this is not aligned
};

class Bigfile_Archive {
public:
    Bigfile_Archive();
    ~Bigfile_Archive();

    void           open(string filepath);
    Bigfile_Entry* find(string name);
    void           finish(void);
    u8*            get_blob_bytes(u64 offset_to_start);
private:
    u8* raw_data;
    u64 raw_data_length;

    u64 entry_count;
    Bigfile_Entry* entries;
    u8*            blob_data;
};

void bigfile_archive_write_to_file(string filename, string* files_to_embed, u64 files_to_embed_length);

#endif
