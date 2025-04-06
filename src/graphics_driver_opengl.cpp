#include "graphics_driver_opengl.h"

#include "engine.h"

#include <stb_image_write.h>

#include "game_ui.h"

local const char* gl_error_string(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: {
            return "GL_NO_ERROR";
        } break;
        case GL_INVALID_ENUM: {
            return "GL_INVALID_ENUM";
        } break;
        case GL_INVALID_VALUE: {
            return "GL_INVALID_VALUE";
        } break;
        case GL_INVALID_OPERATION: {
            return "GL_INVALID_OPERATION";
        } break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: {
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        } break;
        case GL_OUT_OF_MEMORY: {
            return "GL_OUT_OF_MEMORY";
        } break;
        case GL_STACK_UNDERFLOW: {
            return "GL_STACK_UNDERFLOW";
        } break;
        case GL_STACK_OVERFLOW: {
            return "GL_STACK_OVERFLOW";
        } break;
    }

    return "(unknown error)";
}

#define GL_CheckError(description)                                      \
    do {                                                                \
        auto error = glGetError();                                      \
        if (error != GL_NO_ERROR) {                                     \
            _debugprintf("[%s] Error Type: %s", description, gl_error_string(error)); \
            assertion(error == GL_NO_ERROR && "OpenGL error found, please fix."); \
        }                                                               \
    } while (0)

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
    out_vertex_texcoords = vec2(in_vertex_texcoords.x, in_vertex_texcoords.y);
    out_vertex_colors    = in_vertex_colors;
}
)shader";

local const char* default_pixel_shader_source = R"shader(
#version 330 core

in vec2 out_vertex_position;
in vec2 out_vertex_texcoords;
in vec4 out_vertex_colors;

uniform sampler2D sampler_texture;

