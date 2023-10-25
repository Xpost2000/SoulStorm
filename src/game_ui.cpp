#include "input.h"

#include "game_ui.h"
#include "fixed_array.h"

#define RELEASE

enum Widget_Type {
    WIDGET_TYPE_UNKNOWN,
    WIDGET_TYPE_TEXT,
    WIDGET_TYPE_OPTION_SELECTOR,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_CHECKBOX,
    WIDGET_TYPE_F32_SLIDER,
};

struct Widget {
    u32        type;
    V2         where;
    color32f32 modulation;
    string     text;
    f32        scale;

    // for option_selector
    s32 current_selected_index;

    s32 id;
};

struct UI_State {
    font_cache* selected_font;
    font_cache* default_font;
    font_cache* active_font;

    struct render_commands* commands;

    Fixed_Array<Widget> widgets;
    bool in_frame;

    s32 hot_index = -1;
    bool ate_any_mouse_lefts = false;
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
    local void common_widget_initialization(Widget* widget, V2 where, string text, f32 scale, color32f32 modulation) {
        widget->id         = global_ui_state.widgets.size;
        widget->where      = where;
        widget->modulation = modulation;
        widget->text       = text;
        widget->scale      = scale;
    }

    local bool is_selectable_on_active(Widget* widget) {
        return (global_ui_state.hot_index == -1 || global_ui_state.hot_index == widget->id);
    }
    local bool is_currently_hot(Widget* widget) {
        return (global_ui_state.hot_index == widget->id);
    }

    local void register_hot_widget(Widget* widget) {
        if (global_ui_state.hot_index == -1 || global_ui_state.hot_index == widget->id) {
            global_ui_state.hot_index           = widget->id;
            global_ui_state.ate_any_mouse_lefts = true;
        }
    }

