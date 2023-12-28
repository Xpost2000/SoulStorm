/*
 * A new engine. A new project.
 *
 * Platform layer
 *
 * While there is a planned primary Windows path
 * (DirectX rendering), I am still going to use SDL2
 * simply because it has a nicer API for handling controller
 * support, and audio is an animal that I'm admittedly not experienced
 * at tackling (beyond some waveOut tests).
 *
 * Most of the other code is otherwise pretty independent.
 *
 */
#ifdef _WIN32
#include <Windows.h>
#endif

#include <SDL2/SDL.h>

#define JDR_COROUTINE_IMPLEMENTATION
#include "common.h"
#include "audio.h"
#include "input.h"
#include "sdl_scancode_table.h"
#include "memory_arena.h"
#include "v2.h"
#include "thread_pool.h"
#include "engine.h"
#include "game.h"
#include "game_state.h"

#include "graphics_driver.h"

#include "graphics_driver_software.h"
#include "graphics_driver_null.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

enum graphics_device_type {
    GRAPHICS_DEVICE_NULL,
    GRAPHICS_DEVICE_SOFTWARE,
    GRAPHICS_DEVICE_OPENGL,
    // GRAPHICS_DEVICE_D3D11,
};
local void set_graphics_device(s32 id);

Software_Renderer_Graphics_Driver global_software_renderer_driver;
Null_Graphics_Driver              global_null_renderer_driver;
Graphics_Driver* global_graphics_driver = &global_null_renderer_driver;

const char* _build_flags =
#ifdef USE_SIMD_OPTIMIZATIONS
    "using simd,"
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

local V2 get_scaled_screen_resolution(V2 base_resolution) {
    f32 scale_factor = base_resolution.y / ENGINE_BASE_VERTICAL_RESOLUTION;

    base_resolution.x /= scale_factor;
    base_resolution.x = ceilf(base_resolution.x);

    return V2(base_resolution.x, ENGINE_BASE_VERTICAL_RESOLUTION);
}

local void initialize_framebuffer(void) {
    V2 framebuffer_resolution = get_scaled_screen_resolution(V2(REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT));

    // /* I know these sound like constants. They really aren't... */
    SCREEN_WIDTH  = framebuffer_resolution.x;
    SCREEN_HEIGHT = ENGINE_BASE_VERTICAL_RESOLUTION;
    _debugprintf("framebuffer resolution is: (%d, %d) vs (%d, %d) real resolution", SCREEN_WIDTH, SCREEN_HEIGHT, REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT);

    global_graphics_driver->initialize(global_game_window, framebuffer_resolution.x, framebuffer_resolution.y);
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

                const f32 DEADZONE_X = 0.05;
                const f32 DEADZONE_Y = 0.05;
                if (fabs(axis_x) < DEADZONE_X) axis_x = 0;
                if (fabs(axis_y) < DEADZONE_Y) axis_y = 0;
                
                gamepad->right_stick.axes[0] = axis_x;
                gamepad->right_stick.axes[1] = axis_y;
            }
        }
    }
}

local void change_resolution(s32 new_resolution_x, s32 new_resolution_y) {
    SDL_SetWindowSize(global_game_window, new_resolution_x, new_resolution_y);
}

local void set_fullscreen(bool v) {
    LAST_SCREEN_IS_FULLSCREEN = SCREEN_IS_FULLSCREEN;
    SCREEN_IS_FULLSCREEN = v;

    if (LAST_SCREEN_IS_FULLSCREEN != SCREEN_IS_FULLSCREEN) {
        if (SCREEN_IS_FULLSCREEN) {
            // exclusive fullscreen now.
            SDL_SetWindowFullscreen(global_game_window, SDL_WINDOW_FULLSCREEN);
        } else {
            SDL_SetWindowFullscreen(global_game_window, 0);
        }
    }
}

