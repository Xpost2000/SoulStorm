#ifndef INPUT_H
#define INPUT_H
#include "common.h"
#include "v2.h"

/*
  NOTE: I am keeping this with a C API just like it was
  in Legends since there's no real reason to change this

  as rewriting it to be more C++-like has little value.
*/

enum mouse_button {
    MOUSE_BUTTON_LEFT=1,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_COUNT,
};

enum gamepad_axis {
    GAMEPAD_AXIS_UNKNOWN,
    GAMEPAD_AXIS_X,
    GAMEPAD_AXIS_Y,

    GAMEPAD_AXIS_POSITIVE_X,
    GAMEPAD_AXIS_NEGATIVE_X,
    GAMEPAD_AXIS_POSITIVE_Y,
    GAMEPAD_AXIS_NEGATIVE_Y,
};

static const char* gamepad_axis_strings(int key) {
    switch (key) {
        case GAMEPAD_AXIS_UNKNOWN:
            return "GAMEPAD_AXIS_UNKNOWN";
        case GAMEPAD_AXIS_X:
            return "GAMEPAD_AXIS_X";
        case GAMEPAD_AXIS_Y:
            return "GAMEPAD_AXIS_Y";
        case GAMEPAD_AXIS_POSITIVE_X:
            return "GAMEPAD_AXIS_POSITIVE_X";
        case GAMEPAD_AXIS_NEGATIVE_X:
            return "GAMEPAD_AXIS_NEGATIVE_X";
        case GAMEPAD_AXIS_POSITIVE_Y:
            return "GAMEPAD_AXIS_POSITIVE_Y";
        case GAMEPAD_AXIS_NEGATIVE_Y:
            return "GAMEPAD_AXIS_NEGATIVE_Y";
    }
    return "?";
}

static const char* gamepad_axis_strings_lua(int key) {
    switch (key) {
        case GAMEPAD_AXIS_UNKNOWN:
            return "GamepadAxis.UNKNOWN";
        case GAMEPAD_AXIS_X:
            return "GamepadAxis.AXIS_X";
        case GAMEPAD_AXIS_Y:
            return "GamepadAxis.AXIS_Y";
        case GAMEPAD_AXIS_POSITIVE_X:
            return "GamepadAxis.AXIS_POSITIVE_X";
        case GAMEPAD_AXIS_NEGATIVE_X:
            return "GamepadAxis.AXIS_NEGATIVE_X";
        case GAMEPAD_AXIS_POSITIVE_Y:
            return "GamepadAxis.AXIS_POSITIVE_Y";
        case GAMEPAD_AXIS_NEGATIVE_Y:
            return "GamepadAxis.AXIS_NEGATIVE_Y";
    }
    return "?";
}

#define INVALID_INPUT (-1)
enum keyboard_button {
    KEY_UNKNOWN = 0,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,

    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,

    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,

    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,

    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,

    KEY_F11,
    KEY_F12,
    KEY_UP,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_LEFT,

    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    KEY_MINUS,
    KEY_BACKQUOTE,
    KEY_EQUALS,

    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_COMMA,

    KEY_PERIOD,
    KEY_RETURN,
    KEY_BACKSPACE,
    KEY_ESCAPE,


    KEY_INSERT,
    KEY_HOME,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_DELETE,
    KEY_END,

    KEY_PRINTSCREEN,


    /*
      I probably don't actually care about mapping these keys.
    */
    KEY_PAUSE,
    KEY_SCROLL_LOCK,
    KEY_NUMBER_LOCK,

    KEYPAD_0,
    KEYPAD_1,
    KEYPAD_2,
    KEYPAD_3,
    KEYPAD_4,

    KEYPAD_5,
    KEYPAD_6,
    KEYPAD_7,
    KEYPAD_8,
    KEYPAD_9,

    KEYPAD_LEFT,
    KEYPAD_RIGHT,
    KEYPAD_UP,
    KEYPAD_DOWN,

    KEYPAD_ASTERISK,
    KEYPAD_BACKSLASH,

    KEYPAD_MINUS,
    KEYPAD_PLUS,
    KEYPAD_PERIOD,

    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,

    KEY_FORWARDSLASH,
    KEY_BACKSLASH,


    KEY_TAB,
    KEY_SHIFT,

