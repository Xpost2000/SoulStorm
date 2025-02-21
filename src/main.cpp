/*
 * A new engine. A new project.
 *
 * Platform layer
 *
 * Most of the other code is otherwise pretty independent.
 *
 */

// I wish I made this up.
//#define UNUSUAL_INTEL_UHD_OPENGL_FULLSCREEN_FIX 
//#define NO_FANCY_FADEIN_INTRO
#define JDR_COROUTINE_IMPLEMENTATION
#include "common.h"

#ifdef _WIN32
/*
* Ugh... C++ got modules too late for it to really do any good.
* 
* I'm honestly, not sure at what point I reinclude windows.h somewhere else, which is mildly concerning
* but I'm more concerned by the fact that d3d11.h will break if it isn't included directly after windows.h
*/
#include <Windows.h>
#include "graphics_driver_d3d11.h"
#undef near
#undef far
#endif

#include <SDL2/SDL.h>

#include "graphics_driver.h"
#include "graphics_driver_opengl.h"
#include "graphics_driver_software.h"
#include "graphics_driver_null.h"

#include "audio.h"
#include "input.h"
#include "sdl_scancode_table.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "game.h"
#include "game_state.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}


// NOTE: only desktop targets!
#include "discord_rich_presence_integration.h"

enum graphics_device_type {
    GRAPHICS_DEVICE_SOFTWARE = 0,
    GRAPHICS_DEVICE_OPENGL = 1,
    GRAPHICS_DEVICE_D3D11 = 2,
    GRAPHICS_DEVICE_NULL = 3,
};
s32 last_graphics_device_id = GRAPHICS_DEVICE_NULL;
local void set_graphics_device(s32 id);

/*
 * calling confirm_preferences in the middle
 * of the game update render loop has... potentially
 * disasterous results, so I'm going to defer all the preference updates.
 *
 * So that way no unexpected execution happens or hard to reason about state happens....
 */
local bool queued_preference_update = false;

Software_Renderer_Graphics_Driver global_software_renderer_driver;
Null_Graphics_Driver              global_null_renderer_driver;
OpenGL_Graphics_Driver            global_opengl_renderer_driver;
#ifdef _WIN32
Direct3D11_Graphics_Driver        global_direct3d11_renderer_driver;
#endif
Graphics_Driver* global_graphics_driver = &global_null_renderer_driver;
const char* _build_flags =
#ifdef USE_SIMD_OPTIMIZATIONS
    "using simd,"
#endif
#ifdef MULTITHREADED_EXPERIMENTAL
    "using mt-sw,"
#endif
#ifdef _WIN32
  "win32,"
#else
  "other,"
#endif
#ifdef BUILD_DEMO
   "demo,"
#endif
#ifdef RELEASE
    "release,"
#else
    "debug,"
#endif
    ;

#define FRAMETIME_SAMPLE_MAX (32)
struct {
    u16 index;
    u16 length;
    f32 data[FRAMETIME_SAMPLE_MAX];
} global_frametime_sample_array = {};

void add_frametime_sample(f32 data) {
    global_frametime_sample_array.data[global_frametime_sample_array.index++] = data;
    if (global_frametime_sample_array.length <  FRAMETIME_SAMPLE_MAX) global_frametime_sample_array.length++;
    if (global_frametime_sample_array.index  >= FRAMETIME_SAMPLE_MAX) global_frametime_sample_array.index = 0;
}

f32 get_average_frametime(void) {
    f32 sum = 0.0;

    for (unsigned index = 0; index < global_frametime_sample_array.length; ++index) {
        sum += global_frametime_sample_array.data[index];
    }

    return sum / global_frametime_sample_array.length;
}

// Globals
#define TARGET_TICKS_PER_SECOND (1000.0f / 60.0f)

local SDL_Window*          global_game_window           = nullptr;
local SDL_GameController*  global_controller_devices[4] = {};
local Game                 game                         = {};
static SDL_Haptic*         global_haptic_devices[4]     = {};

local bool SCREEN_IS_FULLSCREEN = false;
local bool LAST_SCREEN_IS_FULLSCREEN = false;

