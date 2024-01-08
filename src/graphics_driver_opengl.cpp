#include "graphics_driver_opengl.h"

local const char* default_vertex_shader_source = R"shader(
#version 330 core

layout(location = 0) in vec2 in_vertex_position;
layout(location = 1) in vec2 in_vertex_texcoords;
layout(location = 2) in vec4 in_vertex_colors;

out vec2 out_vertex_position;
out vec2 out_vertex_texcoords;
out vec4 out_vertex_colors;

// uniforms
// NOTE: column matrices
uniform mat4 u_projection_matrix;
uniform mat4 u_view_matrix;

void main() {
    gl_Position = u_projection_matrix * u_view_matrix * vec4(in_vertex_position.xy, 1.0, 1.0);

    out_vertex_position  = in_vertex_position;
    out_vertex_texcoords = in_vertex_texcoords;
    out_vertex_colors    = in_vertex_colors;
}
)shader";

local const char* default_pixel_shader_source = R"shader(
#version 330 core

in vec2 out_vertex_position;
in vec2 out_vertex_texcoords;
in vec4 out_vertex_colors;

out vec4 out_color;
void main() {
    out_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)shader";

// OpenGL specific helpers
// NOTE: since these are all internal, none of these should fail.
local GLuint opengl_build_shader(const char* shader_source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_source, 0);
    glCompileShader(shader);
    {
        GLint compile_status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
#ifndef RELEASE
        local char shader_info_buffer[4096];
        zero_memory(shader_info_buffer, sizeof(shader_info_buffer));
        glGetShaderInfoLog(shader, sizeof(shader_info_buffer), 0, shader_info_buffer);
        _debugprintf("GLSL compile log: %s", shader_info_buffer);
#endif
        assertion(compile_status == GL_TRUE && "Failure to compile shader?");
        _debugprintf("GLSL compile is successful");
    }
    return shader;
}

local GLuint opengl_build_shader_program(GLuint* shaders, unsigned int shader_count) {
    GLuint shader_program = glCreateProgram();

    for (unsigned shader_index = 0; shader_index < shader_count; ++shader_index) {
        glAttachShader(shader_program, shaders[shader_index]);
    }

    glLinkProgram(shader_program);
    {
        GLint link_status;
        glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);
#ifndef RELEASE
        local char linkage_buffer[4096];
        zero_memory(linkage_buffer, sizeof(linkage_buffer));
        glGetProgramInfoLog(shader_program, sizeof(linkage_buffer), 0, linkage_buffer);
        _debugprintf("GLSL link log: %s", linkage_buffer);
#endif
        assertion(link_status == GL_TRUE && "Failure to link shader program?");
        _debugprintf("GLSL link successful");
    }

    for (unsigned shader_index = 0; shader_index < shader_count; ++shader_index) {
        glDetachShader(shader_program, shaders[shader_index]);
        glDeleteShader(shaders[shader_index]);
    }

    return shader_program;
}
// End OpenGL specific helpers

void OpenGL_Graphics_Driver::initialize_default_rendering_state(void) {
    if (initialized_default_shader) {
        return;
    }
    GLuint shaders[] = {
    opengl_build_shader(default_vertex_shader_source, GL_VERTEX_SHADER),
    opengl_build_shader(default_pixel_shader_source, GL_FRAGMENT_SHADER)
    };
    default_shader_program = opengl_build_shader_program(shaders, array_count(shaders));
    initialized_default_shader = true;
}

void OpenGL_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    _debugprintf("OpenGL Initialize");
    
    if (game_window == nullptr) {
        game_window = window;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        context = SDL_GL_CreateContext(game_window);
        SDL_GL_MakeCurrent(game_window, context);
        assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) && "OpenGL functions could not be loaded. That's bad.");
    }

    initialize_backbuffer(V2(width, height));
}

void OpenGL_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("OpenGL Initialize Backbuffer");
    // I mean... OpenGL gives me a default framebuffer which is neat so
    // I just need to setup the matrix and shaders...
    // Which I only really need to do once since I'm not doing any render texture
    // or interesting tricks...
    initialize_default_rendering_state();
}

void OpenGL_Graphics_Driver::swap_and_present() {
    SDL_GL_SetSwapInterval(-1);
    SDL_GL_SwapWindow(game_window);
}

void OpenGL_Graphics_Driver::finish() {
    _debugprintf("OpenGL Driver Finish");
    unimplemented("Not done");
}

void OpenGL_Graphics_Driver::clear_color_buffer(color32u8 color) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void OpenGL_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
 //   unimplemented("Not done");
}

V2 OpenGL_Graphics_Driver::resolution() {
    return V2(640, 480);
}

void OpenGL_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("OpenGL driver upload texture is nop");
 //   unimplemented("Not done");
}

void OpenGL_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    _debugprintf("OpenGL driver upload font is nop");
 //   unimplemented("Not done");
}

void OpenGL_Graphics_Driver::screenshot(char* where) {
    _debugprintf("OpenGL driver screenshot is nop");
 //   unimplemented("Not done");
}


const char* OpenGL_Graphics_Driver::get_name(void) {
    return "(OpenGL 3.3)";
}
