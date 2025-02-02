#include "graphics_driver_software.h"
#include "thread_pool.h"

#include <SDL2/SDL.h>

void Software_Renderer_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    game_window = window;
    if (game_sdl_renderer == nullptr) {
        _debugprintf("Hi, this is Jerry's software renderer driver. I am existing!");
        game_sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    initialize_backbuffer(V2(width, height));
}

void Software_Renderer_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("Reconstructing backbuffer.");
    Thread_Pool::synchronize_tasks(); // finish up any render command tasks hopefully...

    software_framebuffer_finish(&default_framebuffer);
    // lightmask_buffer_finish(&global_lightmask_buffer);

    V2 framebuffer_resolution = resolution;
    default_framebuffer = software_framebuffer_create(framebuffer_resolution.x, framebuffer_resolution.y);

    if (game_texture_surface) {
        SDL_DestroyTexture(game_texture_surface);
        game_texture_surface = NULL;
    }

    game_texture_surface = SDL_CreateTexture(game_sdl_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, default_framebuffer.width, default_framebuffer.height);
    initialized = true;
}

void Software_Renderer_Graphics_Driver::swap_and_present() {
    if (!game_texture_surface) {
        return;
    }

    {
        void* locked_pixel_region;
        s32   _pitch; unused(_pitch);
        SDL_LockTexture(game_texture_surface, 0, &locked_pixel_region, &_pitch);
        memory_copy(default_framebuffer.pixels, locked_pixel_region, default_framebuffer.width * default_framebuffer.height * sizeof(u32));
        SDL_UnlockTexture(game_texture_surface);
    }

    SDL_RenderCopy(game_sdl_renderer, game_texture_surface, 0, 0);
    SDL_RenderPresent(game_sdl_renderer);
}

void Software_Renderer_Graphics_Driver::finish() {
    software_framebuffer_finish(&default_framebuffer);
    SDL_DestroyTexture(game_texture_surface);
    SDL_DestroyRenderer(game_sdl_renderer);
    game_texture_surface = nullptr;
    game_sdl_renderer = nullptr;
}

void Software_Renderer_Graphics_Driver::clear_color_buffer(color32u8 color) {
    software_framebuffer_clear_buffer(&default_framebuffer, color);
}

void Software_Renderer_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    // refer to software_render_commands_implementation.cpp
    if (!game_texture_surface) {
        return;
    }
    software_framebuffer_render_commands(&default_framebuffer, commands); 
}

V2 Software_Renderer_Graphics_Driver::resolution() {
    return V2(default_framebuffer.width, default_framebuffer.height);
}

/*
 * NOTE: these are stubbed
 *
 * Since the graphics_assets already manages assets.
 *
 */
void Software_Renderer_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("Software Renderer upload texture is NOP");
}

void Software_Renderer_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    _debugprintf("Software Renderer upload font is NOP");
}

#include <stb_image_write.h>

void Software_Renderer_Graphics_Driver::screenshot(char* where) {
    stbi_write_png(where, default_framebuffer.width, default_framebuffer.height, 4, default_framebuffer.pixels, 4 * default_framebuffer.width);
    //stbi_write_bmp(where, default_framebuffer.width, default_framebuffer.height, 1, default_framebuffer.pixels);
  //  stbi_write_jpg(where, default_framebuffer.width, default_framebuffer.height, 0, default_framebuffer.pixels, 0);
}

const char* Software_Renderer_Graphics_Driver::get_name(void) {
    return "(Software Renderer)";
}
