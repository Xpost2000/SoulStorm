#include "input.h"
#include "action_mapper.h"

#include "game_ui.h"
#include "fixed_array.h"
#include "engine.h"

/*
  NOTE:

  This UI wasn't really designed with keyboard in mind so whoops.
*/

enum Widget_Type {
    WIDGET_TYPE_UNKNOWN,
    WIDGET_TYPE_TEXT,
    WIDGET_TYPE_OPTION_SELECTOR,
    WIDGET_TYPE_BUTTON,
    WIDGET_TYPE_CHECKBOX,
    WIDGET_TYPE_F32_SLIDER,
};

// TODO: not accounting for inactives.
struct Widget {
    u32        type = 0;
    V2         where;
    color32f32 modulation;
    string     text;
    f32        scale;
    bool       active;

    // for option_selector
    s32 current_selected_index;

    s32 id = -1;
};

struct UI_State {
    font_cache* selected_font = nullptr;
    font_cache* default_font = nullptr;
    font_cache* active_font = nullptr;

    struct render_commands* commands = nullptr;

    Fixed_Array<Widget> widgets;
    bool in_frame;

    s32 hot_index = -1;
    bool ate_any_mouse_lefts = false;

    // Don't want a frame of latency to possibly
    // break things...
    bool picked_first_index = false;
    s32 selected_index = 0;

    char* last_ui_id = nullptr;
    char* ui_id      = nullptr;
};

UI_State* global_ui_state = nullptr;

/*
  NOTE: this is sort of like a stateful IMGUI, which is an oxymoron!
 */
namespace GameUI {
    local bool is_selectable_widget_type(s32 type) {
        switch (type) {
            case WIDGET_TYPE_UNKNOWN:
            case WIDGET_TYPE_TEXT:
            case WIDGET_TYPE_OPTION_SELECTOR:
                return false;
            case WIDGET_TYPE_BUTTON:
            case WIDGET_TYPE_CHECKBOX:
            case WIDGET_TYPE_F32_SLIDER:
                return true;
        }
        return false;
    }
    local void common_widget_initialization(Widget* widget, V2 where, string text, f32 scale, color32f32 modulation) {
        widget->id         = global_ui_state->widgets.size;
        widget->where      = where;
        widget->modulation = modulation;
        widget->text       = string_clone(&Global_Engine()->scratch_arena, text);
        widget->scale      = scale;

        if (!global_ui_state->picked_first_index && is_selectable_widget_type(widget->type)) {
            global_ui_state->selected_index     = widget->id-1;
            global_ui_state->picked_first_index = true;
            _debugprintf("I picked my first widget. (%d)", global_ui_state->selected_index);
        }
    }

    local bool is_selected_widget(Widget* widget) {
        if (widget->type == WIDGET_TYPE_OPTION_SELECTOR) {
            // NOTE: not working. Not a big deal but check later...
            return (
                widget->id   == global_ui_state->selected_index ||
                widget->id+1 == global_ui_state->selected_index ||
                widget->id-1 == global_ui_state->selected_index
            );
        }

        return widget->id-1 == global_ui_state->selected_index;
    }

    local bool is_selectable_on_active(Widget* widget) {
        return (global_ui_state->hot_index == -1 ||
                global_ui_state->hot_index == widget->id ||
                global_ui_state->selected_index == widget->id-1);
    }
    local bool is_currently_hot(Widget* widget) {
        return (global_ui_state->hot_index == widget->id || global_ui_state->selected_index == widget->id-1);
    }

    local void register_hot_widget(Widget* widget) {
        if (global_ui_state->hot_index == -1 || global_ui_state->hot_index == widget->id) {
            global_ui_state->hot_index           = widget->id;
            global_ui_state->ate_any_mouse_lefts = true;
        }
    }

