#include "render_commands.h"

struct render_commands render_commands(Memory_Arena* arena, s32 capacity, struct camera camera) {
    struct render_commands result = {};
    result.should_clear_buffer = 0;
    result.clear_buffer_color.r = 0;
    result.clear_buffer_color.g = 0;
    result.clear_buffer_color.b = 0;
    result.clear_buffer_color.a = 0;
    result.camera = camera;
    result.command_capacity = capacity;
    result.command_count = 0;
    result.commands         = (render_command*)arena->push_unaligned(capacity * sizeof(*result.commands));
    return result;
}

struct render_command* render_commands_new_command(struct render_commands* commands, s16 type) {
    assert(commands->command_count < commands->command_capacity && "Overrun command buffer?");
    struct render_command* command = &commands->commands[commands->command_count++];
    command->type = type;
    return command;
}

void render_commands_set_shader(struct render_commands* commands, s32 shader_id, void* context) {
    struct render_command* last_command = &commands->commands[commands->command_count - 1];
    last_command->shader_id_type = shader_id;
    last_command->shader_context = context;
}

void render_commands_push_quad_ext(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, V2 rotation_origin, s32 angle, u8 blend_mode) {
    struct render_command* command = render_commands_new_command(commands, RENDER_COMMAND_DRAW_QUAD);
    command->destination           = destination;
    command->flags                 = 0;
    command->modulation_u8         = rgba;
    command->blend_mode            = blend_mode;
    command->rotation_center       = rotation_origin;
    command->angle_degrees         = angle;
}

void render_commands_push_quad(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode) {
    render_commands_push_quad_ext(commands, destination, rgba, V2(0, 0), 0, blend_mode);
}

void render_commands_push_image_ext(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, V2 rotation_origin, s32 angle, u32 flags, u8 blend_mode) {
    struct render_command* command = render_commands_new_command(commands, RENDER_COMMAND_DRAW_IMAGE);
    command->destination           = destination;
    command->image                 = image;
    command->source                = source;
    command->flags                 = flags;
    command->modulation_u8         = color32f32_to_color32u8(rgba);
    command->blend_mode            = blend_mode;
    command->rotation_center       = rotation_origin;
    command->angle_degrees         = angle;
}

void render_commands_push_image(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, u32 flags, u8 blend_mode){
    render_commands_push_image_ext(commands, image, destination, source, rgba, V2(0, 0), 0, flags, blend_mode);
}

void render_commands_push_line(struct render_commands* commands, V2 start, V2 end, union color32u8 rgba, u8 blend_mode) {
    struct render_command* command = render_commands_new_command(commands, RENDER_COMMAND_DRAW_LINE);
    command->start         = start;
    command->end           = end;
    command->flags         = 0;
    command->modulation_u8 = rgba;
    command->blend_mode    = blend_mode;
}

void render_commands_push_text(struct render_commands* commands, struct font_cache* font, f32 scale, V2 xy, string text, union color32f32 rgba, u8 blend_mode) {
    struct render_command* command = render_commands_new_command(commands, RENDER_COMMAND_DRAW_TEXT);
    command->font          = font;
    command->xy            = xy;
    command->scale         = scale;
    command->text          = text;
    command->modulation_u8 = color32f32_to_color32u8(rgba);
    command->flags         = 0;
    command->blend_mode    = blend_mode;
}

void render_commands_push_shader_application(struct render_commands* commands, s32 shader_id, void* context) {
    struct render_command* command = render_commands_new_command(commands, RENDER_COMMAND_POSTPROCESS_APPLY_SHADER);
    command->shader_id_type = shader_id;
    command->shader_context = context;
}

void render_commands_clear(struct render_commands* commands) {
    commands->command_count = 0;
}

void transform_command_into_clip_space(V2 resolution, struct render_command* command, struct camera* camera, V2 trauma_displacement) {
    f32 half_screen_width  = resolution.x/2;
    f32 half_screen_height = resolution.y/2;

    {
        command->start.x       *= camera->zoom;
        command->start.y       *= camera->zoom;
        command->end.x         *= camera->zoom;
        command->end.y         *= camera->zoom;
        command->destination.x *= camera->zoom;
        command->destination.y *= camera->zoom;
        command->destination.w *= camera->zoom;
        command->destination.h *= camera->zoom;
        command->scale         *= camera->zoom;
    }

    if (camera->centered) {
        command->start.x       += half_screen_width;
        command->start.y       += half_screen_height;
        command->end.x         += half_screen_width;
        command->end.y         += half_screen_height;
        command->destination.x += half_screen_width;
        command->destination.y += half_screen_height;
    }

    {
        command->start.x       -= camera->xy.x + (trauma_displacement.x);
        command->start.y       -= camera->xy.y + (trauma_displacement.y);
        command->end.x         -= camera->xy.x + (trauma_displacement.x);
        command->end.y         -= camera->xy.y + (trauma_displacement.y);
        command->destination.x -= camera->xy.x + (trauma_displacement.x);
        command->destination.y -= camera->xy.y + (trauma_displacement.y);
    }
}
