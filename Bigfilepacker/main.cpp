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

/*
    NOTE: stub required because I'm using the "common runtime" code
    from the engine, which is intended for the game code primarily...

    So I need this minor little hack here for now.
*/
namespace DebugUI {
    void print(char*) {}
}

int main(int argc, char** argv) {
    return 0;
}
