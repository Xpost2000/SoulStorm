#include "graphics_driver.h"
#include <SDL2/SDL.h>

Resolution_Mode Graphics_Driver::display_modes[128] = {};
int             Graphics_Driver::display_mode_count = 0;

s32 Graphics_Driver::find_index_of_resolution(s32 w, s32 h) {
    for (int i = 0; i < display_mode_count; ++i) {
        auto& dm = display_modes[i];

        if (dm.width == w && dm.height == h) {
            return i;
        }
    }

    return -1;
}

bool Graphics_Driver::already_have_resolution(s32 w, s32 h) {
    return find_index_of_resolution(w, h) != -1;
}

void Graphics_Driver::populate_display_mode_list(SDL_Window* game_window) {
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
            !Graphics_Driver::already_have_resolution(display_mode.w, display_mode.h)) {
            auto& dm = display_modes[display_mode_count++];
            dm.width  = display_mode.w;
            dm.height = display_mode.h;
        }
    }
}

Slice<Resolution_Mode> Graphics_Driver::get_display_modes(void) {
    return make_slice<Resolution_Mode>((Resolution_Mode*)display_modes, display_mode_count);
}

void Graphics_Driver::change_resolution(s32 new_resolution_x, s32 new_resolution_y) {
    SDL_SetWindowSize(game_window, new_resolution_x, new_resolution_y);
}

void Graphics_Driver::unload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("Default unload_texture is NOP");
}

void Graphics_Driver::unload_font(struct graphics_assets* assets, font_id image) {
    _debugprintf("Default unload_font is NOP");
}

const char* Graphics_Driver::get_name(void) {
    return "(Generic Graphics Driver)";
}
