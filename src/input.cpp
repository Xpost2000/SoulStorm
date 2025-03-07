extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "input.h"

// NOTE: if I'm exposing this to a config, especially a lua one where arbitrary parsing
//       can happen, I need to make these actually error out sometimes!

namespace Input {
    struct {
        struct input_state current_state;
        struct input_state last_state;

        bool keys_receiving_events[KEY_COUNT];
    } global_input = {};

    struct game_controller global_controllers[4];

    void initialize() {
        zero_memory(global_controllers, sizeof(global_controllers));
        zero_memory(&global_input,      sizeof(global_input));
    }

    game_controller* get_game_controller(int idx) {
        return &global_controllers[idx];
    }

    float angle_formed_by_joystick(game_controller* controller, s32 which) {
        struct game_controller_joystick target;
        switch (which) {
            case CONTROLLER_JOYSTICK_LEFT: {
                target = controller->left_stick;
            } break;
            case CONTROLLER_JOYSTICK_RIGHT: {
                target = controller->right_stick;
            } break;
        }

        f32 angle = atan2(target.axes[1], target.axes[0]);
        return angle;
    }

    void eat_key(s32 keyid) {
        global_input.current_state.keys[keyid] = false;
        global_input.last_state.keys[keyid] = false;
    }

    void eat_controller_button(s32 which, s32 button) {
        if (which < 0 || which >= 4) {
            return;
        }

        auto& controller = global_controllers[which];
        controller.buttons[button] = false;
        controller.last_buttons[button] = false;
        controller.buttons_that_received_events[button] = false;
    }

    void eat_mouse_button(s32 button_id) {
        global_input.current_state.mouse_buttons[button_id] = false;
        global_input.last_state.mouse_buttons[button_id]    = false;
    }

    void register_key_down(s32 keyid) {
        /*
          I literally don't know anywhere better to put this text editing specific
          code, and I always just throw it like here.
        */
        if (keyid == KEY_BACKSPACE) {
            if (global_input.current_state.editing_text && global_input.current_state.text_edit_cursor > 0) {
                global_input.current_state.text[--global_input.current_state.text_edit_cursor] = 0;
            }
        }
        global_input.current_state.keys[keyid]    = true;
        global_input.keys_receiving_events[keyid] = true;
    }

    void register_key_up(s32 keyid) {
        global_input.current_state.keys[keyid] = false;
    }

    void register_mouse_position(s32 x, s32 y) {
        global_input.current_state.mouse_x = x;
        global_input.current_state.mouse_y = y;
    }

    void register_mouse_wheel(s32 x, s32 y) {
        global_input.current_state.mouse_wheel_relative_x = x;
        global_input.current_state.mouse_wheel_relative_y = y;
    }

    void register_mouse_button(s32 button_id, bool state) {
        assertion((button_id >= 0 && button_id < MOUSE_BUTTON_COUNT) && "wtf?");
        global_input.current_state.mouse_buttons[button_id] = state;
    }

    void get_last_mouse_location(s32* mx, s32* my) {
        safe_assignment(mx) = global_input.last_state.mouse_x;
        safe_assignment(my) = global_input.last_state.mouse_y;
    }

    void get_mouse_location(s32* mx, s32* my) {
        safe_assignment(mx) = global_input.current_state.mouse_x;
        safe_assignment(my) = global_input.current_state.mouse_y;
    }

    void get_mouse_buttons(bool* left, bool* middle, bool* right) {
        safe_assignment(left)   = global_input.current_state.mouse_buttons[MOUSE_BUTTON_LEFT];
        safe_assignment(middle) = global_input.current_state.mouse_buttons[MOUSE_BUTTON_MIDDLE];
        safe_assignment(right)  = global_input.current_state.mouse_buttons[MOUSE_BUTTON_RIGHT];
    }
    void get_mouse_buttons_pressed(bool* left, bool* middle, bool* right) {
        safe_assignment(left)   = pressed_mouse_left();
        safe_assignment(middle) = pressed_mouse_middle();
        safe_assignment(right)  = pressed_mouse_right();
    }

