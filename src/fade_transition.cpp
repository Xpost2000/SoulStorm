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
    bool fading();
    bool faded_in();
    void stop();

    void register_on_delay_finish(on_delay_finish_callback callback);
    void register_on_finish(on_finish_callback callback);
    void register_on_start(on_start_callback callback);

    void do_horizontal_slide_in(color32f32 target_color, f32 delay_timer, f32 time);
    void do_horizontal_slide_out(color32f32 target_color, f32 delay_timer, f32 time);

    void do_vertical_slide_in(color32f32 target_color, f32 delay_timer, f32 time);
    void do_vertical_slide_out(color32f32 target_color, f32 delay_timer, f32 time);

    void do_curtainclose_in(color32f32 target_color, f32 delay_timer, f32 time);
    void do_curtainclose_out(color32f32 target_color, f32 delay_timer, f32 time);

    void do_shuteye_in(color32f32 target_color, f32 delay_timer, f32 time);
    void do_shuteye_out(color32f32 target_color, f32 delay_timer, f32 time);

    void do_color_transition_in(color32f32 target_color, f32 delay_time, f32 time);
    void do_color_transition_out(color32f32 target_color, f32 delay_time, f32 time);

    void update_and_render(struct render_commands* commands, f32 dt);
}