    KEY_META,
    KEY_SUPER,
    KEY_SPACE,


    KEY_CTRL,
    KEY_ALT,


    KEY_COUNT,
};

enum controller_button {
    BUTTON_UNKNOWN = 0,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_RS,
    BUTTON_LS,
    BUTTON_RB,
    BUTTON_LB,
    BUTTON_START,
    BUTTON_BACK,
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    BUTTON_COUNT,
};

static const char* controller_button_strings(int button) {
    switch (button) {
        case BUTTON_UNKNOWN: return "PAD_NONE";
        case BUTTON_A:       return "GAMEPAD_A";
        case BUTTON_B:       return "GAMEPAD_B";
        case BUTTON_X:       return "GAMEPAD_X";
        case BUTTON_Y:       return "GAMEPAD_Y";
        case BUTTON_RS:      return "GAMEPAD_RS";
        case BUTTON_LS:      return "GAMEPAD_LS";
        case BUTTON_RB:      return "GAMEPAD_RB";
        case BUTTON_LB:      return "GAMEPAD_LB";
        case BUTTON_START:   return "GAMEPAD_START";
        case BUTTON_BACK:    return "GAMEPAD_BACK";
        case DPAD_UP:        return "GAMEPAD_DPAD_UP";
        case DPAD_DOWN:      return "GAMEPAD_DPAD_DOWN";
        case DPAD_LEFT:      return "GAMEPAD_DPAD_LEFT";
        case DPAD_RIGHT:     return "GAMEPAD_DPAD_RIGHT";
    }
    return "?";
}

static const char* controller_button_strings_readable(int button) {
    switch (button) {
        case BUTTON_UNKNOWN: return "None";
        case BUTTON_A:       return "Gamepad A";
        case BUTTON_B:       return "Gamepad B";
        case BUTTON_X:       return "Gamepad X";
        case BUTTON_Y:       return "Gamepad Y";
        case BUTTON_RS:      return "Gamepad RS";
        case BUTTON_LS:      return "Gamepad LS";
        case BUTTON_RB:      return "Gamepad RB";
        case BUTTON_LB:      return "Gamepad LB";
        case BUTTON_START:   return "Gamepad Start";
        case BUTTON_BACK:    return "Gamepad Back";
        case DPAD_UP:        return "Gamepad Up";
        case DPAD_DOWN:      return "Gamepad Down";
        case DPAD_LEFT:      return "Gamepad Left";
        case DPAD_RIGHT:     return "Gamepad Right";
    }
    return "?";
}

static const char* controller_button_strings_lua(int button) {
    switch (button) {
        case BUTTON_UNKNOWN: return "Buttons.UNKNOWN";
        case BUTTON_A:       return "Buttons.A";
        case BUTTON_B:       return "Buttons.B";
        case BUTTON_X:       return "Buttons.X";
        case BUTTON_Y:       return "Buttons.Y";
        case BUTTON_RS:      return "Buttons.RS";
        case BUTTON_LS:      return "Buttons.LS";
        case BUTTON_RB:      return "Buttons.RB";
        case BUTTON_LB:      return "Buttons.LB";
        case BUTTON_START:   return "Buttons.START";
        case BUTTON_BACK:    return "Buttons.BACK";
        case DPAD_UP:        return "Buttons.DPAD_UP";
        case DPAD_DOWN:      return "Buttons.DPAD_DOWN";
        case DPAD_LEFT:      return "Buttons.DPAD_LEFT";
        case DPAD_RIGHT:     return "Buttons.DPAD_RIGHT";
    }
    return "?";
}