    bool is_key_down_with_repeat(s32 keyid) {
        assertion(keyid < KEY_COUNT && "invalid key id?");
        return global_input.keys_receiving_events[keyid];
    }

    bool is_key_down(s32 keyid) {
        assertion(keyid < KEY_COUNT && "invalid key id?");
        return global_input.current_state.keys[keyid];
    }

    bool is_key_pressed(s32 keyid) {
        bool current_keystate = global_input.current_state.keys[keyid];
        bool last_keystate = global_input.last_state.keys[keyid];

        if (current_keystate == last_keystate) {
            return false;
        }

        if (current_keystate == true && last_keystate == false) {
            return true;
        }

        return false;
    }

    bool controller_button_down_with_repeat(struct game_controller* controller, u8 button_id) {
        if (controller) {
            bool current = controller->buttons_that_received_events[button_id];
            return current;
        }

        return false;
    }

    bool controller_button_down(struct game_controller* controller, u8 button_id) {
        if (controller) {
            bool current = controller->buttons[button_id];
            return current;
        }

        return false;
    }

    local f32 joystick_axis(struct game_controller_joystick* axes, u8 axis_id) {
        switch (axis_id) {
            case GAMEPAD_AXIS_X: return axes->axes[0];
            case GAMEPAD_AXIS_Y: return axes->axes[1];
            case GAMEPAD_AXIS_POSITIVE_X: {
                if (axes->axes[0] > 0)
                    return axes->axes[0];
                else
                    return 0.0f;
            } break;
            case GAMEPAD_AXIS_NEGATIVE_X: {
                if (axes->axes[0] < 0)
                    return axes->axes[0];
                else
                    return 0.0f;
            } break;
            case GAMEPAD_AXIS_POSITIVE_Y: {
                if (axes->axes[1] > 0)
                    return axes->axes[1];
                else
                    return 0.0f;
            } break;
            case GAMEPAD_AXIS_NEGATIVE_Y: {
                if (axes->axes[1] < 0)
                    return axes->axes[1];
                else
                    return 0.0f;
            } break;
        }

        return 0.0f;
    }

    f32 controller_left_axis(struct game_controller* controller, u8 axis_id) {
        if (!controller) return 0.0f;
        return joystick_axis(&controller->left_stick, axis_id);
    }

    f32 controller_right_axis(struct game_controller* controller, u8 axis_id) {
        if (!controller) return 0.0f;
        return joystick_axis(&controller->right_stick, axis_id);
    }

    bool controller_button_pressed(struct game_controller* controller, u8 button_id) {
        if (controller) {
            bool last    = controller->last_buttons[button_id];
            bool current = controller->buttons[button_id];

            if (last == current) {
                return false;
            } else if (current == true && last == false) {
                return true;
            }
        }

        return false;
    }

    bool controller_left_axis_flicked(struct game_controller* controller, u8 axis_id) {
        if (controller) {
            auto last    = controller->last_left_stick;
            auto current = controller->left_stick;

            f32 difference = joystick_axis(&current, axis_id) - joystick_axis(&last, axis_id);

            return difference >= CONTROLLER_AXIS_FLICK_TOLERANCE;
        }

        return false;
    }

    bool controller_right_axis_flicked(struct game_controller* controller, u8 axis_id) {
        if (controller) {
            auto last    = controller->last_right_stick;
            auto current = controller->right_stick;

            f32 difference = joystick_axis(&current, axis_id) - joystick_axis(&last, axis_id);

            return difference >= CONTROLLER_AXIS_FLICK_TOLERANCE;
        }

        return false;
    }

    struct game_controller* get_gamepad(s32 index) {
        assertion(index >= 0 && index < (s32)array_count(global_controllers) && "Bad controller index");
        return global_controllers + index;
    }