local u32 SCREEN_WIDTH                    = 0;
local u32 SCREEN_HEIGHT                   = 0;
local u32 REAL_SCREEN_WIDTH               = 1024;
local u32 REAL_SCREEN_HEIGHT              = 768;
const u32 ENGINE_BASE_VERTICAL_RESOLUTION = 480; // scaling up to 480p resolution

local int last_resolution_w = REAL_SCREEN_WIDTH;
local int last_resolution_h = REAL_SCREEN_HEIGHT;

local V2 get_scaled_screen_resolution(V2 base_resolution) {
    f32 scale_factor = base_resolution.y / ENGINE_BASE_VERTICAL_RESOLUTION;

    base_resolution.x /= scale_factor;
    base_resolution.x = ceilf(base_resolution.x);

    return V2(base_resolution.x, ENGINE_BASE_VERTICAL_RESOLUTION);
}

local void initialize_framebuffer(void) {
    V2 framebuffer_resolution = get_scaled_screen_resolution(V2(REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT));

    // /* I know these sound like constants. They really aren't... */
    u32 last_screen_width               = SCREEN_WIDTH;
    u32 last_screen_height              = SCREEN_HEIGHT;
    SCREEN_WIDTH                        = framebuffer_resolution.x;
    SCREEN_HEIGHT                       = ENGINE_BASE_VERTICAL_RESOLUTION;
    Global_Engine()->real_screen_width  = REAL_SCREEN_WIDTH;
    Global_Engine()->real_screen_height = REAL_SCREEN_HEIGHT;
    Global_Engine()->virtual_screen_width  = SCREEN_WIDTH;
    Global_Engine()->virtual_screen_height = SCREEN_HEIGHT;
    Global_Engine()->fullscreen = SCREEN_IS_FULLSCREEN;

    if (!global_graphics_driver->is_initialized()) {
        global_graphics_driver->initialize(global_game_window, framebuffer_resolution.x, framebuffer_resolution.y);
        _debugprintf("first time initialize");
    }
    _debugprintf("Update backbuffer");
#if 0
    if (last_screen_width == SCREEN_WIDTH && last_screen_height == SCREEN_HEIGHT) {
        _debugprintf("Framebuffer did not change resolutions. No change needed.");
    } else {
        _debugprintf("framebuffer resolution is: (%d, %d) vs (%d, %d) real resolution", SCREEN_WIDTH, SCREEN_HEIGHT, REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT);
        global_graphics_driver->initialize(global_game_window, framebuffer_resolution.x, framebuffer_resolution.y);
    }
#else
    global_graphics_driver->initialize_backbuffer(framebuffer_resolution);
#endif
}

local const f32 r16by9Ratio  = 16/9.0f;
local const f32 r16by10Ratio = 16/10.0f;
local const f32 r4by3Ratio   = 4/3.0f;

local void set_window_transparency(f32 transparency) {
    SDL_SetWindowOpacity(global_game_window, transparency);
}

local void close_all_controllers(void) {
    for (unsigned controller_index = 0; controller_index < array_count(global_controller_devices); ++controller_index) {
        if (global_controller_devices[controller_index]) {
            SDL_GameControllerClose(global_controller_devices[controller_index]);
        }
    }
}

local void poll_and_register_controllers(void) {
    for (unsigned controller_index = 0; controller_index < array_count(global_controller_devices); ++controller_index) {
        SDL_GameController* controller = global_controller_devices[controller_index];

        if (controller) {
            if (!SDL_GameControllerGetAttached(controller)) {
                SDL_GameControllerClose(controller);
                global_controller_devices[controller_index] = NULL;
                _debugprintf("Controller at %d is bad", controller_index);
            }
        } else {
            if (SDL_IsGameController(controller_index)) {
                global_controller_devices[controller_index] = SDL_GameControllerOpen(controller_index);
                _debugprintf("Opened controller index %d (%s)\n", controller_index, SDL_GameControllerNameForIndex(controller_index));
            }
        }
    }
}

void register_controller_down(s32 which, s32 button) {
    SDL_GameController* controller = global_controller_devices[which];

    if (controller) {
        for (s32 controller_index = 0; controller_index < 4; ++controller_index) {
            game_controller* controller = Input::get_game_controller(controller_index);
            if (controller->_internal_controller_handle == controller) {
                controller->buttons_that_received_events[button] = true;
                return;
            }
        }
    }
}


