#include "graphics_driver.h"
#include <SDL.h>

s32  Graphics_Driver::find_index_of_resolution(s32 w, s32 h) {
    for (int i = 0; i < display_mode_count; ++i) {
        auto& dm = display_modes[i];

        if (dm.width == w && dm.height == h)
            return i;
    }

    return -1;
}

bool Graphics_Driver::already_have_resolution(s32 w, s32 h) {
    return find_index_of_resolution(w, h) != -1;
}

Slice<Resolution_Mode> Graphics_Driver::get_display_modes() {
    // I hope enumeration is not slow...
    assertion(game_window != nullptr && "This shouldn't happen");
    s32 current_display_index = SDL_GetWindowDisplayIndex(game_window);
    s32 num_display_modes     = SDL_GetNumDisplayModes(current_display_index);

    num_display_modes  = max<s32>(0, array_count(display_modes));
    display_mode_count = 0;
    for (s32 index = 0; index < num_display_modes; ++index) {
        SDL_DisplayMode display_mode = {};
        SDL_GetDisplayMode(current_display_index, index, &display_mode);

        if (display_mode.w != 0 &&
            display_mode.h != 0 &&
            !already_have_resolution(display_mode.w, display_mode.h)) {
            auto& dm = display_modes[display_mode_count++];
            dm.width  = display_mode.w;
            dm.height = display_mode.h;
        }
    }

    return make_slice<Resolution_Mode>((Resolution_Mode*)display_modes, display_mode_count);
}

void Graphics_Driver::change_resolution(s32 new_resolution_x, s32 new_resolution_y) {
    SDL_SetWindowSize(game_window, new_resolution_x, new_resolution_y);
}