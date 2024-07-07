#include "fade_transition.h"

struct transition_fader_state {
    u32                      type;
    color32f32               color;
    f32                      max_time;
    f32                      delay_time;
    f32                      time;
    bool                     forwards;
    on_delay_finish_callback on_delay_finish;
    on_finish_callback       on_finish;
    on_start_callback        on_start;
};

local struct transition_fader_state global_transition_fader_state      = {};
local struct transition_fader_state last_global_transition_fader_state = {};

namespace Transitions {
    bool fading() {
        if (global_transition_fader_state.type != TRANSITION_FADER_TYPE_NONE) {
            if (global_transition_fader_state.delay_time <= 0) {
                if (global_transition_fader_state.time >= global_transition_fader_state.max_time) {
                    return false;
                }

                return true;
            } else {
                return true;
            }
        }

        return false;
    }

    bool faded_in() {
        if (last_global_transition_fader_state.forwards) {
            return true;
        }

        return false;
    }

    void stop() {
        zero_memory(&global_transition_fader_state, sizeof(global_transition_fader_state));
    }

    void register_on_delay_finish(on_delay_finish_callback callback) {
        global_transition_fader_state.on_delay_finish = callback;
    }

    void register_on_finish(on_finish_callback callback) {
        global_transition_fader_state.on_finish = callback;
    }

    void register_on_start(on_start_callback callback) {
        global_transition_fader_state.on_start = callback;
    }

    local void fader_generic_setup(s32 type, color32f32 color, f32 delay_timer, f32 time, bool forwards) {
        struct transition_fader_state* state = &global_transition_fader_state;
        state->type                          = type;
        state->color                         = color;
        state->max_time                      = time;
        state->time                          = 0;
        state->delay_time                    = delay_timer;
        state->forwards                      = forwards;
        register_on_delay_finish([](void*){});
        register_on_finish([](void*){});
        register_on_start([](void*){});
    }