    void begin_input_frame(void) {
        /* nope */
    }

    void end_input_frame(void) {
        global_input.last_state = global_input.current_state;

        for (unsigned index = 0; index < array_count(global_controllers); ++index) {
            struct game_controller* controller = global_controllers + index;

            {
                controller->last_triggers    = controller->triggers;
                controller->last_left_stick  = controller->left_stick;
                controller->last_right_stick = controller->right_stick;
                memory_copy(controller->buttons, controller->last_buttons, sizeof(controller->buttons));
            }
        }

        global_input.current_state.mouse_wheel_relative_x = 0;
        global_input.current_state.mouse_wheel_relative_y = 0;

        zero_array(global_input.keys_receiving_events);
        for (s32 controller_index = 0; controller_index < (s32)array_count(global_controllers); ++controller_index) {
            zero_array(global_controllers[controller_index].buttons_that_received_events);
        }
    }

    void start_text_edit(char* target, size_t length) {
        if (!global_input.current_state.editing_text) {
            zero_array(global_input.current_state.text);
            if (target) memory_copy(global_input.current_state.text, target, length);
            global_input.current_state.text_edit_cursor = length;
            global_input.current_state.editing_text = true;
        }
    }

    void end_text_edit(char* target_buffer, size_t target_buffer_size) {
        if (target_buffer && target_buffer_size) {
            memory_copy(target_buffer, global_input.current_state.text, target_buffer_size);
            target_buffer[target_buffer_size-1] = 0;
        }

        zero_array(global_input.current_state.text);
        global_input.current_state.editing_text = false;
    }

    void send_text_input(char* text, size_t text_length) {
        for (size_t index = 0; index < text_length && global_input.current_state.text_edit_cursor < 1024; ++index) {
            global_input.current_state.text[global_input.current_state.text_edit_cursor++] = text[index];
        }
    }

    bool is_editing_text(void) {
        return global_input.current_state.editing_text;
    }

    char* current_text_buffer(void) {
        if (global_input.current_state.editing_text) {
            return global_input.current_state.text;
        }
        return (char*)"";
    }

    s32 any_key_down(void) {
        for (unsigned index = 0; index < array_count(global_input.current_state.keys); ++index) {
            if (global_input.current_state.keys[index]) {
                return index;
            }
        }

        return 0;
    }

    s32 any_key_pressed(void) {
        for (unsigned index = 0; index < array_count(global_input.current_state.keys); ++index) {
            if (!global_input.last_state.keys[index] && global_input.current_state.keys[index]) {
                return index;
            }
        }

        return 0;
    }

    s32 controller_any_button_pressed(struct game_controller* controller) {
        if (!controller)
            return 0;

        for (unsigned index = 0; index < array_count(controller->buttons); ++index) {
            if (!controller->last_buttons[index] && controller->buttons[index]) {
                return index;
            }
        }

        return 0;
    }

    s32 controller_any_button_down(struct game_controller* controller) {
        if (!controller)
            return 0;

        for (unsigned index = 0; index < array_count(controller->buttons); ++index) {
            if (controller->buttons[index]) {
                return index;
            }
        }

        return 0;
    }

    V2 last_mouse_location(void) {
        s32 mouse_positions[2];
        get_last_mouse_location(mouse_positions, mouse_positions+1);

        return V2(mouse_positions[0], mouse_positions[1]);
    }

    bool any_input_activity(void) {
        V2 last_mouse = last_mouse_location();
        V2 current_mouse = mouse_location();

        V2 diff = current_mouse - last_mouse;

        if (!f32_close_enough(diff.x, 0) ||
            !f32_close_enough(diff.y, 0)) {
            return true;
        }

        return
            controller_any_button_down(get_gamepad(0)) ||
            any_key_down()                             ||
            mouse_left()                               ||
            mouse_right()                              ||
            mouse_middle();
    }

