#ifndef ACTION_MAPPER_H
#define ACTION_MAPPER_H

#include "input.h"
/*
  Unlike the one I used in Legends, this one will actually have joy stick support.

  The game doesn't have need of the triggers currently, so those are not implemented.

  NOTE: I haven't really needed an input layering system yet, so it also doesn't
  exist, but I might consider one in the future I suppose.

  NOTE: keys do not have modifiers
*/
struct Action_Data {
    s32 key_id[2] = {0, 0};
    s32 button_id = 0;
    s32 joystick_id = -1;
    s32 axis_id = -1;

    // NOTE: all keys will have analog
    //       value as part of their binding so
    //       that way utilizing the Action mapping
    //       in situations which benefit from "analog" input
    //       will still use the same code.
    f32 analog_value = 0.0f;
};

enum Action_ID {
    ACTION_MOVE_UP,
    ACTION_MOVE_DOWN,
    ACTION_MOVE_LEFT,
    ACTION_MOVE_RIGHT,
    ACTION_ACTION,
    ACTION_FOCUS,
    ACTION_CANCEL,
    ACTION_MENU,
    ACTION_SCREENSHOT,
    ACTION_USE_BOMB,
    ACTION_COUNT,
};

local const char* action_id_string_readable_name(s32 action_id) {
    switch (action_id) {
        case ACTION_MOVE_UP:
            return "Move Up";
        case ACTION_MOVE_DOWN:
            return "Move Down";
        case ACTION_MOVE_LEFT:
            return "Move Left";
        case ACTION_MOVE_RIGHT:
            return "Move Right";
        case ACTION_ACTION:
            return "Action / Fire";
        case ACTION_FOCUS:
            return "Focus";
        case ACTION_CANCEL:
            return "Cancel";
        case ACTION_MENU:
            return "Menu / Pause";
        case ACTION_SCREENSHOT:
            return "Screenshot";
        case ACTION_USE_BOMB: 
            return "Use Bomb";
    }

    return "?";
}

local const char* action_id_strings_lua(s32 action_id) {
    switch (action_id) {
        case ACTION_MOVE_UP:
            return "Actions.MOVE_UP";
        case ACTION_MOVE_DOWN:
            return "Actions.MOVE_DOWN";
        case ACTION_MOVE_LEFT:
            return "Actions.MOVE_LEFT";
        case ACTION_MOVE_RIGHT:
            return "Actions.MOVE_RIGHT";
        case ACTION_ACTION:
            return "Actions.ACTION";
        case ACTION_FOCUS:
            return "Actions.FOCUS";
        case ACTION_CANCEL:
            return "Actions.CANCEL";
        case ACTION_MENU:
            return "Actions.MENU";
        case ACTION_SCREENSHOT:
            return "Actions.SCREENSHOT";
        case ACTION_USE_BOMB: 
            return "Actions.USE_BOMB";
    }

    return "?";
}

struct lua_State;
namespace Action {
    // TODO: add with_repeat
    void register_action_keys(s32 action_id, s32 key_id, s32 key_id2=KEY_UNKNOWN, f32 analog_value=1.0);
    void register_action_button(s32 action_id, s32 button_id, f32 analog_value=1.0);
    void register_action_joystick_axis(s32 action_id, s32 which, s32 axis_id);

    // for analog values like on a joystick.
    f32  value(s32 action_id);

    bool is_down(s32 action_id);
    bool is_pressed(s32 action_id); // might be a bit harder to "judge" with joysticks...

    int luaL_open_game_actionlib(lua_State* L);

    bool save(string filename);
    bool load(string filename);

    void copy_action_map(Action_Data* source, Action_Data* dest);

    Action_Data* get_action_map(void);
    Action_Data* get_action_data_for(s32 action_id);
};

#endif