out vec4 out_color;
void main() {
    out_color = texture2D(sampler_texture, out_vertex_texcoords) * out_vertex_colors;
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
        GL_CheckError("Shader compile status");
#ifndef RELEASE
        local char shader_info_buffer[4096];
        zero_memory(shader_info_buffer, sizeof(shader_info_buffer));
        glGetShaderInfoLog(shader, sizeof(shader_info_buffer), 0, shader_info_buffer);
        GL_CheckError("Shader info log");
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

local GLuint opengl_build_texture2d_object(struct image_buffer* image_buffer) {
    GLuint texture_object;
    glGenTextures(1, &texture_object);
    glBindTexture(GL_TEXTURE_2D, texture_object);
    {
        GLsizei image_width  = (image_buffer->pot_square_size)  ? image_buffer->pot_square_size : image_buffer->width;
        GLsizei image_height = (image_buffer->pot_square_size) ? image_buffer->pot_square_size : image_buffer->height;
        assertion(image_width > 0 && image_height > 0 && "It should be impossible to generate a texture like this...");
        glTexImage2D(
            GL_TEXTURE_2D,
            0, 
            GL_RGBA, 
            image_width, 
            image_height, 
            0, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            image_buffer->pixels
        );
        GL_CheckError("glTexImage2D called with parameters.");
        assertion(glGetError() == GL_NO_ERROR && "No OpenGL error");
        /*
            NOTE: technically I have the facilities to make my own rudimentary mipmaps (since I have a working rotozoomer in this engine...),
            but it's easier to use this, and in a 2D game it's unnoticable (I mean also indie tier 3D games as
            well...)
        */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

#if 0
        // hopefully do not need this.
        // since I have a pot resizer...
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif

        GL_CheckError("Set texture parameters for current texture");
        glGenerateMipmap(GL_TEXTURE_2D);
        GL_CheckError("Generate mipmap with glGenerateMipmap");
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture_object;
}
// End OpenGL specific helpers

void OpenGL_Graphics_Driver::initialize_default_rendering_state(void) {
    if (initialized_default_shader) {
        return;
    }
    zero_array(texture_ids);

    GLuint shaders[] = {
        opengl_build_shader(default_vertex_shader_source, GL_VERTEX_SHADER),
        opengl_build_shader(default_pixel_shader_source, GL_FRAGMENT_SHADER)
    };
    default_shader_program = opengl_build_shader_program(shaders, array_count(shaders));
    projection_matrix_uniform_location = glGetUniformLocation(default_shader_program, "u_projection_matrix");
    view_matrix_uniform_location       = glGetUniformLocation(default_shader_program, "u_view_matrix");
    texture_sampler_uniform_location   = glGetUniformLocation(default_shader_program, "sampler_texture");

    {
        GLfloat identity4x4[] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };

        glUseProgram(default_shader_program);
        {
            glUniformMatrix4fv(projection_matrix_uniform_location, 1, false, identity4x4);
            glUniformMatrix4fv(view_matrix_uniform_location, 1, false, identity4x4);
            glUniform1i(texture_sampler_uniform_location, 0); // only one texture unit needed.
        }
        glUseProgram(0);
    }

    // Generate white pixel so I can render textured and untextured quads in the same pass.
    {
        struct image_buffer white_pixel_image_buffer = image_buffer_create_blank(1, 1);
        *white_pixel_image_buffer.pixels_u32         = 0xFFFFFFFF;
        white_pixel                                  = opengl_build_texture2d_object(&white_pixel_image_buffer);
        image_buffer_free(&white_pixel_image_buffer);
    }

    // build vertex buffers
    {
        const int VERTEX_ATTRIB_INDEX      = 0;
        const int TEXCOORD_ATTRIB_INDEX    = 1;
        const int VERTEXCOLOR_ATTRIB_INDEX = 2;

        glGenBuffers(1, &quad_vertex_buffer);
        glGenVertexArrays(1, &quad_vertex_array_object);

        glBindVertexArray(quad_vertex_array_object);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);
        // NOTE: orphaned array buffer.
        glBufferData(GL_ARRAY_BUFFER, quad_vertices.capacity * sizeof(OpenGL_Vertex_Format), nullptr, GL_DYNAMIC_DRAW);
        GL_CheckError("Create vertex buffer data");
        glEnableVertexAttribArray(VERTEX_ATTRIB_INDEX);
        glEnableVertexAttribArray(TEXCOORD_ATTRIB_INDEX);
        glEnableVertexAttribArray(VERTEXCOLOR_ATTRIB_INDEX);
        glVertexAttribPointer(VERTEX_ATTRIB_INDEX,      2, GL_FLOAT, GL_FALSE, sizeof(OpenGL_Vertex_Format), (void*)offsetof(OpenGL_Vertex_Format, position));
        GL_CheckError("Vertex position attribute");
        glVertexAttribPointer(TEXCOORD_ATTRIB_INDEX,    2, GL_FLOAT, GL_FALSE, sizeof(OpenGL_Vertex_Format), (void*)offsetof(OpenGL_Vertex_Format, texcoord));
        GL_CheckError("Tex Coord position attribute");
        glVertexAttribPointer(VERTEXCOLOR_ATTRIB_INDEX, 4, GL_FLOAT, GL_FALSE, sizeof(OpenGL_Vertex_Format), (void*)offsetof(OpenGL_Vertex_Format, color));
        GL_CheckError("Vertex Color position attribute");
        glBindVertexArray(0);
    }
    initialized_default_shader = true;

    glEnable(GL_BLEND);
}

bool OpenGL_Graphics_Driver::find_first_free_texture_id(unsigned int* result) {
    for (unsigned index = 0; index < array_count(texture_ids); ++index) {
        if (texture_ids[index] == 0) {
            *result = index;
            return true;
        }
    }

    return false;
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

        {
            quad_vertices = Fixed_Array<OpenGL_Vertex_Format>(&Global_Engine()->main_arena, MAX_OPENGL_VERTICES_FOR_QUAD_BUFFER);
        }
    }

    initialize_backbuffer(V2(width, height));
    initialized = true;
}

