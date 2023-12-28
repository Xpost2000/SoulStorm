/*
 * Bigfile packer
 *
 * A simple flat binary file builder.
 *
 * Doesn't do any compression, it's solely meant to allow for single file distribution.
 */

// This is solely so I have the typedefs that I normally use.
#include "../src/common.h"
#include "../src/string.h"
#include "../src/serializer.h"
#include "../src/memory_arena.h"
#include "bigfile.h"

/*
    NOTE: stub required because I'm using the "common runtime" code
    from the engine, which is intended for the game code primarily...

    So I need this minor little hack here for now.
*/
namespace DebugUI {
    void print(char*) {}
}

Memory_Arena bigarena;

static void add_files(string* files, int* write, string pathname) {
    if (is_path_directory(pathname)) {
        auto listing = directory_listing_list_all_files_in(&bigarena, pathname);
        for (int i = 2; i < listing.count; ++i) {
            string clone = string_clone(&bigarena, string_from_cstring(format_temp("%s/%s", listing.basename, listing.files[i].name)));
            add_files(files, write, clone);
        }
    } else {
        files[(*write)++] = (pathname);
        printf("Will pack \"%s\"\n", files[(*write)-1].data);
    }
}

int main(int argc, char** argv) {
    bigarena = Memory_Arena((char*)"bigarena", Megabyte(128)); // go crazy man.
    if (argc <= 2)  {
        printf("bigfilemake <outname> <... filenames>");
        return 1;
    } else {
        string outname = string_from_cstring(argv[1]);

        int files_to_embed_count = argc-2;
        string* files_to_embed = new string[20000]; // lol
        zero_memory(files_to_embed, 20000 * sizeof(*files_to_embed));
        int write = 0;
        for (int i = 2; i < argc; ++i) {
            add_files(files_to_embed, &write, string_from_cstring(argv[i]));
        }

        printf("%d write cursor\n", write);
        bigfile_archive_write_to_file(outname, files_to_embed, write);
        printf("Wrote packfile.\n");
        delete[] files_to_embed;
    }
    return 0;
}