static const char* keyboard_key_strings(int key) {
    switch (key) {
        case KEY_UNKNOWN: return "Unknown Key?";
        case KEY_A: return "A";
        case KEY_B: return "B";
        case KEY_C: return "C";
        case KEY_D: return "D";
        case KEY_E: return "E";
        case KEY_F: return "F";
        case KEY_G: return "G";
        case KEY_H: return "H";
        case KEY_I: return "I";
        case KEY_J: return "J";
        case KEY_K: return "K";
        case KEY_L: return "L";
        case KEY_M: return "M";
        case KEY_N: return "N";
        case KEY_O: return "O";
        case KEY_P: return "P";
        case KEY_Q: return "Q";
        case KEY_R: return "R";
        case KEY_S: return "S";
        case KEY_T: return "T";
        case KEY_U: return "U";
        case KEY_V: return "V";
        case KEY_W: return "W";
        case KEY_X: return "X";
        case KEY_Y: return "Y";
        case KEY_Z: return "Z";

        case KEY_F1:  return "F1";
        case KEY_F2:  return "F2";
        case KEY_F3:  return "F3";
        case KEY_F4:  return "F4";
        case KEY_F5:  return "F5";
        case KEY_F6:  return "F6";
        case KEY_F7:  return "F7";
        case KEY_F8:  return "F8";
        case KEY_F9:  return "F9";
        case KEY_F10: return "F10";
        case KEY_F11: return "F11";
        case KEY_F12: return "F12";

        case KEY_UP: return "Arrow_Up";
        case KEY_DOWN: return "Arrow_Down";
        case KEY_RIGHT: return "Arrow_Right";
        case KEY_LEFT: return "Arrow_Left";
        case KEY_0:  return "0";
        case KEY_1:  return "1";
        case KEY_2:  return "2";
        case KEY_3:  return "3";
        case KEY_4:  return "4";
        case KEY_5:  return "5";
        case KEY_6:  return "6";
        case KEY_7:  return "7";
        case KEY_8:  return "8";
        case KEY_9:  return "9";
        case KEY_MINUS:     return "-";
        case KEY_BACKQUOTE: return "`";
        case KEY_EQUALS:    return "=";
        case KEY_SEMICOLON: return ";";
        case KEY_QUOTE:     return "\'";
        case KEY_COMMA:     return ",";
        case KEY_PERIOD:    return ".";
        case KEY_RETURN:    return "Return";
        case KEY_BACKSPACE: return "Backspace";
        case KEY_ESCAPE:    return "Escape";
        case KEY_INSERT:   return "Insert";
        case KEY_HOME:     return "Home";
        case KEY_PAGEUP:   return "Page Up";
        case KEY_PAGEDOWN: return "Page Down";
        case KEY_DELETE:   return "Delete";
        case KEY_END:      return "End";
        case KEY_PRINTSCREEN: return "Print Screen";
        case KEY_PAUSE:       return "Pause";
        case KEY_SCROLL_LOCK: return "Scroll Lock";
        case KEY_NUMBER_LOCK: return "Number Lock";
        case KEY_LEFT_BRACKET:  return "[";
        case KEY_RIGHT_BRACKET: return "]";
        case KEY_FORWARDSLASH:  return "/";
        case KEY_BACKSLASH:     return "\\";
        case KEY_TAB:   return "Tab";
        case KEY_SHIFT: return "Shift";
        case KEY_SPACE: return "Space";
        case KEY_CTRL:  return "Control";
        case KEY_ALT:   return "Alt";
    }
    return "?";
}