void OpenGL_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("OpenGL Initialize Backbuffer");
    // I mean... OpenGL gives me a default framebuffer which is neat so
    // I just need to setup the matrix and shaders...
    // Which I only really need to do once since I'm not doing any render texture
    // or interesting tricks...
    initialize_default_rendering_state();
    
    real_resolution = V2(Global_Engine()->real_screen_width, Global_Engine()->real_screen_height);
    virtual_resolution = resolution;

    _debugprintf("Real resolution %d, %d", (s32)real_resolution.x, (s32)real_resolution.y);
    _debugprintf("virtual resolution %d, %d", (s32)virtual_resolution.x, (s32)virtual_resolution.y);
    {
      if (virtual_fbo) {
        glDeleteFramebuffers(1, &virtual_fbo);
        virtual_fbo = 0;
      }

      if (virtual_rbo) {
        glDeleteRenderbuffers(1, &virtual_rbo);
        virtual_rbo = 0;
      }

      glGenFramebuffers(1, &virtual_fbo);
      glGenRenderbuffers(1, &virtual_rbo);

      glBindRenderbuffer(GL_RENDERBUFFER, virtual_rbo);
      glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);
      
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, resolution.x, resolution.y);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, virtual_rbo);

      assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && "framebuffer creation error");

      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void OpenGL_Graphics_Driver::swap_and_present() {
    if (!initialized_default_shader) {
        return;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, virtual_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    {
      glBlitFramebuffer(
        0, 0, virtual_resolution.x, virtual_resolution.y,
        0, 0, real_resolution.x, real_resolution.y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST
      );
    }
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    SDL_GL_SetSwapInterval(vsync_enabled);
    SDL_GL_SwapWindow(game_window);
}

void OpenGL_Graphics_Driver::finish() {
    // NOTE: both fixed arrays are not deallocated, which is fine... since they don't take much memory anyway.
    // in a more mature environment they should really be freed otherwise.
    {
        glDeleteVertexArrays(1, &quad_vertex_array_object);
        glDeleteBuffers(1, &quad_vertex_buffer);
        quad_vertices.clear();
    }
    _debugprintf("OpenGL Driver Finish");
    _debugprintf("Deleting shader program and textures.");
    initialized_default_shader = false;
    glDeleteProgram(default_shader_program);
    for (unsigned index = 0; index < array_count(texture_ids); ++index) {
        glDeleteTextures(1, &texture_ids[index]);
        texture_ids[index] = 0;
    }
}

void OpenGL_Graphics_Driver::set_blend_mode(u8 new_blend_mode) {
    if (current_blend_mode != new_blend_mode) {
        // Previous objects are assumed to be in a different blend mode.
        flush_and_render_quads();
        current_blend_mode = new_blend_mode;

        switch (current_blend_mode) {
            case BLEND_MODE_NONE: {
                glBlendFunc(GL_ONE, GL_ZERO);
            } break;
            case BLEND_MODE_ALPHA: {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            } break;
            case BLEND_MODE_ADDITIVE: {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            } break;
            case BLEND_MODE_MULTIPLICATIVE: {
                // NOTE: this relies on premultiplied alpha
                // which I don't think I do,
                // but I also never use multiplicative blending in the game code.
                // pretty sure in the one case I might've used this for like shadowing
                // I'd just do color modulation myself or multitexturing or any of the other
                // solutions that basically look the same...
                glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
            } break;
        }
    }
}

void OpenGL_Graphics_Driver::set_texture_id(GLuint texture_id) {
    if (current_texture_id != texture_id) {
        flush_and_render_quads();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        current_texture_id = texture_id;
    }
}

void OpenGL_Graphics_Driver::clear_color_buffer(color32u8 color) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void OpenGL_Graphics_Driver::push_render_quad_vertices(
    rectangle_f32        destination,
    rectangle_f32        source,
    color32f32           color,
    struct image_buffer* image,
    s32                  angle,
    s32                  angle_y,
    u32                  flags,
    V2                   rotation_origin
) {
    if (image) {
        u32 image_width   = (image->pot_square_size) ? image->pot_square_size : image->width;
        u32 image_height  = (image->pot_square_size) ? image->pot_square_size : image->height;

        if ((source.x == 0) && (source.y == 0) && (source.w == 0) && (source.h == 0)) {
            source.w = image_width;
            source.h = image_height;
        }

        // normalize coordinates.
        source.x         /= image_width;
        source.y         /= image_height;
        source.w         /= image_width;
        source.h         /= image_height;


        if (image->_driver_userdata == nullptr) {
            _debugprintf("Warning? No opengl texture image id? Uploading dynamically.");
            upload_image_buffer_to_gpu(image);
        }
        // assertion(image->_driver_userdata && "How does this image not have a OpenGL texture id?");
        set_texture_id(*((GLuint*)image->_driver_userdata));
    } else {
        set_texture_id(white_pixel);
    }

    f32 c = cosf(degree_to_radians(angle));
    f32 s = sinf(degree_to_radians(angle));
    f32 c1 = cosf(degree_to_radians(angle_y));
    f32 s1 = sinf(degree_to_radians(angle_y));

    // rotate the basis axes
    // instead, since I cannot rotate each vertex.

    // although I do still have to rotate the top left vertex based on the basis differences.
    V2 down_basis  = V2_rotate(V2(0, destination.h), angle, angle_y);
    V2 right_basis = V2_rotate(V2(destination.w, 0), angle, angle_y);

    OpenGL_Vertex_Format top_left;
    {
        V2 displacement = V2(destination.x + ((rotation_origin.x) * destination.w), destination.y + ((rotation_origin.y) * destination.h));
        top_left.position =
            V2_rotate(
                V2(destination.x, destination.y) - displacement,
                angle, angle_y)
            + displacement;
        top_left.color    = color;
    }

    OpenGL_Vertex_Format top_right;
    {
        top_right.position = top_left.position + right_basis;
        top_right.color    = color;
    }
    OpenGL_Vertex_Format bottom_left;
    {
        bottom_left.position = top_left.position + down_basis;
        bottom_left.color    = color;
    }
    OpenGL_Vertex_Format bottom_right;
    {
        bottom_right.position = top_left.position + right_basis + down_basis;
        bottom_right.color    = color;
    }


#if 1
    top_left.texcoord     = V2(source.x, source.y);
    top_right.texcoord    = V2(source.x + source.w, source.y);
    bottom_left.texcoord  = V2(source.x, source.y + source.h);
    bottom_right.texcoord = V2(source.x + source.w, source.y + source.h);
#else
    top_left.texcoord     = V2(source.x, source.y + source.h);
    top_right.texcoord    = V2(source.x + source.w, source.y + source.h);
    bottom_left.texcoord  = V2(source.x, source.y);
    bottom_right.texcoord = V2(source.x + source.w, source.y);
#endif

    if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY)) {
        top_left.texcoord.x = source.x + source.w;
        top_right.texcoord.x = source.x;
        bottom_left.texcoord.x = source.x + source.w;
        bottom_right.texcoord.x = source.x;
    }

    if ((flags & DRAW_IMAGE_FLIP_VERTICALLY)) {
        top_left.texcoord.y = source.y + source.h;
        top_right.texcoord.y = source.y + source.h;
        bottom_left.texcoord.y = source.y;
        bottom_right.texcoord.y = source.y;
    }

    quad_vertices.push(top_left);
    quad_vertices.push(top_right);
    quad_vertices.push(bottom_left);
    quad_vertices.push(bottom_left);
    quad_vertices.push(top_right);
    quad_vertices.push(bottom_right);

    if (quad_vertices.size >= quad_vertices.capacity) {
        flush_and_render_quads();
    }
}