    V2 mouse_location(void) {
        s32 mouse_positions[2];
        get_mouse_location(mouse_positions, mouse_positions+1);

        return V2(mouse_positions[0], mouse_positions[1]);
    }

    s32 mouse_wheel_x(void) {
        return global_input.current_state.mouse_wheel_relative_x;
    }

    s32 mouse_wheel_y(void) {
        return global_input.current_state.mouse_wheel_relative_y;
    }

    bool is_mouse_wheel_up(void) {
        s32 y = mouse_wheel_y();

        if (y > 0) {
            return true;
        }

        return false;
    }

    bool is_mouse_wheel_down(void) {
        s32 y = mouse_wheel_y();

        if (y < 0) {
            return true;
        }

        return false;
    }

    const char* keycode_to_string(s32 keyid) {
        if (keyid == 0) {
            return "KEY_NONE";
        }

        const char* result = keyboard_key_strings(keyid);
        assertion(result && "A string does not exist for this key yet...");
        return result;
    }

    // s32 string_to_keyid(const char* keystring) {
    //     for (s32 string_index = 0; string_index < (s32)array_count(keyboard_key_strings); ++string_index) {
    //         if (keyboard_key_strings[string_index]) {
    //             const char* str = keyboard_key_strings[string_index];

    //             if (strcmp(keystring, str) == 0) {
    //                 return string_index;
    //             }
    //         }
    //     }

    //     return KEY_UNKNOWN;
    // }

    const char* controller_button_to_string(s32 buttonid) {
        if (buttonid == 0) {
            return "GAMEPAD_NONE";
        }
        return controller_button_strings(buttonid);
    }

    // s32 string_to_controller_button(const char* bruttonstring) {
    //     for (s32 string_index = 0; string_index < (s32)array_count(controller_button_strings); ++string_index) {
    //         if (strcmp(buttonstring, controller_button_strings[string_index]) == 0) {
    //             return string_index;
    //         }
    //     }

    //     return BUTTON_UNKNOWN;
    // }

    bool mouse_left(void) {
        return global_input.current_state.mouse_buttons[MOUSE_BUTTON_LEFT];
    }

    bool mouse_middle(void) {
        return global_input.current_state.mouse_buttons[MOUSE_BUTTON_MIDDLE];
    }

    bool mouse_right(void) {
        return global_input.current_state.mouse_buttons[MOUSE_BUTTON_RIGHT];
    }

    bool pressed_mouse_left(void) {
        bool last = global_input.last_state.mouse_buttons[MOUSE_BUTTON_LEFT];
        bool current = global_input.current_state.mouse_buttons[MOUSE_BUTTON_LEFT];

        if (last == current) {
            return false;
        } else if (current && !last) {
            return true;
        }

        return false;
    }

    bool pressed_mouse_middle(void) {
        bool last = global_input.last_state.mouse_buttons[MOUSE_BUTTON_MIDDLE];
        bool current = global_input.current_state.mouse_buttons[MOUSE_BUTTON_MIDDLE];

        if (last == current) {
            return false;
        } else if (current && !last) {
            return true;
        }

        return false;
    }

    bool pressed_mouse_right(void) {
        bool last = global_input.last_state.mouse_buttons[MOUSE_BUTTON_RIGHT];
        bool current = global_input.current_state.mouse_buttons[MOUSE_BUTTON_RIGHT];

        if (last == current) {
            return false;
        } else if (current && !last) {
            return true;
        }

        return false;
    }

    rectangle_f32 mouse_rectangle(f32 k) {
        V2 mouse_position = Input::mouse_location();
        return rectangle_f32(mouse_position.x, mouse_position.y, k, k);
    }