static const char* keyboard_key_strings_readable(int key) {
    switch (key) {
        case KEY_UNKNOWN: return "Unknown Key?";
        case KEY_A: return "A";
        case KEY_B: return "B";
        case KEY_C: return "C";
        case KEY_D: return "D";
        case KEY_E: return "E";
        case KEY_F: return "F";
        case KEY_G: return "G";
        case KEY_H: return "H";
        case KEY_I: return "I";
        case KEY_J: return "J";
        case KEY_K: return "K";
        case KEY_L: return "L";
        case KEY_M: return "M";
        case KEY_N: return "N";
        case KEY_O: return "O";
        case KEY_P: return "P";
        case KEY_Q: return "Q";
        case KEY_R: return "R";
        case KEY_S: return "S";
        case KEY_T: return "T";
        case KEY_U: return "U";
        case KEY_V: return "V";
        case KEY_W: return "W";
        case KEY_X: return "X";
        case KEY_Y: return "Y";
        case KEY_Z: return "Z";

        case KEY_F1:  return "F1";
        case KEY_F2:  return "F2";
        case KEY_F3:  return "F3";
        case KEY_F4:  return "F4";
        case KEY_F5:  return "F5";
        case KEY_F6:  return "F6";
        case KEY_F7:  return "F7";
        case KEY_F8:  return "F8";
        case KEY_F9:  return "F9";
        case KEY_F10: return "F10";
        case KEY_F11: return "F11";
        case KEY_F12: return "F12";

        case KEY_UP: return "Up Arrow";
        case KEY_DOWN: return "Down Arrow";
        case KEY_RIGHT: return "Right Arrow";
        case KEY_LEFT: return "Left Arrow";
        case KEY_0:  return "0";
        case KEY_1:  return "1";
        case KEY_2:  return "2";
        case KEY_3:  return "3";
        case KEY_4:  return "4";
        case KEY_5:  return "5";
        case KEY_6:  return "6";
        case KEY_7:  return "7";
        case KEY_8:  return "8";
        case KEY_9:  return "9";
        case KEY_MINUS:     return "-";
        case KEY_BACKQUOTE: return "`";
        case KEY_EQUALS:    return "=";
        case KEY_SEMICOLON: return ";";
        case KEY_QUOTE:     return "\'";
        case KEY_COMMA:     return ",";
        case KEY_PERIOD:    return ".";
        case KEY_RETURN:    return "Return/Enter";
        case KEY_BACKSPACE: return "Backspace";
        case KEY_ESCAPE:    return "Escape";
        case KEY_INSERT:   return "Insert";
        case KEY_HOME:     return "Home";
        case KEY_PAGEUP:   return "Page Up";
        case KEY_PAGEDOWN: return "Page Down";
        case KEY_DELETE:   return "Delete";
        case KEY_END:      return "End";
        case KEY_PRINTSCREEN: return "Print Screen";
        case KEY_PAUSE:       return "Pause";
        case KEY_SCROLL_LOCK: return "Scroll Lock";
        case KEY_NUMBER_LOCK: return "Number Lock";
        case KEY_LEFT_BRACKET:  return "[";
        case KEY_RIGHT_BRACKET: return "]";
        case KEY_FORWARDSLASH:  return "/";
        case KEY_BACKSLASH:     return "\\";
        case KEY_TAB:   return "Tab";
        case KEY_SHIFT: return "Shift";
        case KEY_SPACE: return "Space";
        case KEY_CTRL:  return "Control";
        case KEY_ALT:   return "Alt";
    }
    return "?";
}