void OpenGL_Graphics_Driver::render_command_draw_quad(const render_command& rc) {
    auto destination_rect = rc.destination;
    auto source_rect      = rc.source;
    auto color            = rc.modulation_u8;
    auto image_buffer     = rc.image;
    set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void OpenGL_Graphics_Driver::render_command_draw_image(const render_command& rc) {
    auto destination_rect = rc.destination;
    auto source_rect      = rc.source;
    auto color            = rc.modulation_u8;
    auto image_buffer     = rc.image;
    // same thing but I just need an image.
    set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void OpenGL_Graphics_Driver::render_command_draw_line(const render_command& rc) {
    auto start = rc.start;
    auto end   = rc.end;
    auto color = color32u8_to_color32f32(rc.modulation_u8);
    set_blend_mode(rc.blend_mode);
    set_texture_id(white_pixel);

    auto line_normal = V2_perpendicular(V2_direction(start, end));
    {
        OpenGL_Vertex_Format top_left;
        {
            top_left.position = V2(start.x, start.y);
            top_left.color    = color;
        }
        OpenGL_Vertex_Format top_right;
        {
            top_right.position = V2(start.x, start.y) + line_normal * 1;
            top_right.color    = color;
        }
        OpenGL_Vertex_Format bottom_left;
        {
            bottom_left.position = V2(end.x, end.y);
            bottom_left.color    = color;
        }
        OpenGL_Vertex_Format bottom_right;
        {
            bottom_right.position = V2(end.x, end.y) + line_normal * 1;
            bottom_right.color    = color;
        }

        quad_vertices.push(top_left);
        quad_vertices.push(top_right);
        quad_vertices.push(bottom_left);
        quad_vertices.push(bottom_left);
        quad_vertices.push(top_right);
        quad_vertices.push(bottom_right);

        if (quad_vertices.size >= quad_vertices.capacity) {
            flush_and_render_quads();
        }
    }
}

void OpenGL_Graphics_Driver::render_command_draw_text(const render_command& rc) {
    auto font     = rc.font;
    auto scale    = rc.scale;
    auto position = rc.xy;
    auto text     = rc.text;
    auto color    = rc.modulation_u8;
    set_blend_mode(rc.blend_mode);

    {
        f32 x_cursor = position.x;
        f32 y_cursor = position.y;
        for (unsigned index = 0; index < text.length; ++index) {
            if (text.data[index] == '\n') {
                y_cursor += font->tile_height * scale;
                x_cursor =  position.x;
            } else {
                s32 character = text.data[index] - 32;
                auto destination_rect = rectangle_f32(
                    x_cursor, y_cursor + (rc.flags & BIT(0)) * GameUI::get_wobbly_factor(index, hash_bytes_fnv1a((u8*) text.data, text.length)),
                    font->tile_width  * scale,
                    font->tile_height * scale
                );

                auto source_rect = rectangle_f32(
                    (character % font->atlas_cols) * font->tile_width,
                    (character / font->atlas_cols) * font->tile_height,
                    font->tile_width, font->tile_height
                );

                push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), (struct image_buffer*)font);
                x_cursor += font->tile_width * scale;
            }
        }
    }
}