    void do_horizontal_slide_in(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_HORIZONTAL_SLIDE,
            target_color,
            delay_timer,
            time,
            true
        );
        _debugprintf("Starting a horizontal slide fade in!");
    }

    void do_horizontal_slide_out(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_HORIZONTAL_SLIDE,
            target_color,
            delay_timer,
            time,
            false
        );
        _debugprintf("Starting a horizontal slide fade out!");
    }

    void do_vertical_slide_in(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_VERTICAL_SLIDE,
            target_color,
            delay_timer,
            time,
            true
        );
        _debugprintf("Starting a vertical slide fade in!");
    }

    void do_vertical_slide_out(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_VERTICAL_SLIDE,
            target_color,
            delay_timer,
            time,
            false
        );
        _debugprintf("Starting a vertical slide fade out!");
    }

    void do_curtainclose_in(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_CURTAINCLOSE_SLIDE,
            target_color,
            delay_timer,
            time,
            true
        );
        _debugprintf("Starting a curtain close slide fade in!");
    }

    void do_curtainclose_out(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_CURTAINCLOSE_SLIDE,
            target_color,
            delay_timer,
            time,
            false
        );
        _debugprintf("Starting a curtain close slide fade out!");
    }

    void do_shuteye_in(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_SHUTEYE_SLIDE,
            target_color,
            delay_timer,
            time,
            true
        );
        _debugprintf("Starting a shuteye slide fade in!");
    }

    void do_shuteye_out(color32f32 target_color, f32 delay_timer, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_SHUTEYE_SLIDE,
            target_color,
            delay_timer,
            time,
            false
        );
        _debugprintf("Starting a shuteye slide fade out!");
    }

    void do_color_transition_in(color32f32 target_color, f32 delay_time, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_COLOR,
            target_color,
            delay_time,
            time,
            true
        );

        _debugprintf("Starting a color fade in!");
    }

    void do_color_transition_out(color32f32 target_color, f32 delay_time, f32 time) {
        struct transition_fader_state* state = &global_transition_fader_state;
        fader_generic_setup(
            TRANSITION_FADER_TYPE_COLOR,
            target_color,
            delay_time,
            time,
            false
        );

        _debugprintf("Starting a color fade out!");
    }

    local void update_and_render_color_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t);
    local void update_and_render_horizontal_slide_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t);
    local void update_and_render_vertical_slide_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t);
    local void update_and_render_curtainclose_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t);
    local void update_and_render_shuteye_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t);

    f32 fade_t() {
        struct transition_fader_state* transition_state = &global_transition_fader_state;
        f32 effective_t = clamp<f32>((transition_state->time / transition_state->max_time), 0, 1);
        return effective_t;
    }

    void clear_effect() {
        struct transition_fader_state* transition_state = &global_transition_fader_state;
        auto                           on_delay_finish  = transition_state->on_delay_finish;
        auto                           on_start         = transition_state->on_start;
        auto                           on_finish        = transition_state->on_finish;

        transition_state->type = TRANSITION_FADER_TYPE_NONE;
        transition_state->time = 0;

        // NOTE: can be reentrant.
#if 0
        // I do not remember why I did this, but the code that uses clear_effect()
        // has no need to do any "emergency" behavior so it's fine.
        if (on_delay_finish) 
            on_delay_finish(nullptr);
        if (on_finish)
            on_finish(nullptr);

        last_global_transition_fader_state = global_transition_fader_state;
        stop();

        register_on_delay_finish([](void*){});
        register_on_finish([](void*){});
        register_on_start([](void*){});
#else
        stop();
        register_on_delay_finish([](void*){});
        register_on_finish([](void*){});
        register_on_start([](void*){});
#endif
    }

    // can be touched for other stuff later I guess.
    // I guess you can use inheritance for this but there's no reason imo.
    void update_and_render(struct render_commands* commands, f32 dt) {
        struct transition_fader_state* transition_state = &global_transition_fader_state;
        auto                           on_delay_finish  = transition_state->on_delay_finish;
        auto                           on_start         = transition_state->on_start;
        auto                           on_finish        = transition_state->on_finish;

        if (transition_state->type == TRANSITION_FADER_TYPE_NONE) {
            return;
        }

        if (transition_state->time == 0.0f) {
            transition_state->on_start(nullptr);
        }

        f32 effective_t = clamp<f32>((transition_state->time / transition_state->max_time), 0, 1);

        switch (transition_state->type) {
            case TRANSITION_FADER_TYPE_COLOR: {
                update_and_render_color_fades(transition_state, commands, effective_t);
            } break;
            case TRANSITION_FADER_TYPE_HORIZONTAL_SLIDE: {
                update_and_render_horizontal_slide_fades(transition_state, commands, effective_t);
            } break;
            case TRANSITION_FADER_TYPE_VERTICAL_SLIDE: {
                update_and_render_vertical_slide_fades(transition_state, commands, effective_t);
            } break;
            case TRANSITION_FADER_TYPE_SHUTEYE_SLIDE: {
                update_and_render_shuteye_fades(transition_state, commands, effective_t);
            } break;
            case TRANSITION_FADER_TYPE_CURTAINCLOSE_SLIDE: {
                update_and_render_curtainclose_fades(transition_state, commands, effective_t);
            } break;

                bad_case;
        }

        if (transition_state->delay_time > 0) {
            transition_state->delay_time -= dt;

            if (transition_state->delay_time <= 0) {
                on_delay_finish(nullptr);
            }
        } else {
            if (transition_state->time < transition_state->max_time) {
                transition_state->time += dt;

                if (transition_state->time >= transition_state->max_time) {
                    on_finish(nullptr);
                    last_global_transition_fader_state = global_transition_fader_state;
                }
            } else {
                /* ? */
            }
        }
    }

    local void update_and_render_color_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t) {
        if (fader_state->forwards) {
            effective_t = 1 - effective_t;
        }

        union color32f32 render_color = fader_state->color;
        render_color.a                = lerp_f32(fader_state->color.a, 0, effective_t);

        render_commands_push_quad(commands, rectangle_f32(0, 0, commands->screen_width, commands->screen_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
    }

    local void update_and_render_horizontal_slide_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t) {
        if (!fader_state->forwards) {
            effective_t = 1 - effective_t;
        }

        union color32f32 render_color = fader_state->color;
        f32 position_to_draw          = lerp_f32(-(s32)commands->screen_width, 0, effective_t);

        render_commands_push_quad(commands,
                                  rectangle_f32(position_to_draw, 0,
                                                commands->screen_width, commands->screen_height),
                                  color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
    }

    local void update_and_render_vertical_slide_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t) {
        if (!fader_state->forwards) {
            effective_t = 1 - effective_t;
        }

        union color32f32 render_color = fader_state->color;
        f32 position_to_draw          = lerp_f32(-(s32)commands->screen_height, 0, effective_t);

        render_commands_push_quad(commands, rectangle_f32(0, position_to_draw, commands->screen_width, commands->screen_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
    }

    local void update_and_render_curtainclose_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t) {
        if (!fader_state->forwards) {
            effective_t = 1 - effective_t;
        }

        union color32f32 render_color = fader_state->color;
        s32              half_width  = commands->screen_width/2;

        f32 lid_positions[] = {
            lerp_f32(-(s32)commands->screen_width,                     0, effective_t),
            lerp_f32((s32)commands->screen_width+half_width,  half_width, effective_t),
        };

        render_commands_push_quad(commands, rectangle_f32(lid_positions[0], 0, half_width, commands->screen_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
        render_commands_push_quad(commands, rectangle_f32(lid_positions[1], 0, half_width, commands->screen_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
    }

    local void update_and_render_shuteye_fades(struct transition_fader_state* fader_state, struct render_commands* commands, f32 effective_t) {
        if (!fader_state->forwards) {
            effective_t = 1 - effective_t;
        }

        union color32f32 render_color = fader_state->color;
        s32              half_height  = commands->screen_height/2;

        f32 lid_positions[] = {
            lerp_f32(-(s32)commands->screen_height,                        0, effective_t),
            lerp_f32((s32)commands->screen_height+half_height,   half_height, effective_t),
        };

        render_commands_push_quad(commands, rectangle_f32(0, lid_positions[0], commands->screen_width, half_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
        render_commands_push_quad(commands, rectangle_f32(0, lid_positions[1], commands->screen_width, half_height), color32f32_to_color32u8(render_color), BLEND_MODE_ALPHA);
    }
}
