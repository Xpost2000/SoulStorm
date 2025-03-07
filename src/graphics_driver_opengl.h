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

#include "fixed_array.h"

#define MAX_OPENGL_SUPPORTED_TEXTURES 512
#define MAX_OPENGL_BATCHED_QUADS (4096)
#define MAX_OPENGL_VERTICES_FOR_QUAD_BUFFER (MAX_OPENGL_BATCHED_QUADS * 6)
struct OpenGL_Vertex_Format {
    V2         position;
    V2         texcoord;
    color32f32 color;
};
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
    void upload_image_buffer_to_gpu(struct image_buffer* image); // NOTE: also update driver_userdata field
    void push_render_quad_vertices(rectangle_f32 destination, rectangle_f32 source, color32f32 color, struct image_buffer* image, s32 angle = 0, s32 angle_y = 0, u32 flags = NO_FLAGS, V2 rotation_origin = V2(0,0));
    void set_blend_mode(u8 new_blend_mode);
    void set_texture_id(GLuint texture_id);
    // Batching State
    u8 current_blend_mode;
    GLuint current_texture_id;
    GLuint quad_vertex_buffer;
    GLuint quad_vertex_array_object;

    GLuint virtual_fbo = 0;
    GLuint virtual_rbo = 0;
    Fixed_Array<OpenGL_Vertex_Format> quad_vertices;
    // End Batching State
    void render_command_draw_quad(const render_command& rc);
    void render_command_draw_image(const render_command& rc);
    void render_command_draw_line(const render_command& rc);
    void render_command_draw_text(const render_command& rc);
    void render_command_draw_set_scissor(const render_command& rc);
    void render_command_draw_clear_scissor(const render_command& rc);
    void flush_and_render_quads(void);

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
    GLuint texture_sampler_uniform_location;
};

#endif