void OpenGL_Graphics_Driver::render_command_draw_set_scissor(const render_command& rc) {
    unimplemented("set scissor not implemented");
}

void OpenGL_Graphics_Driver::render_command_draw_clear_scissor(const render_command& rc) {
    unimplemented("clear scissor not implemented");
}

void OpenGL_Graphics_Driver::flush_and_render_quads(void) {
    if (quad_vertices.size == 0)
        return;

    glBindVertexArray(quad_vertex_array_object);
    GL_CheckError("bind vertex array");
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertex_buffer);

    // glBufferData(GL_ARRAY_BUFFER, quad_vertices.size * sizeof(OpenGL_Vertex_Format), quad_vertices.data, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, quad_vertices.size * sizeof(OpenGL_Vertex_Format), quad_vertices.data);
    GL_CheckError("buffer subdata update?");
    glDrawArrays(GL_TRIANGLES, 0, quad_vertices.size);
    GL_CheckError("draw array");

    quad_vertices.clear();
}

void OpenGL_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    if (!initialized_default_shader) {
        return;
    }

    /*
    * NOTE(jerry):
    * 
    * Ugh, this was a technical mistake since it relies on the fact the backbuffer isn't
    * supposed to be resized frequently (like in the D3D11 backend where you control the framebuffer).
    * 
    * OpenGL's default framebuffer is managed by the driver(?) which desynchronizes the viewport from
    * what the graphics code is expecting as the resizing code tries to prevent recreating the framebuffer
    * as much as possible (I.E. internally draws at some perfect multiple of a resolution so that it can
    * operate in a virtual pixel space, since afaik that's the *right* way to do 2D games.)
    */
    real_resolution = V2(Global_Engine()->real_screen_width, Global_Engine()->real_screen_height);

    // NOTE:
    // unlike the software renderer this doesn't try to clip anything
    // because it kind of doesn't have to? It would be too fast anyway...

    // Build new view matrix
    glUseProgram(default_shader_program);
    const auto& camera = commands->camera;
    {
        GLfloat transform_x = -(camera.xy.x + camera.trauma_displacement.x);
        GLfloat transform_y = -(camera.xy.y + camera.trauma_displacement.y);
        GLfloat scale_x     = camera.zoom;
        GLfloat scale_y     = camera.zoom;

        if (camera.centered) {
            transform_x += commands->screen_width/2;
            transform_y += commands->screen_height/2;
        }

        GLfloat view_matrix[] = {
            scale_x, 0,       0,                          0,
            0,       scale_y, 0,                          0,
            0,       0,       1.0f,                       0.0,
            transform_x,       transform_y,       0,    1.0
        };

        glUniformMatrix4fv(view_matrix_uniform_location, 1, false, view_matrix);
    }
    // setup projection matrix
    {
      GLfloat right = virtual_resolution.x;
      GLfloat left = 0;
      GLfloat top = 0;
      GLfloat bottom = virtual_resolution.y;

      GLfloat far = 100.0f;
      GLfloat near = 0.1f;
      GLfloat orthographic_matrix[] = {
          2 / (right - left), 0, 0, 0,
          0, 2 / (top - bottom), 0, 0,
          0, 0, 2 / (far - near), 0,
          -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1
      };

      glUniformMatrix4fv(projection_matrix_uniform_location, 1, 0, orthographic_matrix);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, virtual_fbo);
    glViewport(0, 0, virtual_resolution.x, virtual_resolution.y);

    if (commands->should_clear_buffer) {
        clear_color_buffer(commands->clear_buffer_color);
    }

    set_blend_mode(BLEND_MODE_ALPHA); // good default.
    // for (unsigned command_index = 0; command_index < commands->command_count; ++command_index) {
    //     auto& command = commands->commands[command_index];
    for (struct render_command_iterator it = render_command_iterator(commands);
         !render_command_iterator_finished(&it);
         render_command_iterator_advance(&it))
    {
        auto& command = *it.it;

        switch (command.type) {
            case RENDER_COMMAND_DRAW_QUAD: {
                render_command_draw_quad(command);
            } break;
            case RENDER_COMMAND_DRAW_IMAGE: {
                render_command_draw_image(command);
            } break;
            case RENDER_COMMAND_DRAW_TEXT: {
                render_command_draw_text(command);
            } break;
            case RENDER_COMMAND_DRAW_LINE: {
                render_command_draw_line(command);
            } break;
            case RENDER_COMMAND_SET_SCISSOR: {
                render_command_draw_set_scissor(command);
            } break;
            case RENDER_COMMAND_CLEAR_SCISSOR: {
                render_command_draw_clear_scissor(command);
            } break;
            case RENDER_COMMAND_POSTPROCESS_APPLY_SHADER: {
                unimplemented("I haven't had to use this, and I hope I'm not going to use it.");
            } break;
        }
    }

    flush_and_render_quads();
    render_commands_clear(commands);
    glUseProgram(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

V2 OpenGL_Graphics_Driver::resolution() {
    return V2(virtual_resolution.x, virtual_resolution.y);
}

void OpenGL_Graphics_Driver::upload_image_buffer_to_gpu(struct image_buffer* image) {
    auto backing_image_buffer         = image;
    auto image_buffer_driver_resource = (GLuint*)(backing_image_buffer->_driver_userdata);

    if (image_buffer_driver_resource) {
        _debugprintf("Existing driver resource found for (%p)(%p). Do not need to reload.", image, image->_driver_userdata);
        return;
    }

    _debugprintf("Loading new driver resource");
    unsigned int free_texture_id;
    assertion(find_first_free_texture_id(&free_texture_id) && "No more free texture ids. Bump the number.");
    texture_ids[free_texture_id] = opengl_build_texture2d_object(backing_image_buffer);
    backing_image_buffer->_driver_userdata = &texture_ids[free_texture_id];
    _debugprintf("Loaded texture2d resource");
}

void OpenGL_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    auto backing_image_buffer = graphics_assets_get_image_by_id(assets, image);
    upload_image_buffer_to_gpu(backing_image_buffer);
}