    local Widget* push_label(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state->widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_TEXT) {
            result->type       = WIDGET_TYPE_TEXT;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_button(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state->widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_BUTTON) {
            result->type       = WIDGET_TYPE_BUTTON;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_checkbox(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state->widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_CHECKBOX) {
            result->type = WIDGET_TYPE_CHECKBOX;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_f32_slider(V2 where, string text, f32 scale, color32f32 modulation) {
        auto result   = global_ui_state->widgets.alloc();

        // specific initialization behavior if this was a different
        // widget last frame.
        if (result->type != WIDGET_TYPE_F32_SLIDER) {
            result->type = WIDGET_TYPE_F32_SLIDER;
        }

        common_widget_initialization(result, where, text, scale, modulation);
        return result;
    }

    local Widget* push_option_selector(V2 where, string text, f32 scale, color32f32 modulation, s32* value) {
        auto result   = global_ui_state->widgets.alloc();

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
        global_ui_state->selected_font = font;
    }

    void set_font_active(font_cache* font) {
        global_ui_state->active_font = font;
    }

    void set_font(font_cache* font) {
        global_ui_state->default_font = font;
    }

    void label(V2 where, string text, color32f32 modulation, f32 scale, bool active) {
        assertion(global_ui_state->in_frame && "Need to call begin_frame first.");
        auto widget = push_label(where, text, scale, modulation);
        widget->active = active;
        auto font = global_ui_state->default_font;

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state->commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);
    }

    s32 button(V2 where, string text, color32f32 modulation, f32 scale, bool active) {
        assertion(global_ui_state->in_frame && "Need to call begin_frame first.");
        auto widget      = push_button(where, text, scale, modulation);
        widget->active    = active;
        auto font        = global_ui_state->default_font;
        auto text_width  = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height = font_cache_text_height(font) * widget->scale;
        auto button_rect = rectangle_f32(widget->where.x, widget->where.y, text_width, text_height);

        bool clicked        = false;

        auto mouse_rect = Input::mouse_rectangle();
        bool intersecting = rectangle_f32_intersect(button_rect, mouse_rect);

        s32 status = WIDGET_ACTION_NONE;
        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || intersecting)) {
            font = global_ui_state->active_font;
            if (Input::mouse_left() && intersecting) {
                font = global_ui_state->selected_font;
                register_hot_widget(widget);
            }
            
            if ((Input::pressed_mouse_left() && intersecting)
                || (Action::is_pressed(ACTION_ACTION) && is_selected_widget(widget))) {
                status = WIDGET_ACTION_ACTIVATE;
            } else {
                status = WIDGET_ACTION_HOT;
            }
        }

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state->commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        return status;
    }

    bool checkbox(V2 where, string text, color32f32 modulation, f32 scale, bool* ptr, bool active) {
        assertion(global_ui_state->in_frame && "Need to call begin_frame first.");
        auto widget         = push_button(where, text, scale, modulation);
        widget->active    = active;
        auto font           = global_ui_state->default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font) * widget->scale;
        auto button_rect    = rectangle_f32(widget->where.x + text_width * 1.15f, widget->where.y, text_height, text_height);

