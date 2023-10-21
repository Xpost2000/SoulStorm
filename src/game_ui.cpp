#include "input.h"

#include "game_ui.h"
#include "fixed_array.h"

enum Widget_Type {
    WIDGET_TYPE_UNKNOWN,
    WIDGET_TYPE_TEXT,
    WIDGET_TYPE_BUTTON,
};

struct Widget {
    u32        type;
    V2         where;
    color32f32 modulation;
    string     text;
    f32        scale;
};

struct UI_State {
    font_cache* default_font;
    font_cache* active_font;

    struct render_commands* commands;

    Fixed_Array<Widget> widgets;
    bool in_frame;
};

// void render_commands_push_text(
//     struct render_commands* commands,
//     struct font_cache* font,
//     f32 scale,
//     V2 xy,
//     string cstring,
//     union color32f32 rgba,
//     u8 blend_mode
// );

local UI_State global_ui_state = {};

/*
  NOTE: this is sort of like a stateful IMGUI, which is an oxymoron!
 */
namespace GameUI {
    local Widget* push_label(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_TEXT) {
            result->type       = WIDGET_TYPE_TEXT;
        }
        result->where      = where;
        result->modulation = modulation;
        result->text       = text;
        result->scale      = scale;

        return result;
    }

    local Widget* push_button(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_BUTTON) {
            result->type       = WIDGET_TYPE_BUTTON;
        }
        result->where      = where;
        result->modulation = modulation;
        result->text       = text;
        result->scale      = scale;

        return result;
    }

    void set_font_active(font_cache* font) {
        global_ui_state.active_font = font;
    }

    void set_font(font_cache* font) {
        global_ui_state.default_font = font;
    }

    void label(V2 where, string text, color32f32 modulation, f32 scale) {
        assertion(global_ui_state.in_frame && "Neeed to call begin_frame first.");
        auto widget = push_label(where, text, scale, modulation);
        auto font = global_ui_state.default_font;

        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);
    }

    bool button(V2 where, string text, color32f32 modulation, f32 scale) {
        assertion(global_ui_state.in_frame && "Neeed to call begin_frame first.");
        auto widget         = push_label(where, text, scale, modulation);
        auto font           = global_ui_state.default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font);
        auto button_rect    = rectangle_f32(widget->where.x, widget->where.y, text_width, text_height);

        V2   mouse_position = Input::mouse_location();
        bool clicked        = false;

        if (rectangle_f32_intersect(button_rect, rectangle_f32(mouse_position.x, mouse_position.y, 5, 5))) {
            font    = global_ui_state.active_font;
            clicked = Input::pressed_mouse_left();
        }

        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);
        return clicked;
    }

    void initialize(Memory_Arena* arena) {
        global_ui_state.widgets = Fixed_Array<Widget>(arena, 256);
    }

    void begin_frame(struct render_commands* commands) {
        global_ui_state.in_frame = true;
        global_ui_state.commands = commands;
    }

    void end_frame() {
        global_ui_state.in_frame = false;
        global_ui_state.commands = nullptr;
    }

    void update(f32 dt) {
        for (s32 i = 0; i < (s32)global_ui_state.widgets.size; ++i) {
            auto& widget = global_ui_state.widgets[i];
            // Nothing to do right now.
        }
    }
}