void OpenGL_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    auto backing_image_buffer = (struct image_buffer*)graphics_assets_get_font_by_id(assets, font);
    upload_image_buffer_to_gpu(backing_image_buffer);
}

void OpenGL_Graphics_Driver::unload_texture(struct graphics_assets* assets, image_id image) {
    auto backing_image_buffer         = graphics_assets_get_image_by_id(assets, image);
    auto  image_buffer_driver_resource = (GLuint*)(backing_image_buffer->_driver_userdata);
    if (image_buffer_driver_resource) {
        _debugprintf("Deleting graphics resource %p", image_buffer_driver_resource);
        glDeleteTextures(1, image_buffer_driver_resource);
    }
}

void OpenGL_Graphics_Driver::screenshot(char* where) {
    _debugprintf("OpenGL driver screenshot is nop");


    // NOTE: pretty sure opengl might read it upside down...
    struct image_buffer image = image_buffer_create_blank(real_resolution.x, real_resolution.y);
    glReadPixels(0, 0, real_resolution.x, real_resolution.y, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels);

    stbi_flip_vertically_on_write(1);
    stbi_write_png(where, image.width, image.height, 4, image.pixels, 4 * image.width);
    image_buffer_free(&image);
    //   unimplemented("Not done");
}


const char* OpenGL_Graphics_Driver::get_name(void) {
    return "(OpenGL 3.3)";
}
