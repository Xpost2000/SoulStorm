extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "action_mapper.h"

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

local Action_Data action_map[ACTION_COUNT] = {};
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

        if (action.axis_id != -1) {
            switch (action.joystick_id) {
                case CONTROLLER_JOYSTICK_LEFT: {
                    controller_response |= Input::controller_left_axis_flicked(gamepad, action.axis_id);
                } break;
                case CONTROLLER_JOYSTICK_RIGHT: {
                    controller_response |= Input::controller_right_axis_flicked(gamepad, action.axis_id);
                } break;
            }
        }

        return controller_response || keyboard_response;
    }

    int luaL_open_game_actionlib(lua_State* L) {
        const char* action_enums = R"(
Actions = {
MOVE_UP = 0,
MOVE_DOWN = 1,
MOVE_LEFT = 2,
MOVE_RIGHT = 3,
ACTION = 4,
FOCUS = 5,
CANCEL = 6,
MENU = 7,
SCREENSHOT= 8
}
)";
        luaL_dostring(L, action_enums);

        /* for now the API will mirror the C one (which admittedly isn't perfect but whatever.) */
        lua_register(L, "register_action_keys",
                     [](lua_State* L) {
                         int   action_id    = luaL_checkinteger(L, 1);
                         int   key_id       = luaL_checkinteger(L, 2);
                         int   key_id2      = luaL_checkinteger(L, 3);
                         float analog_value = luaL_checknumber(L, 4);

                         register_action_keys(action_id, key_id, key_id2, analog_value);
                         return 1;
                     }
        );
        lua_register(L, "register_action_button",
                     [](lua_State* L) {
                         int   action_id    = luaL_checkinteger(L, 1);
                         int   button_id    = luaL_checkinteger(L, 2);
                         float analog_value = luaL_checknumber(L, 3);

                         register_action_button(action_id, button_id, analog_value);
                         return 1;
                     }
        );
        lua_register(L, "register_action_joystick_axis",
                     [](lua_State* L) {
                         int action_id = luaL_checkinteger(L, 1);
                         int which     = luaL_checkinteger(L, 2);
                         int axis_id   = luaL_checkinteger(L, 3);

                         register_action_joystick_axis(action_id, which, axis_id);
                         return 1;
                     }
        );
        // NOTE: the intended use of lua is to author level choreographies and configuration
        // so I only need this much.
        // lua_register(L, "value");
        // lua_register(L, "is_down");
        // lua_register(L, "is_pressed");
        return 1;
    }

    bool save(string filename) {
        // NOTE:
        // So this is not very user friendly, but that can change pretty quickly.
        // Unfortunately because I don't have access to designated initializers
        // there is no real convenient way to map files to strings without the standard library.

        FILE* f = fopen(filename.data, "wb+");
        if (!f) {
            _debugprintf("Could not save action preferences?");
            return false;
        }
        {
            fprintf(f, "---- Action preferences. Currently not user-friendly. Check engine code action_mapper.h & input.h for key ids.\n");
            for (int i = 0; i < ACTION_COUNT; ++i) {
                auto action = action_map[i];

                /* I need a way to reverse all the amppings. */
                if (!(action.key_id[0] == -1 && action.key_id[1] == -1)) {
                    fprintf(f,  "register_action_keys(%d, %d, %d, %3.3f)\n", i, action.key_id[0], action.key_id[1], action.analog_value);
                }

                if (action.button_id != -1) {
                    fprintf(f, "register_action_button(%d, %d, %3.3f)\n", i, action.button_id, action.analog_value);
                }

                if (!(action.joystick_id == -1 && action.axis_id == -1)) {
                    fprintf(f, "register_action_joystick_axis(%d, %d, %d)\n", i, action.joystick_id, action.axis_id);
                }
            }
        }

        _debugprintf("Wrote action mapper config");
        fclose(f);
        return true;
    }

    bool load(string filename) {
        lua_State* L = luaL_newstate();

        Input::luaL_open_game_inputlib(L);
        luaL_open_game_actionlib(L);

        if (luaL_dofile(L, filename.data) != 0) {
            _debugprintf("Could not exec %.*s. Hopefully it doesn't exist.", filename.length, filename.data);
            lua_close(L);
            return false;
        }

        _debugprintf("Successfully loaded config from file");
        lua_close(L);
        return true;
    }
};
