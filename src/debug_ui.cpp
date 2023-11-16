#include "v2.h"
#include "debug_ui.h"
#include "render_commands.h"
#include "input.h"

#include "engine.h"
#include "thread_pool.h"

#define DEBUG_UI_MAX_STORED_LINES (16)
#define DEBUG_UI_MAX_CHARACTER_LENGTH (512)

local s32  line_count                                                      = 0;
local s32  line_cursor                                                     = 0;
local char lines[DEBUG_UI_MAX_STORED_LINES][DEBUG_UI_MAX_CHARACTER_LENGTH] = {};

// 0 - do not show
// 1 - show low opacity
// 2 - show full
local int show = 0;
#ifndef RELEASE

namespace DebugUI {
    bool enabled() {
#ifndef RELEASE
        return show;
#else
        return false;
#endif
    }

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
        show += Input::is_key_pressed(KEY_F1);
        if (show > 2) show = 0;
        if (!show) return;

        const f32 scale = 1.0f;
        f32 font_height = font_cache_text_height(font) * scale;

        f32 opacity_modifier = 0.5 * show;
        render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, 250), color32u8(0, 0, 0, 128 * opacity_modifier), BLEND_MODE_ALPHA);
        for (int i = 0; i < line_count; ++i) {
            char* line = lines[i];
            V2 xy = (V2(0, i) * font_height);

            f32 a = 1.0;
            if (i > line_cursor) {
                a = 0.5;
            }
            render_commands_push_text(commands, font, scale, xy, string_from_cstring(line), color32f32(a, a, a, 1.0f * opacity_modifier), BLEND_MODE_ALPHA);
        }

        // I realize this is mildy dumb because I await all the jobs, so they should all be finished.
        // I do parallel update but not necessarily asynchronous in this engine.
        // render_commands_push_text(commands, font, 2, V2(0, commands->screen_height-64), string_from_cstring(format_temp("ThreadPool::active_jobs: %d", Thread_Pool::active_jobs())), color32f32(1,1,1,1), BLEND_MODE_ALPHA);
        render_commands_push_text(commands, font, 2, V2(0, commands->screen_height-32), Global_Engine()->memory_usage_strings(), color32f32(1,1,1,1), BLEND_MODE_ALPHA);
    }
}
#else
namespace DebugUI {
    void print(char* s) {}
    void print(string what) {}
    void render(struct render_commands* commands, struct font_cache* font) {}
}
#endif