        auto mouse_rect = Input::mouse_rectangle();
        bool intersecting = rectangle_f32_intersect(button_rect, mouse_rect);
        bool clicked        = false;

        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || intersecting)) {
            font = global_ui_state->active_font;
            if (Input::mouse_left() && intersecting) {
                font = global_ui_state->selected_font;
                register_hot_widget(widget);
            }
            
            if ((Input::pressed_mouse_left() && intersecting) || (Action::is_pressed(ACTION_ACTION) && is_selected_widget(widget))) {
                *ptr ^= 1;
            } else {
            }
        }

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state->commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        render_commands_push_quad(global_ui_state->commands, button_rect, color32u8(255, 255, 255, widget->modulation.a * 255), BLEND_MODE_ALPHA);
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
            render_commands_push_quad(global_ui_state->commands, inner_rect, color, BLEND_MODE_ALPHA);
        }

        return *ptr;
    }

    void option_selector(V2 where, string text, color32f32 modulation, f32 scale, string* options, s32 options_count, s32* out_selected, bool active) {
        assertion(global_ui_state->in_frame && "Need to call begin_frame first.");
        auto widget = push_option_selector(where, text, scale, modulation, out_selected);
        widget->active    = active;
        auto font   = global_ui_state->default_font;

        label(where, text, modulation, scale, active);
        where.x += font_cache_text_width(font, widget->text, widget->scale);
        label(where, options[widget->current_selected_index], modulation, scale, active);
        where.x += font_cache_text_width(font, longest_string_in_list(options, options_count), widget->scale) * 1.25;

        // no wrapping behavior
        if (button(where, string_literal("[<<]"), modulation, scale, active) == WIDGET_ACTION_ACTIVATE ||
            is_selected_widget(widget) && Action::is_pressed(ACTION_MOVE_LEFT)) {
            widget->current_selected_index -= 1;
            if (widget->current_selected_index < 0) {
                widget->current_selected_index = 0;
            }
        }
        where.x += font_cache_text_width(font, string_literal("[<<]"), widget->scale);
        if (button(where, string_literal("[>>]"), modulation, scale, active) == WIDGET_ACTION_ACTIVATE ||
            is_selected_widget(widget) && Action::is_pressed(ACTION_MOVE_RIGHT)) {
            widget->current_selected_index += 1;
            if (widget->current_selected_index >= options_count) {
                widget->current_selected_index = options_count - 1;
            }
        }
        where.x += font_cache_text_width(font, string_literal("[>>]"), widget->scale)*1.2;

        // thankfully this is just a label and some buttons.
        *out_selected = widget->current_selected_index;
    }

    void f32_slider(V2 where, string text, color32f32 modulation, f32 scale, f32* ptr, f32 min_value, f32 max_value, f32 slider_width_px, bool active) {
        assertion(global_ui_state->in_frame && "Need to call begin_frame first.");
        auto widget         = push_f32_slider(where, text, scale, modulation);
        widget->active    = active;
        auto font           = global_ui_state->default_font;
        auto text_width     = font_cache_text_width(font, widget->text, widget->scale);
        auto text_height    = font_cache_text_height(font) * widget->scale;

        auto bar_rect       = rectangle_f32(widget->where.x + text_width * 1.15f, widget->where.y, slider_width_px, text_height);

        const f32 range = (max_value - min_value);

        V2 mouse_position = Input::mouse_location();

        auto mouse_rect = Input::mouse_rectangle();
        bool intersecting = rectangle_f32_intersect(bar_rect, mouse_rect);

        if (is_selectable_on_active(widget) && active && (is_currently_hot(widget) || intersecting)) {
            font = global_ui_state->active_font;

            if (Input::mouse_left() && intersecting) {
                register_hot_widget(widget);
                font = global_ui_state->selected_font;

                // drag the slider around...
                f32 new_percentage = clamp<f32>((mouse_position.x - bar_rect.x) / slider_width_px, 0.0f, 1.0f);
                *ptr               = (new_percentage * range) + min_value;
            }

            if (is_selected_widget(widget)) {
                // usually this doesn't hurt anyone...
                if (Action::is_pressed(ACTION_MOVE_RIGHT)) {
                    *ptr += range/4.0f;
                } else if (Action::is_pressed(ACTION_MOVE_LEFT)) {
                    *ptr -= range/4.0f;
                }

                if (*ptr < min_value) *ptr = min_value;
                if (*ptr > max_value) *ptr = max_value;
            }
        }

        f32 percentage_of = ((*ptr) - min_value) / (range);

        if (!active) widget->modulation.a = 0.5 * modulation.a;
        else         widget->modulation.a = 1.0 * modulation.a;
        render_commands_push_text(global_ui_state->commands, font, widget->scale, widget->where, widget->text, widget->modulation, BLEND_MODE_ALPHA);

        render_commands_push_quad(global_ui_state->commands, bar_rect, color32u8(255, 255, 255, widget->modulation.a * 255), BLEND_MODE_ALPHA);
        {
            // inner "value".
            auto inner_rect = bar_rect;
            inner_rect.w *= 0.95;
            inner_rect.h *= 0.85;

            auto dx = bar_rect.w - inner_rect.w;
            auto dy = bar_rect.h - inner_rect.h;

            inner_rect.x += (dx / 2);
            inner_rect.y += (dy / 2);


            render_commands_push_quad(global_ui_state->commands, inner_rect, color32u8(0, 0, 0, 255), BLEND_MODE_ALPHA);

            color32u8 color = color32u8(255 * !(*ptr), 255 * (*ptr), 0, 255 - 128 * !active);
            inner_rect.w *= percentage_of;
            render_commands_push_quad(global_ui_state->commands, inner_rect, color, BLEND_MODE_ALPHA);
        }
    }

    void initialize(Memory_Arena* arena) {
        global_ui_state         = (UI_State*)arena->push_unaligned(sizeof(*global_ui_state));
        global_ui_state->widgets = Fixed_Array<Widget>(arena, 256);
    }

    void begin_frame(struct render_commands* commands) {
        global_ui_state->widgets.clear();
        global_ui_state->in_frame = true;
        global_ui_state->commands = commands;
        global_ui_state->ate_any_mouse_lefts = false;
    }

    void end_frame() {
        if (Action::is_pressed(ACTION_MOVE_DOWN)) {
            // _debugprintf("Move down!");
            GameUI::move_selected_widget_id(1);
        }

        if (Action::is_pressed(ACTION_MOVE_UP)) {
            // _debugprintf("Move up!");
            GameUI::move_selected_widget_id(-1);
        }

#ifndef RELEASE
        auto font           = global_ui_state->default_font;
        render_commands_push_text(global_ui_state->commands, font, 1.0, V2(0,0),
                                  string_from_cstring(format_temp("(lastui_id: %s)\nselected_index: %d\n", global_ui_state->last_ui_id, global_ui_state->selected_index)), color32f32(1,1,1,1), BLEND_MODE_ALPHA);
#endif

        global_ui_state->in_frame = false;

        if (!global_ui_state->ate_any_mouse_lefts) {
            global_ui_state->hot_index = -1;
        }
    }

    void update(f32 dt) {
        for (s32 i = 0; i < (s32)global_ui_state->widgets.size; ++i) {
            auto& widget = global_ui_state->widgets[i];
            // Nothing to do right now.
        }
    }

    void set_ui_id(char* id_string) {
        if (global_ui_state->last_ui_id != id_string) {
            global_ui_state->widgets.zero();
            global_ui_state->picked_first_index = false;
            global_ui_state->selected_index     = -1;

            _debugprintf("This is a different UI id.");
            global_ui_state->last_ui_id = global_ui_state->ui_id;
            global_ui_state->ui_id = id_string;
        }
    }

    void move_selected_widget_id(s32 increments) {
        _debugprintf("Moving selected widget id by (%d is the current id) %d", global_ui_state->selected_index, increments);
        // NOTE: I want to skip the [+][-] buttons in the future
        global_ui_state->selected_index += increments;

        if (global_ui_state->selected_index < 0) {
            global_ui_state->selected_index = global_ui_state->widgets.size-1;
        } else if (global_ui_state->selected_index >= global_ui_state->widgets.size) {
            global_ui_state->selected_index = 0;
        }

        auto& next_widget = global_ui_state->widgets[global_ui_state->selected_index];
        _debugprintf("Next widget is id(%d)", global_ui_state->selected_index);
        if (!is_selectable_widget_type(next_widget.type) && next_widget.active) {
            _debugprintf("I do not think (%d) is selectable.", global_ui_state->selected_index);
            move_selected_widget_id(sign_s32(increments));
        }
    }
}