static const char* keyboard_key_strings_lua(int key) {
    switch (key) {
        case KEY_UNKNOWN: return "Keys.UNKNOWN";
        case KEY_A: return "Keys.A";
        case KEY_B: return "Keys.B";
        case KEY_C: return "Keys.C";
        case KEY_D: return "Keys.D";
        case KEY_E: return "Keys.E";
        case KEY_F: return "Keys.F";
        case KEY_G: return "Keys.G";
        case KEY_H: return "Keys.H";
        case KEY_I: return "Keys.I";
        case KEY_J: return "Keys.J";
        case KEY_K: return "Keys.K";
        case KEY_L: return "Keys.L";
        case KEY_M: return "Keys.M";
        case KEY_N: return "Keys.N";
        case KEY_O: return "Keys.O";
        case KEY_P: return "Keys.P";
        case KEY_Q: return "Keys.Q";
        case KEY_R: return "Keys.R";
        case KEY_S: return "Keys.S";
        case KEY_T: return "Keys.T";
        case KEY_U: return "Keys.U";
        case KEY_V: return "Keys.V";
        case KEY_W: return "Keys.W";
        case KEY_X: return "Keys.X";
        case KEY_Y: return "Keys.Y";
        case KEY_Z: return "Keys.Z";

        case KEY_F1:  return "Keys.F1";
        case KEY_F2:  return "Keys.F2";
        case KEY_F3:  return "Keys.F3";
        case KEY_F4:  return "Keys.F4";
        case KEY_F5:  return "Keys.F5";
        case KEY_F6:  return "Keys.F6";
        case KEY_F7:  return "Keys.F7";
        case KEY_F8:  return "Keys.F8";
        case KEY_F9:  return "Keys.F9";
        case KEY_F10: return "Keys.F10";
        case KEY_F11: return "Keys.F11";
        case KEY_F12: return "Keys.F12";

        case KEY_UP: return "Keys.UP";
        case KEY_DOWN: return "Keys.DOWN";
        case KEY_RIGHT: return "Keys.RIGHT";
        case KEY_LEFT: return "Keys.LEFT";
        case KEY_0:  return "Keys.KEY_0";
        case KEY_1:  return "Keys.KEY_1";
        case KEY_2:  return "Keys.KEY_2";
        case KEY_3:  return "Keys.KEY_3";
        case KEY_4:  return "Keys.KEY_4";
        case KEY_5:  return "Keys.KEY_5";
        case KEY_6:  return "Keys.KEY_6";
        case KEY_7:  return "Keys.KEY_7";
        case KEY_8:  return "Keys.KEY_8";
        case KEY_9:  return "Keys.KEY_9";
        case KEY_MINUS:     return "Keys.MINUS";
        case KEY_BACKQUOTE: return "Keys.BACKQUOTE";
        case KEY_EQUALS:    return "Keys.EQUALS";
        case KEY_SEMICOLON: return "Keys.SEMICOLON";
        case KEY_QUOTE:     return "Keys.QUOTE";
        case KEY_COMMA:     return "Keys.COMMA";
        case KEY_PERIOD:    return "Keys.PERIOD";
        case KEY_RETURN:    return "Keys.RETURN";
        case KEY_BACKSPACE: return "Keys.BACKSPACE";
        case KEY_ESCAPE:    return "Keys.ESCAPE";
        case KEY_INSERT:   return  "Keys.INSERT";
        case KEY_HOME:     return "Keys.HOME";
        case KEY_PAGEUP:   return "Keys.PAGEUP";
        case KEY_PAGEDOWN: return "Keys.PAGEDOWN";
        case KEY_DELETE:   return "Keys.DELETE";
        case KEY_END:      return "Keys.END";
        case KEY_PRINTSCREEN: return "Keys.PRINTSCREEN";
        case KEY_PAUSE:       return "Keys.PAUSE";
        case KEY_SCROLL_LOCK: return "Keys.SCROLL_LOCK";
        case KEY_NUMBER_LOCK: return "Keys.NUMBER_LOCK";
        case KEY_LEFT_BRACKET:  return "Keys.LEFT_BRACKET";
        case KEY_RIGHT_BRACKET: return "Keys.RIGHT_BRACKET";
        case KEY_FORWARDSLASH:  return "Keys.FORWARDSLASH";
        case KEY_BACKSLASH:     return "Keys.BACKSPACE";
        case KEY_TAB:   return "Keys.TAB";
        case KEY_SHIFT: return "Keys.SHIFT";
        case KEY_SPACE: return "Keys.SPACE";
        case KEY_CTRL:  return "Keys.CTRL";
        case KEY_ALT:   return "Keys.ALT";
    }
    return "?";
}

enum {
    CONTROLLER_JOYSTICK_LEFT,
    CONTROLLER_JOYSTICK_RIGHT,
    CONTROLLER_JOYSTICK_UNKNOWN,
};

static const char* controller_joystick_strings(int joystick) {
    switch (joystick) {
        case CONTROLLER_JOYSTICK_LEFT:
            return "CONTROLLER_JOYSTICK_LEFT";
        case CONTROLLER_JOYSTICK_RIGHT:
            return "CONTROLLER_JOYSTICK_RIGHT";
        case CONTROLLER_JOYSTICK_UNKNOWN:
            return "CONTROLLER_JOYSTICK_UNKNOWN";
    }

    return 0;
}

static const char* controller_joystick_strings_lua(int joystick) {
    switch (joystick) {
        case CONTROLLER_JOYSTICK_LEFT:
            return "Joystick.LEFT";
        case CONTROLLER_JOYSTICK_RIGHT:
            return "Joystick.RIGHT";
        case CONTROLLER_JOYSTICK_UNKNOWN:
            return "Joystick.UNKNOWN";
    }

    return 0;
}

struct game_controller_triggers {
    f32 left;
    f32 right;
};
#define CONTROLLER_AXIS_FLICK_TOLERANCE (0.15)
struct game_controller_joystick {
    f32 axes[2];
};
struct game_controller {
    struct game_controller_triggers triggers;
    u8                              buttons[BUTTON_COUNT];
    struct game_controller_joystick left_stick;
    struct game_controller_joystick right_stick;

    struct game_controller_triggers last_triggers;
    u8                              last_buttons[BUTTON_COUNT];
    struct game_controller_joystick last_left_stick;
    struct game_controller_joystick last_right_stick;

