#ifndef LUA_BINDING_MACRO_H
#define LUA_BINDING_MACRO_H

// NOTE:
// Firstly, I don't really want to look in the files to check stuff,
// and also I'm probably going to forget a lot of my own lua functions for the
// engine.

// I still have to "self-document" but at least having an API that's updated from the code
// is better than nothing...

// I don't think anyone would mod this game, but I need this because I don't remember enough
// of my own stuff I guess...

// I just need this to generate a markdown file that's "up to date"

// But this complicates the build process slightly...
#define GAME_LUA_STITCH_STRING(name) __lua_bind_#name
#define GAME_LUA_MODULE(name, short_description, long_description)
#define GAME_LUA_PROC(name, params_as_string, short_description, long_description) \
    int GAME_LUA_STITCH_STRING(name)(lua_State* L)
#endif