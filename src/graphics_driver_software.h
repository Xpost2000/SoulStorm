#ifndef SOFTWARE_RENDERER_DRIVER_H
#define SOFTWARE_RENDERER_DRIVER_H

#include "software_renderer.h"
#include "graphics_driver.h"
/*
 * Jerry's handmade software renderer I guess
 * backed through writing into an SDL2 renderer.
 *
 * Technically the SDL2 renderer itself is actually hardware accelerated
 * (to blit faster, the software renderer on my end is just so I can write pixels
 *  myself :) )
 */
struct SDL_Renderer;
struct SDL_Texture;
class Software_Renderer_Graphics_Driver : public Graphics_Driver {
public:
    void initialize(SDL_Window* window, int width, int height);
    void initialize_backbuffer(V2 resolution);
    void swap_and_present();
    void finish();
    void clear_color_buffer(color32u8 color);
    void consume_render_commands(struct render_commands* commands);
    V2   resolution();

private:
    SDL_Window*          game_window          = nullptr;
    SDL_Renderer*        game_sdl_renderer    = nullptr;
    SDL_Texture*         game_texture_surface = nullptr;
    software_framebuffer default_framebuffer  = {};
};

#endif
