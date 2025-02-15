#include "bigfile.h"

Bigfile_Archive::Bigfile_Archive() :
    raw_data(nullptr),
    raw_data_length(0),
    entry_count(0),
    entries(nullptr),
    blob_data(nullptr)
{
}

Bigfile_Archive::~Bigfile_Archive() {
    
}

bool Bigfile_Archive::open(string filepath) {
    if (OS_file_exists(filepath)) {
        struct file_buffer filebuffer = OS_read_entire_file(heap_allocator(), filepath);
        raw_data                      = filebuffer.buffer; // take ownership of the blob
        raw_data_length               = filebuffer.length;

        {
            u8* read_pointer = raw_data;
            entry_count = *((u64*)(read_pointer));
            read_pointer += sizeof(u64);
            entries   = (Bigfile_Entry*)read_pointer;
            read_pointer += sizeof(Bigfile_Entry) * entry_count;
            blob_data = (u8*)read_pointer;
        }

        return true;
    }

    return false;
}

local string unixify_pathname(string original) {
    static char buffer[MAX_BIGFILE_PATHNAME_LENGTH];

    int write = 0;
    int i = 0;

    if (original.length >= 2) {
        if (original.data[0] == '.' && (original.data[1] == '\\' || original.data[1] == '/')) {
            //relative paths might be kind of evil so be careful.
            i += 2;
        }
    }

    while (i < original.length) {
        switch (original.data[i]) {
            case '/':
            case '\\': {
                buffer[write++] = '/';
                while (original.data[i] == '\\' ||
                       original.data[i] == '/'  &&
                       i < original.length) {
                    i++;
                }

            } break;
            default: {
                buffer[write++] = original.data[i++];
            } break;
        }
    }

    buffer[write++] = '\0';
    return string_from_cstring(buffer);
}

Bigfile_Entry* Bigfile_Archive::find(string name) {
    name = unixify_pathname(name);

    for (unsigned index = 0; index < entry_count; ++index) {
        auto&  entry      = entries[index];
        string entry_name = string_from_cstring(entry.name);

        if (string_equal(name, entry_name)) {
            return &entry;
        }
    }

    return nullptr;
}

void Bigfile_Archive::finish(void) {
    entry_count = 0;
    entries     = nullptr;
    blob_data   = nullptr;
    if (raw_data) {
        free(raw_data);
    }
    raw_data = nullptr;
}

u8* Bigfile_Archive::get_blob_bytes(u64 offset_to_start) {
    return (blob_data + offset_to_start);
}

void bigfile_archive_write_to_file(string filename, string* files_to_embed, u64 files_to_embed_length) {
    struct binary_serializer file_serializer     = open_write_file_serializer(filename);
    struct file_buffer*      file_buffers        = nullptr;
    Bigfile_Entry*           entries_array       = nullptr;

    u8* array_memory                           = (u8*)malloc(
        sizeof(*entries_array) * files_to_embed_length +
        sizeof(*file_buffers)  * files_to_embed_length
    );
    
    u64 write_offset = 0;
    file_buffers     = (struct file_buffer*)(array_memory);
    entries_array    = (Bigfile_Entry*)(array_memory + sizeof(*file_buffers)*files_to_embed_length);

    // pre-estimate everything here
    for (unsigned file_index = 0; file_index < files_to_embed_length; ++file_index) {
        string entry_name = files_to_embed[file_index];
        auto&  entry      = entries_array[file_index];

        assert(entry_name.data);
        file_buffers[file_index] = OS_read_entire_file(heap_allocator(), entry_name);
        zero_array(entry.name);
        copy_string_into_cstring(unixify_pathname(entry_name), entry.name, MAX_BIGFILE_PATHNAME_LENGTH);
        entry.offset_to_start = write_offset;
        entry.length          = file_buffers[file_index].length;

        write_offset += (entry.length+1); // invisible null terminator
    }

    // actually write here
    serialize_u64(&file_serializer, &files_to_embed_length);
    serialize_bytes(&file_serializer, entries_array, sizeof(Bigfile_Entry) * files_to_embed_length);
    for (unsigned file_index = 0; file_index < files_to_embed_length; ++file_index) {
        auto& file_buffer = file_buffers[file_index];
        // NOTE: struct file_buffer implicitly null terminate the data it has so
        // that it can safely be used by other things that expect it. So I also need to write
        // the null terminator.
        //
        // This is because in order to avoid allocation, all file "access" is just offsets into the
        // giant blob. (the entire bigfile is always in memory which is probably not **good**, but
        // it's fine for 2D game like this one. Even then changing the API to legitimately stream from disk instead
        // of keeping it all in memory isn't really that difficult, but would burden the caller more imo.)
        serialize_bytes(&file_serializer, file_buffer.buffer, file_buffer.length+1);
        _debugprintf("Serialized %d bytes\n", file_buffer.length);
    }

    // bye
    for (unsigned file_index = 0; file_index < files_to_embed_length; ++file_index) {
        file_buffer_free(&file_buffers[file_index]);
    }
    free(array_memory);
    serializer_finish(&file_serializer);
}
