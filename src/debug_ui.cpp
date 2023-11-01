#include "v2.h"
#include "debug_ui.h"
#include "render_commands.h"

local s32  line_count                                                      = 0;
local s32  line_cursor                                                     = 0;
local char lines[DEBUG_UI_MAX_STORED_LINES][DEBUG_UI_MAX_CHARACTER_LENGTH] = {};

#ifndef RELEASE

namespace DebugUI {

    void print(char* s) {
        print(string_from_cstring(s));
    }

    void print(string what) {
        // hacky stuff.
        if (line_count < DEBUG_UI_MAX_STORED_LINES) line_count++;
        line_cursor %= DEBUG_UI_MAX_STORED_LINES;
        char* current_line_ptr = lines[line_cursor++];

        s32 length = min(what.length, DEBUG_UI_MAX_CHARACTER_LENGTH);
        strncpy(current_line_ptr, what.data, length);
        current_line_ptr[length] = 0;
    }

    void render(struct render_commands* commands, struct font_cache* font) {
        const f32 scale = 1.0f;
        f32 font_height = font_cache_text_height(font);

        for (int i = 0; i < line_count; ++i) {
            char* line = lines[i];
            V2 xy = (V2(0, i) * font_height);
            render_commands_push_text(commands, font, scale, xy, string_from_cstring(line), color32f32(1, 1, 1, 1), BLEND_MODE_ALPHA);
        }
    }
}
#else
namespace DebugUI {
    void print(char* s) {}
    void print(string what) {}
    void render(struct render_commands* commands, struct font_cache* font) {}
}
#endif