local bool _use_controller_rumble = true;
void controller_rumble_set(bool v) {
    _use_controller_rumble = v;
}

void controller_rumble(struct game_controller* controller, f32 x_magnitude, f32 y_magnitude, u32 ms) {
    if (!_use_controller_rumble)
        return;

    SDL_GameController* sdl_controller = (SDL_GameController*)controller->_internal_controller_handle;
    if (!sdl_controller)
        return;

    x_magnitude                        = clamp<f32>(x_magnitude, 0, 1);
    y_magnitude                        = clamp<f32>(y_magnitude, 0, 1);
    SDL_GameControllerRumble(sdl_controller, (0xFFFF * x_magnitude), (0xFFFF * y_magnitude), ms);
}

void controller_set_led(struct game_controller* controller, u8 r, u8 g, u8 b) {
    if (!controller)
        return;

    SDL_GameController* sdl_controller = (SDL_GameController*)controller->_internal_controller_handle;

    if (!SDL_GameControllerHasLED(sdl_controller))
        return;

    SDL_GameControllerSetLED(sdl_controller, r, g, b);
}

local void update_all_controller_inputs(void) {
    for (unsigned controller_index = 0; controller_index < array_count(global_controller_devices); ++controller_index) {
        SDL_GameController* controller = global_controller_devices[controller_index];

        if (!controller) {
            continue;
        }

        struct game_controller* gamepad = Input::get_gamepad(controller_index);
        gamepad->_internal_controller_handle = controller;

        {
            gamepad->triggers.left  = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / (32767.0f);
            gamepad->triggers.right = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / (32767.0f);
        }

        {
            gamepad->buttons[BUTTON_RB] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            gamepad->buttons[BUTTON_LB] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        }

        {
            gamepad->buttons[BUTTON_A] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
            gamepad->buttons[BUTTON_B] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
            gamepad->buttons[BUTTON_X] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
            gamepad->buttons[BUTTON_Y] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);
        }

        {
            gamepad->buttons[DPAD_UP]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            gamepad->buttons[DPAD_DOWN]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            gamepad->buttons[DPAD_LEFT]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT); 
            gamepad->buttons[DPAD_RIGHT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT); 
        }

        {
            gamepad->buttons[BUTTON_RS] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
            gamepad->buttons[BUTTON_LS]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        }

        {
            gamepad->buttons[BUTTON_START] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START) || SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD);
            gamepad->buttons[BUTTON_BACK]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);
        }

        {
            {
                f32 axis_x = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) / (32767.0f);
                f32 axis_y = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) / (32767.0f);

                const f32 DEADZONE_X = 0.05;
                const f32 DEADZONE_Y = 0.05;
                if (fabs(axis_x) < DEADZONE_X) axis_x = 0;
                if (fabs(axis_y) < DEADZONE_Y) axis_y = 0;

                gamepad->left_stick.axes[0] = axis_x;
                gamepad->left_stick.axes[1] = axis_y;
            }

            {
                f32 axis_x = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) / (32767.0f);
                f32 axis_y = (f32)SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) / (32767.0f);

                const f32 DEADZONE_X = 0.25;
                const f32 DEADZONE_Y = 0.25;
                if (fabs(axis_x) < DEADZONE_X) axis_x = 0;
                if (fabs(axis_y) < DEADZONE_Y) axis_y = 0;
                
                gamepad->right_stick.axes[0] = axis_x;
                gamepad->right_stick.axes[1] = axis_y;
            }
        }
    }
}

// NOTE(jerry): unused, events are set elsewhere.
local void change_resolution(s32 new_resolution_x, s32 new_resolution_y) {
    REAL_SCREEN_WIDTH  = new_resolution_x;
    REAL_SCREEN_HEIGHT = new_resolution_y;
    if (SCREEN_IS_FULLSCREEN) {
      //// todo
      SDL_DisplayMode mode;
      mode.format = SDL_PIXELFORMAT_BGR24;
      mode.w = new_resolution_x;
      mode.h = new_resolution_y;

      SDL_SetWindowDisplayMode(global_game_window,&mode);
    }
    else {
      SDL_SetWindowSize(global_game_window, new_resolution_x, new_resolution_y);
    }
    Global_Engine()->real_screen_width = new_resolution_x;
    Global_Engine()->real_screen_height = new_resolution_y;
}

