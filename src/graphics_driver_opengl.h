#ifndef GRAPHICS_OPENGL_DRIVER_H
#define GRAPHICS_OPENGL_DRIVER_H
#include "graphics_driver.h"

#ifdef _WIN32
#include <windows.h>
#undef near
#undef far
#endif
#include <glad/glad.h>
#include <SDL2/SDL.h>

#define MAX_OPENGL_SUPPORTED_TEXTURES 512
class OpenGL_Graphics_Driver : public Graphics_Driver {
public:
    void initialize(SDL_Window* window, int width, int height);
    void initialize_backbuffer(V2 resolution);
    void swap_and_present();
    void finish();
    void clear_color_buffer(color32u8 color);
    void consume_render_commands(struct render_commands* commands);
    V2   resolution();
    void upload_texture(struct graphics_assets* assets, image_id image);
    void upload_font(struct graphics_assets* assets, font_id font);
    void screenshot(char* where);
    const char* get_name(void);
private:
    SDL_GLContext context;

    int texture_count = 0;
    GLuint texture_ids[MAX_OPENGL_SUPPORTED_TEXTURES];
};

#endif