    int luaL_open_game_inputlib(lua_State* L) {
        /*
         * Using the Lua C API to make a giant table is a recipe for disaster
         * imo, so I'm just going to inline write the code...
         *
         * I'm just going to say. I'm glad my enums are basically never changing...
         *
         * There's no real code, since I don't want the lua script to have access to
         * the raw engine stuff. So this is really just an enum library.
         */
        const char* lua_code = R"(
GamepadAxis = {
UNKNOWN = 0,
AXIS_X = 1,
AXIS_Y = 2,

AXIS_POSITIVE_X = 3,
AXIS_NEGATIVE_X = 4,
AXIS_POSITIVE_Y = 5,
AXIS_NEGATIVE_Y = 6,
}

Joystick = {
LEFT = 0,
RIGHT = 1,
UNKNOWN = 2,
}

Buttons = {
UNKNOWN = 0,
A = 1,
B = 2,
X = 3,
Y = 4,
RS = 5,
LS = 6,
RB = 7,
LB = 8,
START = 9,
BACK = 10,
DPAD_UP = 11,
DPAD_DOWN = 12,
DPAD_LEFT = 13,
DPAD_RIGHT = 14
}

Keys = {
UNKNOWN          = 0,
A                = 1,
B                = 2,
C                = 3,
D                = 4,
E                = 5,
F                = 6,
G                = 7,
H                = 8,
I                = 9,
J                = 10,
K                = 11,
L                = 12,
M                = 13,
N                = 14,
O                = 15,
P                = 16,
Q                = 17,
R                = 18,
S                = 19,
T                = 20,
U                = 21,
V                = 22,
W                = 23,
X                = 24,
Y                = 25,
Z                = 26,
F1               = 27,
F2               = 28,
F3               = 29,
F4               = 30,
F5               = 31,
F6               = 32,
F7               = 33,
F8               = 34,
F9               = 35,
F10              = 36,
F11              = 37,
F12              = 38,
UP               = 39,
DOWN             = 40,
RIGHT            = 41,
LEFT             = 42,
KEY_0            = 43,
KEY_1            = 44,
KEY_2            = 45,
KEY_3            = 46,
KEY_4            = 47,
KEY_5            = 48,
KEY_6            = 49,
KEY_7            = 50,
KEY_8            = 51,
KEY_9            = 52,
MINUS            = 53,
BACKQUOTE        = 54,
EQUALS           = 55,
SEMICOLON        = 56,
QUOTE            = 57,
COMMA            = 58,
PERIOD           = 59,
RETURN           = 60,
BACKSPACE        = 61,
ESCAPE           = 62,
INSERT           = 63,
HOME             = 64,
PAGEUP           = 65,
PAGEDOWN         = 66,
DELETE           = 67,
END              = 68,
PRINTSCREEN      = 69,
PAUSE            = 70,
SCROLL_LOCK      = 71,
NUMBER_LOCK      = 72,
KEYPAD_0         = 73,
KEYPAD_1         = 74,
KEYPAD_2         = 75,
KEYPAD_3         = 76,
KEYPAD_4         = 77,
KEYPAD_5         = 78,
KEYPAD_6         = 79,
KEYPAD_7         = 80,
KEYPAD_8         = 81,
KEYPAD_9         = 82,
KEYPAD_LEFT      = 83,
KEYPAD_RIGHT     = 84,
KEYPAD_UP        = 85,
KEYPAD_DOWN      = 86,
KEYPAD_ASTERISK  = 87,
KEYPAD_BACKSLASH = 88,
KEYPAD_MINUS     = 89,
KEYPAD_PLUS      = 90,
KEYPAD_PERIOD    = 91,
LEFT_BRACKET     = 92,
RIGHT_BRACKET    = 93,
FORWARDSLASH     = 94,
BACKSLASH        = 95,
TAB              = 96,
SHIFT            = 97,
META             = 98,
SUPER            = 99,
SPACE            = 100,
CTRL             = 101,
ALT              = 102
}
)";
        luaL_dostring(L, lua_code);
        return 1;
    }
}
