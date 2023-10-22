#ifndef FADE_TRANSITION_H
#define FADE_TRANSITION_H

#include <functional>
#include "common.h"
#include "graphics_common.h"

/*
  This is mostly ported from legends, and replacing some things such as the callback data with
  std::functions which can allocate their own memory...

  (granted I am fully aware that closures in C++ may use heap memory which is technically not something
  I want. However, it is *C++* and I don't think overloading new is that great of an idea either :P)

  NOTE:
  I don't have the crossfade system, from Legends because that requires a separate framebuffer to accumulate. I'm
  preferring to keep the engine's graphics subset that is trivial to maintain in multiple backends without much change...

  I do admit, I do like how some of the old C stuff got cleaned up when rewritten to be a lighter C++ dialect like this.
*/

enum transition_fader_type {
    TRANSITION_FADER_TYPE_NONE,
    TRANSITION_FADER_TYPE_COLOR,

    TRANSITION_FADER_TYPE_VERTICAL_SLIDE,
    TRANSITION_FADER_TYPE_HORIZONTAL_SLIDE,

    TRANSITION_FADER_TYPE_SHUTEYE_SLIDE,
    TRANSITION_FADER_TYPE_CURTAINCLOSE_SLIDE,

    TRANSITION_FADER_TYPE_COUNT,
};

/* NOTE:
   the void pointer is just here just in case I decide to reinstate the data as part of the transition,
   state.

   In practice it's probably going to be a nullptr for the remainder of the project.
*/
using on_delay_finish_callback = std::function<void(void*)>;
using on_finish_callback       = std::function<void(void*)>;
using on_start_callback        = std::function<void(void*)>;

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

#endif
