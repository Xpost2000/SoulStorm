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
    void unload_texture(struct graphics_assets* assets, image_id image);
    // NOTE: should remove from API, since this isn't really useful...
    // void unload_font(struct graphics_assets* assets, font_id image);
    void screenshot(char* where);
    const char* get_name(void);
private:
    // Batching State
    // End Batching State
    void render_command_draw_quad(const render_command& rc);
    void render_command_draw_image(const render_command& rc);
    void render_command_draw_line(const render_command& rc);
    void render_command_draw_text(const render_command& rc);
    void render_command_draw_set_scissor(const render_command& rc);
    void render_command_draw_clear_scissor(const render_command& rc);
    void flush_and_render_quads(void);
    void flush_and_render_lines(void);

    V2 real_resolution;
    V2 virtual_resolution;

    void initialize_default_rendering_state(void);
    bool find_first_free_texture_id(unsigned int* result);

    SDL_GLContext context;
    GLuint texture_ids[MAX_OPENGL_SUPPORTED_TEXTURES];
    GLuint white_pixel;

    // simple sprite shader program
    bool   initialized_default_shader = false;
    GLuint default_shader_program;
    GLuint projection_matrix_uniform_location;
    GLuint view_matrix_uniform_location;
};

#endif