    u8                              buttons_that_received_events[BUTTON_COUNT];
    void* _internal_controller_handle;
};

struct input_state {
    bool keys[KEY_COUNT];
    s32  mouse_x;
    s32  mouse_y;
    bool mouse_buttons[MOUSE_BUTTON_COUNT];
    s32  mouse_wheel_relative_x;
    s32  mouse_wheel_relative_y;

    bool editing_text;
    s32  text_edit_cursor;
    char text[1024];
    // For a bigger engine, it might be important to have a layer consumption system
    // but since this game is small I can afford to just directly handle UI input state consumption
};

struct lua_State;
namespace Input {
    game_controller* get_game_controller(int idx);

    const char* controller_button_to_string(s32 buttonid);
    void   controller_rumble(struct game_controller* controller, f32 x_magnitude, f32 y_magnitude, u32 ms);
    bool   controller_button_down(struct game_controller* controller, u8 button_id);
    bool   controller_button_down_with_repeat(struct game_controller* controller, u8 button_id);
    bool   controller_button_pressed(struct game_controller* controller, u8 button_id);
    f32    controller_left_axis(struct game_controller* controller, u8 axis_id);
    f32    controller_right_axis(struct game_controller* controller, u8 axis_id);

    // TODO: neither of these are accurate or allow the "full axis range" I put in the above enums.
    bool   controller_left_axis_flicked(struct game_controller* controller, u8 axis_id);
    bool   controller_right_axis_flicked(struct game_controller* controller, u8 axis_id);

    const char* keycode_to_string(s32 keyid);
    struct game_controller* get_gamepad(s32 index);

    float angle_formed_by_joystick(struct game_controller* controller, s32 which);
    void register_key_down(s32 keyid);
    void register_key_up(s32 keyid);
    void register_controller_down(s32 which, s32 button);
    void register_mouse_position(s32 x, s32 y);
    void register_mouse_wheel(s32 x, s32 y);
    void register_mouse_button(s32 button_id, bool state);

    // Consume an input by just setting it to false.
    void eat_key(s32 keyid);
    void eat_controller_button(s32 which, s32 button);
    void eat_mouse_button(s32 button_id);

    s32  mouse_wheel_x(void);
    s32  mouse_wheel_y(void);
    bool is_mouse_wheel_up(void);
    bool is_mouse_wheel_down(void);

    bool is_key_down(s32 keyid);
    bool is_key_down_with_repeat(s32 keyid);
    bool is_key_pressed(s32 keyid);
    s32 any_key_down(void);
    s32 any_key_pressed(void);
    s32 controller_any_button_down(struct game_controller* controller);
    s32 controller_any_button_pressed(struct game_controller* controller);

    void get_mouse_location(s32* mx, s32* my);
    void get_mouse_buttons(bool* left, bool* middle, bool* right);
    void get_mouse_buttons_pressed(bool* left, bool* middle, bool* right);
    bool mouse_left(void);
    bool mouse_middle(void);
    bool mouse_right(void);
    bool pressed_mouse_left(void);
    bool pressed_mouse_middle(void);
    bool pressed_mouse_right(void);
    V2 mouse_location(void);

    rectangle_f32 mouse_rectangle(f32 k=5);

    void initialize(void);
    void begin_input_frame(void);
    void end_input_frame(void);

    void start_text_edit(char* target, size_t length);
    void end_text_edit(char* target, size_t amount); /*copies all text input s32o target buffer. Not necessarily unicode aware. whoops!*/

    void send_text_input(char* text, size_t text_length);
    bool is_editing_text(void);
    char* current_text_buffer(void);

    bool any_input_activity(void);

    int luaL_open_game_inputlib(lua_State* L);
}

// NOTE: platform layer handled.
void controller_rumble_set(bool v);
// NOTE: SDL handles rumble on a separate layer
// so it's fine.
void controller_rumble(struct game_controller* controller, f32 x_magnitude, f32 y_magnitude, u32 ms);

// TODO: I want a nicer led api, even though it might be hard to support the flashing
// however, it does not appear to support this for
// the LED setting, so I'll make an analogous api
void controller_set_led(struct game_controller* controller, u8 r, u8 g, u8 b);

#endif
