#ifndef ACTION_MAPPER_H
#define ACTION_MAPPER_H

#include "input.h"
/*
  Unlike the one I used in Legends, this one will actually have joy stick support.

  The game doesn't have need of the triggers currently, so those are not implemented.

  NOTE: I haven't really needed an input layering system yet, so it also doesn't
  exist, but I might consider one in the future I suppose.
*/

enum Action_ID {
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_ACTION,
    ACTION_FOCUS,
    ACTION_CANCEL,
    ACTION_MENU,
    ACTION_COUNT,
};

namespace Action {
    // TODO: add with_repeat
    void register_action_keys(s32 action_id, s32 key_id, s32 key_id2=KEY_UNKNOWN, f32 analog_value=1.0);
    void register_action_button(s32 action_id, s32 button_id, f32 analog_value=1.0);
    void register_action_joystick_axis(s32 action_id, s32 which, s32 axis_id);

    // for analog values like on a joystick.
    f32  value(s32 action_id);

    bool is_down(s32 action_id);
    bool is_pressed(s32 action_id); // might be a bit harder to "judge" with joysticks...
};

#endif
