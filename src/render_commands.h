#ifndef RENDER_COMMANDS_H
#define RENDER_COMMANDS_H

#include "prng.h"

#include "memory_arena.h"
#include "graphics_common.h"
#include "color.h"

#include "shader_effect_types.h"

enum render_command_type{
    RENDER_COMMAND_DRAW_QUAD,
    RENDER_COMMAND_DRAW_IMAGE,
    RENDER_COMMAND_DRAW_TEXT,
    RENDER_COMMAND_DRAW_LINE,
    // NOTE: this may or may not be dirty in a hardware implementation
    RENDER_COMMAND_POSTPROCESS_APPLY_SHADER,
};

static string render_command_type_strings[] = {
    string_literal("RENDER_COMMAND_DRAW_QUAD"),
    string_literal("RENDER_COMMAND_DRAW_IMAGE"),
    string_literal("RENDER_COMMAND_DRAW_TEXT"),
    string_literal("RENDER_COMMAND_DRAW_LINE"),
    string_literal("RENDER_COMMAND_DRAW_APPLY_SHADER"),
};

#define ALWAYS_ON_TOP (INFINITY)
struct render_command {
    /* easier to mix using a floating point value. */
    s32 shader_id_type;
    
    /*
        NOTE: in software rendering this pointer is directly accessed
        by the shader context.

        NOTE: in hardware rendering this pointer should be accessed as a CONSTANT_BUFFER in directX
        or a bunch of specific uniform settings in OpenGL.

        This is highly game specific.
    */
    void* shader_context;

    union {
        struct {
            struct font_cache*   font;
            string          text;
        };
        struct image_buffer* image;
    };

    struct rectangle_f32 destination;
    struct rectangle_f32 source;
    union {
        V2 start;
        V2 xy;
    };

    V2 rotation_center;
    V2 end;

    f32 scale;

    s32 angle_degrees;
    s32 angle_y_degrees;
    u32 flags;
    union {
        union color32u8  modulation_u8;
    };

    s16 type;
    u8  blend_mode;
};

struct render_commands {
    render_commands() {
        clear_buffer_color = color32u8_BLACK;
        should_clear_buffer = command_count = command_capacity = 0;
        screen_width = screen_height = 0;
        commands = nullptr;
    }
    struct camera          camera;
    u8                     should_clear_buffer;
    union color32u8        clear_buffer_color;
    struct render_command* commands;
    s32                    command_count;
    s32                    command_capacity;

    //                     This is not ideal,
    //                     but it is simplest to cache
    //                     the screen dimensions here.
    s32                    screen_width;
    s32                    screen_height;
};

struct render_commands render_commands(Memory_Arena* arena, s32 capacity, struct camera camera);

void render_commands_push_quad(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode);
void render_commands_push_quad_ext(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, V2 rotation_origin, s32 angle, u8 blend_mode);
void render_commands_push_quad_ext2(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, V2 rotation_origin, s32 angle, s32 angle_y, u8 blend_mode);

void render_commands_push_image(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, u32 flags, u8 blend_mode);
void render_commands_push_image_ext(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, V2 rotation_origin, s32 angle, u32 flags, u8 blend_mode);
void render_commands_push_image_ext2(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, V2 rotation_origin, s32 angle, s32 angle_y, u32 flags, u8 blend_mode);

void render_commands_push_line(struct render_commands* commands, V2 start, V2 end, union color32u8 rgba, u8 blend_mode);
void render_commands_push_text(struct render_commands* commands, struct font_cache* font, f32 scale, V2 xy, string cstring, union color32f32 rgba, u8 blend_mode);
void render_commands_push_shader_application(struct render_commands* commands, s32 shader_id, void* context);
// TODO: rotated text. This is more of a novelty that I don't think I need yet so I won't do it quite yet
// void render_commands_push_text_ext(struct render_commands* commands, struct font_cache* font, f32 scale, V2 xy, V2 rotation_origin, s32 angle, string cstring, union color32f32 rgba, u8 blend_mode);

void render_commands_set_shader(struct render_commands* commands, s32 shader_id, void* context);
void render_commands_clear(struct render_commands* commands);

// helper
void transform_command_into_clip_space(V2 resolution, struct render_command* command, struct camera* camera, V2 trauma_displacement);

// NOTE: Graphics Driver prototypes are here!
// but they should be defined in their own file.
void software_framebuffer_render_commands(struct software_framebuffer* framebuffer, struct render_commands* commands);

#endif
