#include "action_mapper.h"

struct Action_Data {
    s32 key_id[2] = {-1, -1};
    s32 button_id = -1;
    s32 joystick_id = -1;
    s32 axis_id = -1;

    // NOTE: all keys will have analog
    //       value as part of their binding so
    //       that way utilizing the Action mapping
    //       in situations which benefit from "analog" input
    //       will still use the same code.
    f32 analog_value = 0.0f;
};

local Action_Data action_map[ACTION_COUNT];

namespace Action {
    void register_action_keys(s32 action_id, s32 key_id, s32 key_id2, f32 analog_value) {
        auto& action        = action_map[action_id];
        action.key_id[0]    = key_id;
        action.key_id[1]    = key_id2;
        action.analog_value = analog_value;
    }

    void register_action_button(s32 action_id, s32 button_id, f32 analog_value) {
        auto& action        = action_map[action_id];
        action.button_id    = button_id;
        action.analog_value = analog_value;
    }

    void register_action_joystick_axis(s32 action_id, s32 which, s32 axis_id) {
        auto& action = action_map[action_id];
        action.joystick_id = which;
        action.axis_id = axis_id;
    }

    f32 value(s32 action_id) {
        auto& action = action_map[action_id];
        auto gamepad = Input::get_game_controller(0);

        if (Input::is_key_down(action.key_id[0]) ||
            Input::is_key_down(action.key_id[1]) ||
            Input::controller_button_down(gamepad, action.button_id)) {
            return action.analog_value;
        }

        switch (action.joystick_id) {
            case CONTROLLER_JOYSTICK_LEFT: {
                f32 axis = Input::controller_left_axis(gamepad, action.axis_id);
                return axis;
            } break;
            case CONTROLLER_JOYSTICK_RIGHT: {
                f32 axis = Input::controller_right_axis(gamepad, action.axis_id);
                return axis;
            } break;
        }

        return 0;
    }

    bool is_down(s32 action_id) {
        auto& action = action_map[action_id];
        auto gamepad = Input::get_game_controller(0);

        // NOTE: not sure of good way to interpret axis here.
        return
            Input::is_key_down(action.key_id[0]) ||
            Input::is_key_down(action.key_id[1]) ||
            Input::controller_button_down(gamepad, action.button_id);
    }

    bool is_pressed(s32 action_id) {
        auto gamepad = Input::get_game_controller(0);
        auto& action = action_map[action_id];

        bool controller_response = Input::controller_button_pressed(gamepad, action.button_id);
        bool keyboard_response   =
            Input::is_key_pressed(action.key_id[0]) ||
            Input::is_key_pressed(action.key_id[1]);

        switch (action.joystick_id) {
            case CONTROLLER_JOYSTICK_LEFT: {
                controller_response |= Input::controller_left_axis_flicked(gamepad, action.axis_id);
            } break;
            case CONTROLLER_JOYSTICK_RIGHT: {
                controller_response |= Input::controller_right_axis_flicked(gamepad, action.axis_id);
            } break;
        }

        return controller_response || keyboard_response;
    }
};