    local Widget* push_label(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_TEXT) {
            result->type       = WIDGET_TYPE_TEXT;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_button(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_BUTTON) {
            result->type       = WIDGET_TYPE_BUTTON;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_checkbox(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_CHECKBOX) {
            result->type = WIDGET_TYPE_CHECKBOX;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_f32_slider(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_F32_SLIDER) {
            result->type = WIDGET_TYPE_F32_SLIDER;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_option_selector(V2 where, string text, f32 scale, color32f32 modulation, s32* value) {
        auto result   = global_ui_state.widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_OPTION_SELECTOR) {
            result->type                   = WIDGET_TYPE_OPTION_SELECTOR;
            assertion(value && "we need a valid option value");
            result->current_selected_index = *value;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    void set_font_selected(font_cache* font) {
        global_ui_state.selected_font = font;
    }

    void set_font_active(font_cache* font) {
        global_ui_state.active_font = font;
    }

    void set_font(font_cache* font) {
        global_ui_state.default_font = font;
    }

    void label(V2 where, string text, color32f32 modulation, f32 scale, bool active) {
        assertion(global_ui_state.in_frame && "Need to call begin_frame first.");
        auto widget = push_label(where, text, scale, modulation);
        auto font = global_ui_state.default_font;

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);
    }

    s32 button(V2 where, string text, color32f32 modulation, f32 scale, bool active) {
        assertion(global_ui_state.in_frame && "Need to call begin_frame first.");
        auto widget         = push_button(where, text, scale, modulation);
        auto font           = global_ui_state.default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font) * widget->scale;
        auto button_rect    = rectangle_f32(widget->where.x, widget->where.y, text_width, text_height);

        V2   mouse_position = Input::mouse_location();
        bool clicked        = false;

        s32 status = WIDGET_ACTION_NONE;
        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || rectangle_f32_intersect(button_rect, rectangle_f32(mouse_position.x, mouse_position.y, 5, 5)))) {
            font = global_ui_state.active_font;
            if (Input::mouse_left()) {
                font = global_ui_state.selected_font;
                register_hot_widget(widget);
            }
            
            if (Input::pressed_mouse_left()) {
                status = WIDGET_ACTION_ACTIVATE;
            } else {
                status = WIDGET_ACTION_HOT;
            }
        }

#ifndef RELEASE
        render_commands_push_quad(global_ui_state.commands, button_rect, color32u8(255, 0, 0, 64), BLEND_MODE_ALPHA);
#endif
        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        return status;
    }

    bool checkbox(V2 where, string text, color32f32 modulation, f32 scale, bool* ptr, bool active) {
        assertion(global_ui_state.in_frame && "Need to call begin_frame first.");
        auto widget         = push_button(where, text, scale, modulation);
        auto font           = global_ui_state.default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font) * widget->scale;
        auto button_rect    = rectangle_f32(widget->where.x + text_width * 1.15f, widget->where.y, text_height, text_height);

        V2   mouse_position = Input::mouse_location();
        bool clicked        = false;

        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || rectangle_f32_intersect(button_rect, rectangle_f32(mouse_position.x, mouse_position.y, 5, 5)))) {
            font = global_ui_state.active_font;
            if (Input::mouse_left()) {
                font = global_ui_state.selected_font;
                register_hot_widget(widget);
            }
            
            if (Input::pressed_mouse_left()) {
                *ptr ^= 1;
            } else {
            }
        }

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        render_commands_push_quad(global_ui_state.commands, button_rect, color32u8(255, 255, 255, widget->modulation.a * 255), BLEND_MODE_ALPHA);
        {
            // inner "value".
            auto inner_rect = button_rect;
            inner_rect.w *= 0.9;
            inner_rect.h *= 0.9;

            auto dx = button_rect.w - inner_rect.w;
            auto dy = button_rect.h - inner_rect.h;

            inner_rect.x += (dx / 2);
            inner_rect.y += (dy / 2);


            color32u8 color = color32u8(255 * !(*ptr), 255 * (*ptr), 0, 255 - 128 * !active);
            render_commands_push_quad(global_ui_state.commands, inner_rect, color, BLEND_MODE_ALPHA);
        }

        return *ptr;
    }

    void option_selector(V2 where, string text, color32f32 modulation, f32 scale, string* options, s32 options_count, s32* out_selected, bool active) {
        assertion(global_ui_state.in_frame && "Need to call begin_frame first.");
        auto widget = push_option_selector(where, text, scale, modulation, out_selected);
        auto font   = global_ui_state.default_font;

        label(where, text, modulation, scale, active);
        where.x += font_cache_text_width(font, widget->text, widget->scale);
        label(where, options[widget->current_selected_index], modulation, scale, active);
        where.x += font_cache_text_width(font, longest_string_in_list(options, options_count), widget->scale) * 1.25;

        // no wrapping behavior
        if (button(where, string_literal("[+]"), modulation, scale, active) == WIDGET_ACTION_ACTIVATE) {
            widget->current_selected_index += 1;
            if (widget->current_selected_index >= options_count) {
                widget->current_selected_index = options_count - 1;
            }
        }
        where.x += font_cache_text_width(font, string_literal("[+]"), widget->scale)*1.2;
        if (button(where, string_literal("[-]"), modulation, scale, active) == WIDGET_ACTION_ACTIVATE) {
            widget->current_selected_index -= 1;
            if (widget->current_selected_index < 0) {
                widget->current_selected_index = 0;
            }
        }
        where.x += font_cache_text_width(font, string_literal("[-]"), widget->scale);

        // thankfully this is just a label and some buttons.
        *out_selected = widget->current_selected_index;
    }

    void f32_slider(V2 where, string text, color32f32 modulation, f32 scale, f32* ptr, f32 min_value, f32 max_value, f32 slider_width_px, bool active) {
        assertion(global_ui_state.in_frame && "Need to call begin_frame first.");
        auto widget         = push_f32_slider(where, text, scale, modulation);

        auto font           = global_ui_state.default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font) * widget->scale;

        auto bar_rect       = rectangle_f32(widget->where.x + text_width * 1.15f, widget->where.y, slider_width_px, text_height);

        const f32 range = (max_value - min_value);

        V2 mouse_position = Input::mouse_location();
        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || rectangle_f32_intersect(bar_rect, rectangle_f32(mouse_position.x, mouse_position.y, 5, 5)))) {
            font = global_ui_state.active_font;
            if (Input::mouse_left()) {
                register_hot_widget(widget);
                font = global_ui_state.selected_font;

                // drag the slider around...
                f32 new_percentage = clamp<f32>((mouse_position.x - bar_rect.x) / slider_width_px, 0.0f, 1.0f);
                *ptr               = (new_percentage * range) + min_value;
            }
        }

        f32 percentage_of = ((*ptr) - min_value) / (range);

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state.commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        render_commands_push_quad(global_ui_state.commands, bar_rect, color32u8(255, 255, 255, widget->modulation.a * 255), BLEND_MODE_ALPHA);
        {
            // inner "value".
            auto inner_rect = bar_rect;
            inner_rect.w *= 0.95;
            inner_rect.h *= 0.85;

            auto dx = bar_rect.w - inner_rect.w;
            auto dy = bar_rect.h - inner_rect.h;

            inner_rect.x += (dx / 2);
            inner_rect.y += (dy / 2);


            render_commands_push_quad(global_ui_state.commands, inner_rect, color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);

            color32u8 color = color32u8(255 * !(*ptr), 255 * (*ptr), 0, 255 - 128 * !active);
            inner_rect.w *= percentage_of;
            render_commands_push_quad(global_ui_state.commands, inner_rect, color, BLEND_MODE_ALPHA);
        }
    }

    void initialize(Memory_Arena* arena) {
        global_ui_state.widgets = Fixed_Array<Widget>(arena, 256);
    }

    void begin_frame(struct render_commands* commands) {
        global_ui_state.widgets.clear();
        global_ui_state.in_frame = true;
        global_ui_state.commands = commands;
        global_ui_state.ate_any_mouse_lefts = false;
    }

    void end_frame() {
        global_ui_state.in_frame = false;
        global_ui_state.commands = nullptr;

        if (!global_ui_state.ate_any_mouse_lefts) {
            global_ui_state.hot_index = -1;
        }
    }

    void update(f32 dt) {
        for (s32 i = 0; i < (s32)global_ui_state.widgets.size; ++i) {
            auto& widget = global_ui_state.widgets[i];
            // Nothing to do right now.
        }
    }
}