local void toggle_fullscreen(void) {
    if (SCREEN_IS_FULLSCREEN) {
        set_fullscreen(false);
    } else {
        set_fullscreen(true);
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
                            _debugprintf("Size change event... Reevaluating framebuffers");
                            s32 new_width  = current_event.window.data1;
                            s32 new_height = current_event.window.data2;

                            REAL_SCREEN_WIDTH  = new_width;
                            REAL_SCREEN_HEIGHT = new_height;
                            initialize_framebuffer();
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

    u32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI;

    global_game_window = SDL_CreateWindow("SoulStorm",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          REAL_SCREEN_WIDTH,
                                          REAL_SCREEN_HEIGHT,
                                          flags);

    Thread_Pool::initialize();

#ifndef NO_FANCY_FADEIN_INTRO
    set_window_transparency(0);
#endif
    SDL_ShowWindow(global_game_window);

    // set_graphics_device(GRAPHICS_DEVICE_NULL);
    set_graphics_device(GRAPHICS_DEVICE_SOFTWARE);
    initialize_framebuffer();

    // initialize game_preferences structure here.
    {
        auto& preferences = game.preferences;
        preferences.width = REAL_SCREEN_WIDTH;
        preferences.height = REAL_SCREEN_HEIGHT;
        global_graphics_driver->get_display_modes(); // update internal list of display modes.
        preferences.resolution_option_index = global_graphics_driver->find_index_of_resolution(preferences.width, preferences.height);
        preferences.music_volume = 0.5f;
        preferences.sound_volume = 0.5f;
        preferences.fullscreen   = SCREEN_IS_FULLSCREEN;
        preferences.controller_vibration = true;
    }

    /*
     * NOTE: since graphics resources aren't initialized yet. It's generally safe
     *       to set_graphics_device one last time, from here. Any point after, I do not
     *       retain any resources, and the Graphics_Assets system does not reload stuff from
     *       disk!
     */
    game.init(global_graphics_driver);
    Input::initialize();
}

void update_preferences(Game_Preferences* a, Game_Preferences* b) {
    *a = *b;

    auto display_mode = global_graphics_driver->get_display_modes()[b->resolution_option_index];
    a->width          = display_mode.width;
    a->height         = display_mode.height;
}

void confirm_preferences(Game_Preferences* preferences, Game_Resources* resources) {
    REAL_SCREEN_WIDTH  = preferences->width;
    REAL_SCREEN_HEIGHT = preferences->height;
    set_fullscreen(preferences->fullscreen);

    // rebuild the back buffer or framebuffer again.
    initialize_framebuffer();
    global_graphics_driver->change_resolution(REAL_SCREEN_WIDTH, REAL_SCREEN_HEIGHT);

    Audio::set_volume_sound(preferences->sound_volume);
    Audio::set_volume_music(preferences->music_volume);

    global_graphics_driver->get_display_modes(); // update internal list of display modes.
    preferences->resolution_option_index = global_graphics_driver->find_index_of_resolution(preferences->width, preferences->height);
    _use_controller_rumble = preferences->controller_vibration;

    graphics_assets_update_graphics_driver(&resources->graphics_assets, global_graphics_driver);
}

/*
 * These are my first LUA proving grounds.
 */
bool save_preferences_to_disk(Game_Preferences* preferences, string path) {
    _debugprintf("Hi, preferences are not written to disk yet.");
    FILE* f = fopen(path.data, "wb+");
    {
        fprintf(f, "---- Preferences generated from the game\n");
        fprintf(f, "---- NOTE: while this is a lua file, you can't really use any thing special :P\n");
        fprintf(f, "width = %d\n",    preferences->width);
        fprintf(f, "height = %d\n",    preferences->height);
        fprintf(f, "music_volume = %3.3f\n", preferences->music_volume);
        fprintf(f, "sound_volume = %3.3f\n", preferences->sound_volume);
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
        lua_getglobal(L, "width");
        preferences->width = lua_tointeger(L, -1);
    }
    {
        lua_getglobal(L, "height");
        preferences->height = lua_tointeger(L, -1);
    }
    {
        lua_getglobal(L, "music_volume");
        preferences->music_volume = lua_tonumber(L, -1);
    }

    {
        lua_getglobal(L, "sound_volume");
        preferences->sound_volume = lua_tonumber(L, -1);
    }
    {
        lua_getglobal(L, "fullscreen");
        preferences->fullscreen = lua_toboolean(L, -1);
    }
    {
        lua_getglobal(L, "controller_vibration");
        preferences->controller_vibration = lua_toboolean(L, -1);
    }

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
    if (global_graphics_driver) {
        // clean the previous device resources
        global_graphics_driver->finish();
    }

    switch (id) {
        case GRAPHICS_DEVICE_SOFTWARE: {
            _debugprintf("Using software graphics device");
            global_graphics_driver = &global_software_renderer_driver;
        } break;
        case GRAPHICS_DEVICE_OPENGL: {
            _debugprintf("Using opengl graphics device");
            unimplemented("not here yet.");
        } break;
    }

    // reinitialize game assets (reuploading stuff basically)
    game.init_graphics_resources(global_graphics_driver);
    Global_Engine()->driver = global_graphics_driver;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetProcessDPIAware();
#endif
    initialize();
#define TARGET_TICKS_PER_SECOND (1000.0f / 60.0f)
    while (Global_Engine()->running) {
        u32 start_frame_time = SDL_GetTicks();
        engine_main_loop();

        Global_Engine()->last_elapsed_delta_time = (SDL_GetTicks() - start_frame_time) / 1000.0f;

        if (SDL_GetTicks()-start_frame_time < TARGET_TICKS_PER_SECOND) {
            int sleep_time = TARGET_TICKS_PER_SECOND - (SDL_GetTicks() - start_frame_time);
            SDL_Delay(sleep_time);
        }

        Global_Engine()->last_elapsed_delta_time = (SDL_GetTicks() - start_frame_time) / 1000.0f;
        Global_Engine()->global_elapsed_time += Global_Engine()->last_elapsed_delta_time;
    }
    deinitialize();
    return 0; 
}
