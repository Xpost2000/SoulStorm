#ifndef DEBUG_UI_H
#define DEBUG_UI_H

// calling this "a debug UI" is pretty generous.
// I just need a convenient way to printf debug without using
// the console, since I can't read stuff from there sometimes...
// I'm just going to keep this to the last N lines with a basic log.
// but I don't need really visualization or anything crazy...

// To be fair, the UI for all of my game-like projects
// (except for Legends, I was actually really happy with the result
// of the stuff I did make for that. That's pretty rare) is something
// I wish I could work more on, but it usually has to be auxiliary and
// just "functional" and not too uncomfortable since the other parts are
// more important.

#define DEBUG_UI_MAX_STORED_LINES (16)
#define DEBUG_UI_MAX_CHARACTER_LENGTH (512)

struct render_commands;
struct font_cache;
namespace DebugUI {
    void print(char*);
    void print(string what);
    void render(struct render_commands* commands, struct font_cache* font);
}

#endif