local void set_fullscreen(bool v, bool f=false) {
    LAST_SCREEN_IS_FULLSCREEN = SCREEN_IS_FULLSCREEN;
    SCREEN_IS_FULLSCREEN = v;

    if (LAST_SCREEN_IS_FULLSCREEN != SCREEN_IS_FULLSCREEN || f) {
        if (SCREEN_IS_FULLSCREEN) {
          // exclusive fullscreen now.
          /*
            NOTE(jerry):
            for opengl specifically, it seems to really... really dislike the fullscreen circus
            and I can't even actually fake the fullscreen myself either! If I set it to exactly mode.h instead of
            mode.h - 1, there's a weird flickering issue on Intel UHD ONLY?!
          */
#ifdef UNUSUAL_INTEL_UHD_OPENGL_FULLSCREEN_FIX
          {
            last_resolution_w = REAL_SCREEN_WIDTH;
            last_resolution_h = REAL_SCREEN_HEIGHT;
            SDL_SetWindowBordered(global_game_window, SDL_FALSE);
            SDL_DisplayMode mode;
            int index = SDL_GetWindowDisplayIndex(global_game_window);
            SDL_GetCurrentDisplayMode(index, &mode);
            SDL_SetWindowSize(global_game_window, mode.w, mode.h-1);
          }
          SDL_SetWindowPosition(global_game_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
#else
          SDL_SetWindowFullscreen(global_game_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
          {
            SDL_DisplayMode mode;
            int index = SDL_GetWindowDisplayIndex(global_game_window);
            SDL_GetCurrentDisplayMode(index, &mode);
            //SDL_SetWindowSize(global_game_window, mode.w, mode.h);
            _debugprintf("set display mode %d, %d", mode.w, mode.h);
            SDL_SetWindowDisplayMode(global_game_window, &mode);
          }
#endif
          SDL_SetWindowAlwaysOnTop(global_game_window, SDL_TRUE);
        } else {
            SDL_SetWindowFullscreen(global_game_window, SDL_FALSE);
            SDL_SetWindowAlwaysOnTop(global_game_window, SDL_FALSE);
            SDL_SetWindowBordered(global_game_window, SDL_TRUE);
            SDL_SetWindowSize(global_game_window, last_resolution_w, last_resolution_h);
            _debugprintf("resolution change, %d, %d", last_resolution_w, last_resolution_h);
            SDL_SetWindowPosition(global_game_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
    }

    Global_Engine()->fullscreen = SCREEN_IS_FULLSCREEN;
}

local void toggle_fullscreen(void) {
    if (SCREEN_IS_FULLSCREEN) {
        set_fullscreen(false, true);
    } else {
        set_fullscreen(true, true);
    }
}

void handle_sdl_events(void) {
    {
        SDL_Event current_event;

        f32 resolution_scale_x = (f32)REAL_SCREEN_WIDTH / SCREEN_WIDTH;
        f32 resolution_scale_y = (f32)REAL_SCREEN_HEIGHT / SCREEN_HEIGHT;
        // f32 resolution_scale_x = 1;
        // f32 resolution_scale_y = 1;

        while (SDL_PollEvent(&current_event)) {
            switch (current_event.type) {
                case SDL_WINDOWEVENT: {
                    switch (current_event.window.event) {
                        case SDL_WINDOWEVENT_RESIZED:
                        case SDL_WINDOWEVENT_SIZE_CHANGED: {
                            s32 new_width  = current_event.window.data1;
                            s32 new_height = current_event.window.data2;

                            _debugprintf("Size change event... Reevaluating framebuffers (%d, %d)", new_width, new_height);
                            if (REAL_SCREEN_WIDTH != new_width ||
                              REAL_SCREEN_HEIGHT != new_height) {
                              REAL_SCREEN_WIDTH = new_width;
                              REAL_SCREEN_HEIGHT = new_height;
                              initialize_framebuffer();
                              SDL_SetWindowPosition(global_game_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                            }
                        } break;
                        case SDL_WINDOWEVENT_FOCUS_LOST:
                        case SDL_WINDOWEVENT_LEAVE: {
                          _debugprintf("lost focus?");
                        } break;
                    }
                } break;

                case SDL_QUIT: {
                    Global_Engine()->running = false;
                } break;
                        
                case SDL_KEYUP:
                case SDL_KEYDOWN: {
                    bool is_keydown = (current_event.type == SDL_KEYDOWN);
                    if (is_keydown) {
                        Input::register_key_down(translate_sdl_scancode(current_event.key.keysym.scancode));
                    } else {
                        Input::register_key_up(translate_sdl_scancode(current_event.key.keysym.scancode));
                    }
                } break;

                case SDL_MOUSEWHEEL: {
                    Input::register_mouse_wheel(current_event.wheel.x, current_event.wheel.y);
                } break;
                case SDL_MOUSEMOTION: {
                    Input::register_mouse_position(current_event.motion.x / resolution_scale_x, current_event.motion.y / resolution_scale_y);
                } break;
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    s32 button_id = 0;
                    switch (current_event.button.button) {
                        case SDL_BUTTON_LEFT: {
                            button_id = MOUSE_BUTTON_LEFT;
                        } break;
                        case SDL_BUTTON_MIDDLE: {
                            button_id = MOUSE_BUTTON_MIDDLE;
                        } break;
                        case SDL_BUTTON_RIGHT: {
                            button_id = MOUSE_BUTTON_RIGHT; 
                        } break;
                    }

                    Input::register_mouse_position(current_event.button.x/resolution_scale_x, current_event.button.y/resolution_scale_y);
                    Input::register_mouse_button(button_id, current_event.button.state == SDL_PRESSED);
                } break;

                case SDL_CONTROLLERDEVICEREMOVED:
                case SDL_CONTROLLERDEVICEADDED: {
                    poll_and_register_controllers();
                } break;

                case SDL_CONTROLLERBUTTONUP:
                case SDL_CONTROLLERBUTTONDOWN: {
                    s32 controller_id  = current_event.cbutton.which;
                    u8  button_pressed = current_event.cbutton.button;
                    u8  button_state   = current_event.cbutton.state;

                    if (button_state == SDL_PRESSED) {
                        register_controller_down(controller_id, button_pressed);
                    }
                } break;

                case SDL_TEXTINPUT: {
                    char* input_text = current_event.text.text;
                    size_t input_length = strlen(input_text);
                    Input::send_text_input(input_text, input_length);
                } break;

                default: {} break;
            }
        }
    }
}

void initialize() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, 0);
    SDL_GameControllerAddMappingsFromFile("./gamecontrollerdb.txt");

    Audio::initialize();

    u32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    // initialize game_preferences structure here.
    {
        auto& preferences = game.preferences;
        preferences.width = REAL_SCREEN_WIDTH;
        preferences.height = REAL_SCREEN_HEIGHT;
        // NOTE: not populated yet...
        preferences.resolution_option_index = Graphics_Driver::find_index_of_resolution(preferences.width, preferences.height);
        preferences.music_volume = 0.5f;
        preferences.sound_volume = 0.5f;
        preferences.renderer_type = GRAPHICS_DEVICE_D3D11;
        preferences.fullscreen   = SCREEN_IS_FULLSCREEN;
        preferences.controller_vibration = true;
    }

    // load preferences file primarily.
    game.handle_preferences();

    /*
     * NOTE: the initialization regarding preferences in this engine is a bit weird, and the main thing
     * is because I can switch graphics drivers at runtime. Some things are also to prevent undefined state
     * from stale pointers and other weird problems due to some architectural mistakes I made pretty early on, and are
     * annoying for me to go back and correct, especially since this is otherwise pretty stable.
     */
#if 1
     SCREEN_IS_FULLSCREEN = game.preferences.fullscreen;
     Global_Engine()->fullscreen = SCREEN_IS_FULLSCREEN;

     if (Global_Engine()->fullscreen) {
       SDL_DisplayMode mode;
       SDL_GetCurrentDisplayMode(0, &mode); // assume monitor 0
       game.preferences.width = mode.w;
       game.preferences.height = mode.h;
     }
     REAL_SCREEN_WIDTH = game.preferences.width;
     REAL_SCREEN_HEIGHT = game.preferences.height;
     last_resolution_w = REAL_SCREEN_WIDTH;
     last_resolution_h = REAL_SCREEN_HEIGHT;
     Global_Engine()->real_screen_width  = REAL_SCREEN_WIDTH;
     Global_Engine()->real_screen_height = REAL_SCREEN_HEIGHT;
     Audio::set_volume_sound(game.preferences.sound_volume);
     Audio::set_volume_music(game.preferences.music_volume);
#endif

    global_game_window = SDL_CreateWindow("SolStorm",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          REAL_SCREEN_WIDTH,
                                          REAL_SCREEN_HEIGHT,
                                          flags);
    set_graphics_device(game.preferences.renderer_type);
    Thread_Pool::initialize();

    // NOTE(jerry): fullscreen adjustments.
    if (Global_Engine()->fullscreen) {
      set_fullscreen(true, true);
       update_preferences(&game.temp_preferences, &game.preferences);
    }

    Graphics_Driver::populate_display_mode_list(global_game_window); // update internal list of display modes.

    // properly populate the resolution_option_index field
    game.preferences.resolution_option_index = Graphics_Driver::find_index_of_resolution(game.preferences.width, game.preferences.height);
    update_preferences(&game.temp_preferences, &game.preferences);

#ifndef NO_FANCY_FADEIN_INTRO
    set_window_transparency(0);
#endif
    SDL_ShowWindow(global_game_window);
    game.init(global_graphics_driver);
    Input::initialize();
    Discord_Integration::initialize();
}

void update_preferences(Game_Preferences* a, Game_Preferences* b) {
    *a = *b;

    // NOTE: only happens at "fresh" start.
    if (b->resolution_option_index == -1)
        return;

    auto display_mode = Graphics_Driver::get_display_modes()[b->resolution_option_index];
    a->width          = display_mode.width;
    a->height         = display_mode.height;
}

void actually_confirm_and_update_preferences(Game_Preferences* preferences, Game_Resources* resources) {
    if (!queued_preference_update) {
        return;
    }
    _debugprintf("Updating preferences");
    set_fullscreen(preferences->fullscreen);
    set_graphics_device(preferences->renderer_type);

    // change_resolution(preferences->width, preferences->height);

    Audio::set_volume_sound(preferences->sound_volume);
    Audio::set_volume_music(preferences->music_volume);


    if (preferences->fullscreen) {
      SDL_DisplayMode mode;
      int index = SDL_GetWindowDisplayIndex(global_game_window);
      SDL_GetCurrentDisplayMode(index, &mode);
      _debugprintf("window display mode: %d, %d", mode.w, mode.h);
      preferences->width = mode.w;
      preferences->height = mode.h;
    }
    else {
      last_resolution_w = preferences->width;
      last_resolution_h = preferences->height;
      global_graphics_driver->change_resolution(preferences->width, preferences->height);
      _debugprintf("not windowed, changing window res to %d, %d", preferences->width, preferences->height);
    }

    preferences->resolution_option_index = global_graphics_driver->find_index_of_resolution(preferences->width, preferences->height);
    _use_controller_rumble = preferences->controller_vibration;
    queued_preference_update = false;

    game.on_resolution_change(preferences->width, preferences->height);
    initialize_framebuffer();
}

void confirm_preferences(Game_Preferences* preferences, Game_Resources* resources) {
    queued_preference_update = true;
}

/*
 * These are my first LUA proving grounds.
 */
bool save_preferences_to_disk(Game_Preferences* preferences, string path) {
    _debugprintf("Hi, preferences are not written to disk yet.");
    FILE* f = fopen(path.data, "wb+");
    {
        fprintf(f, "---- Preferences generated from the game\n");
        fprintf(f, "width = %d\n",    preferences->width);
        fprintf(f, "height = %d\n",    preferences->height);
        fprintf(f, "music_volume = %3.3f\n", preferences->music_volume);
        fprintf(f, "sound_volume = %3.3f\n", preferences->sound_volume);
        fprintf(f, "renderer = %d\n", preferences->renderer_type);
        fprintf(f, "fullscreen = %s\n",    ((s32)preferences->fullscreen) ? "true" : "false");
        fprintf(f, "controller_vibration = %s\n",    ((s32)preferences->controller_vibration) ? "true" : "false");
        // controls are separate.
    } fclose(f);
    return true;
}

// TODO: sandbox by removing loops and similar keywords that cause danger.
bool load_preferences_from_disk(Game_Preferences* preferences, string path) {
    lua_State* L = luaL_newstate();

    if (luaL_dofile(L, path.data) != 0) {
        _debugprintf("Could not exec %.*s. Hopefully it doesn't exist.", path.length, path.data);
        lua_close(L);
        return false;
    }

    {
        int t = lua_getglobal(L, "width");
        if (t != LUA_TNONE && t != LUA_TNIL)
          preferences->width = lua_tointeger(L, -1);
    }
    {
        int t = lua_getglobal(L, "height");
        if (t != LUA_TNONE && t != LUA_TNIL)
           preferences->height = lua_tointeger(L, -1);
    }
    {
        int t = lua_getglobal(L, "music_volume");
        if (t != LUA_TNONE && t != LUA_TNIL)
            preferences->music_volume = lua_tonumber(L, -1);
    }

    {
        int t = lua_getglobal(L, "sound_volume");
        if (t != LUA_TNONE && t != LUA_TNIL)
            preferences->sound_volume = lua_tonumber(L, -1);
    }
    {
        int t = lua_getglobal(L, "fullscreen");
        if (t != LUA_TNONE && t != LUA_TNIL)
            preferences->fullscreen = lua_toboolean(L, -1);
    }
    {
        int t = lua_getglobal(L, "renderer");
        if (t != LUA_TNONE && t != LUA_TNIL) {
            preferences->renderer_type = lua_tonumber(L, -1);
        }

        if (preferences->renderer_type == GRAPHICS_DEVICE_NULL) {
            preferences->renderer_type = GRAPHICS_DEVICE_D3D11;
        }
    }
    {
        int t = lua_getglobal(L, "controller_vibration");
        if (t != LUA_TNONE && t != LUA_TNIL)
            preferences->controller_vibration = lua_toboolean(L, -1);
    }

    preferences->resolution_option_index = Graphics_Driver::find_index_of_resolution(preferences->width, preferences->height);
    lua_close(L);
    return true;
}

void deinitialize() {
    game.deinit();
    Thread_Pool::synchronize_and_finish();
    Global_Engine()->main_arena.finish();
    Global_Engine()->scratch_arena.finish();
    close_all_controllers();

    global_graphics_driver->finish();
    SDL_DestroyWindow(global_game_window);

    Audio::deinitialize();
    SDL_Quit();
    Discord_Integration::deinitialize();
}

void engine_main_loop() {
    char window_name_title_buffer[256] = {};

    Input::begin_input_frame();
    {
        handle_sdl_events();
        update_all_controller_inputs();

        local bool _did_window_intro_fade_in   = false;
        local f32  _window_intro_fade_in_timer = 0;

#ifdef NO_FANCY_FADEIN_INTRO
        _did_window_intro_fade_in = true;
#endif

        // NOTE(jerry): platform layer is a mess, so this is hard to move around.
        // alt enter is not reliable due to so much desync with video settings everywhere
#if 0
        if (Input::is_key_down(KEY_ALT) && Input::is_key_pressed(KEY_RETURN)) {
          Input::register_key_up(KEY_RETURN);
          Input::register_key_up(KEY_ALT);
          // write this in to be consistent with what is currently seen.
          queued_preference_update = true;
          game.preferences.fullscreen = (game.temp_preferences.fullscreen ^= true);
        }
#endif

        if (!_did_window_intro_fade_in) {
            const f32 MAX_INTRO_FADE_IN_TIMER = 0.4;
            f32 effective_frametime = Global_Engine()->last_elapsed_delta_time;
            if (effective_frametime >= 1.0/45.0f) {
                effective_frametime = 1.0/45.0f;
            }
            _window_intro_fade_in_timer += effective_frametime;

            f32 alpha = _window_intro_fade_in_timer / MAX_INTRO_FADE_IN_TIMER;
            if (alpha > 1) alpha = 1;

            set_window_transparency(alpha);

            if (_window_intro_fade_in_timer >= MAX_INTRO_FADE_IN_TIMER) {
                _did_window_intro_fade_in = true;
            }
        } else {
            // lightmask_buffer_clear(&global_lightmask_buffer);
            game.update_and_render(global_graphics_driver, Global_Engine()->last_elapsed_delta_time);
        }
    }
    Input::end_input_frame();
    global_graphics_driver->swap_and_present();

    add_frametime_sample(Global_Engine()->last_elapsed_delta_time);

    {
        f32 average_frametime = get_average_frametime();
        // snprintf(window_name_title_buffer, 256, "BH(%s) - instant fps: %d, (%f ms)", _build_flags, (int)(1.0/(f32)average_frametime), average_frametime);
        SDL_SetWindowTitle(
            global_game_window,
            format_temp("BH(%s) - fps: %d, (%f ms) RENDER: %s", _build_flags, (int)(1.0/(f32)average_frametime), average_frametime, global_graphics_driver->get_name())
        );
    }

    Global_Engine()->scratch_arena.clear();
}

local void set_graphics_device(s32 id) {
    if (last_graphics_device_id != GRAPHICS_DEVICE_NULL) {
        return;
    }
    last_graphics_device_id = id;

    if (global_graphics_driver) {
        global_graphics_driver->finish();
    }

    switch (id) {
        case GRAPHICS_DEVICE_SOFTWARE: {
            _debugprintf("Using software graphics device");
            global_graphics_driver = &global_software_renderer_driver;
        } break;
            /*
             * NOTE(jerry):
             *
             * If not on Windows, D3D11 will fallthrough to the OpenGL renderer
             * but will still allow choosing D3D11 on windows otherwise.
             */
        case GRAPHICS_DEVICE_D3D11:
#ifdef _WIN32
        {
            _debugprintf("Using DirectX11 graphics device");
            global_graphics_driver = &global_direct3d11_renderer_driver;
        }
        break;
#endif
        case GRAPHICS_DEVICE_OPENGL: {
            _debugprintf("Using opengl graphics device");
            global_graphics_driver = &global_opengl_renderer_driver;
        } break;
    }

    // reinitialize game assets (reuploading stuff basically)
    initialize_framebuffer();
    // global_graphics_driver->initialize(global_game_window, SCREEN_WIDTH, SCREEN_HEIGHT);
    Global_Engine()->driver = global_graphics_driver;
    game.init_graphics_resources(global_graphics_driver);
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetProcessDPIAware();
#endif
    initialize();

    Discord_Integration::update_activity(discord_activity());
    {
        string s = get_preference_directory(string_null, string_literal("Soulstorm"));
        _debugprintf("Preference Writing Path: %.*s", _string_unwrap(s));
    }
    while (Global_Engine()->running) {
        u32 start_frame_time = SDL_GetTicks();
        engine_main_loop();

        Global_Engine()->last_elapsed_delta_time = (SDL_GetTicks() - start_frame_time) / 1000.0f;

#ifdef TARGET_TICKS_PER_SECOND
        if (SDL_GetTicks()-start_frame_time < TARGET_TICKS_PER_SECOND) {
            int sleep_time = TARGET_TICKS_PER_SECOND - (SDL_GetTicks() - start_frame_time);
            SDL_Delay(sleep_time);
        }
#endif

        Global_Engine()->last_elapsed_delta_time = (SDL_GetTicks() - start_frame_time) / 1000.0f;
        Global_Engine()->global_elapsed_time += Global_Engine()->last_elapsed_delta_time;

        actually_confirm_and_update_preferences(
            &game.preferences,
            game.resources
        );
        Discord_Integration::per_frame_update(Global_Engine()->last_elapsed_delta_time);
    }
    deinitialize();
    return 0; 
}
