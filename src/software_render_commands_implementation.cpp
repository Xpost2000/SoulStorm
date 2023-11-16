#include "software_renderer.h"
#include "thread_pool.h"

/* TODO: The main benefit of render commands, is that we should be able to parallelize them.
   This should lead to a massive speed up, my SIMD optimization is kind of shit since it stabilizes but doesn't
   really improve framerate.
   
Time to parallelize.
*/
struct render_commands_job_details {
    struct software_framebuffer* framebuffer;
    struct render_commands*      commands;
    struct rectangle_f32         clip_rect;
};

// NOTE: these will be slower since there is basically not much I can pass into the engine...
shader_fn software_framebuffer_get_shader_function_for(s32 shader_id) {
    shader_fn result = nullptr;
    switch (shader_id) {
        case SHADER_EFFECT_TYPE_NONE: { } break;
        default: {
            auto name_of_type = render_command_type_strings[shader_id];
            _debugprintf("No shader implementation for (%.*s)", name_of_type.length, name_of_type.data);
        } break;
    }

    // no shader for now.
    return result;
}

void software_framebuffer_render_commands_tiled(struct software_framebuffer* framebuffer, struct render_commands* commands, struct rectangle_f32 clip_rect) {
    for (unsigned index = 0; index < commands->command_count; ++index) {
        struct render_command* command = &commands->commands[index];

        shader_fn shader_to_select = software_framebuffer_get_shader_function_for(command->shader_id_type);

        {
            rectangle_f32 rectangle = command->destination;

            if (command->type == RENDER_COMMAND_DRAW_TEXT) {
                f32 text_width = font_cache_text_width(command->font, command->text, command->scale);
                f32 text_height = font_cache_text_height(command->font) * command->scale;
                rectangle.x = command->xy.x;
                rectangle.y = command->xy.y;
                rectangle.w = text_width;
                rectangle.h = text_height;
            } else if (command->type == RENDER_COMMAND_DRAW_LINE) {
                // TODO: although I never really draw lines anyway.
                rectangle = clip_rect;
            }

            if (!rectangle_f32_intersect(clip_rect, rectangle)) {
                continue;
            }
        }

        switch (command->type) {
            case RENDER_COMMAND_DRAW_QUAD: {
                software_framebuffer_draw_quad_ext_clipped(
                    framebuffer,
                    command->destination,
                    command->modulation_u8,
                    command->blend_mode,
                    clip_rect,
                    command->rotation_center,
                    command->angle_degrees,
                    command->angle_y_degrees
                );
            } break;
            case RENDER_COMMAND_DRAW_IMAGE: {
                software_framebuffer_draw_image_ext_clipped(
                    framebuffer,
                    command->image,
                    command->destination,
                    command->source,
                    color32u8_to_color32f32(command->modulation_u8),
                    command->flags,
                    command->blend_mode,
                    clip_rect,
                    command->rotation_center,
                    command->angle_degrees,
                    command->angle_y_degrees,
                    shader_to_select,
                    command->shader_context
                );
            } break;
            case RENDER_COMMAND_DRAW_TEXT: {
                // NOTE: has no rotation variation.
                software_framebuffer_draw_text_clipped(
                    framebuffer,
                    command->font,
                    command->scale,
                    command->xy,
                    command->text,
                    color32u8_to_color32f32(command->modulation_u8),
                    command->blend_mode,
                    clip_rect
                );
            } break;
            case RENDER_COMMAND_DRAW_LINE: {
                software_framebuffer_draw_line_clipped(
                    framebuffer,
                    command->start,
                    command->end,
                    command->modulation_u8,
                    command->blend_mode,
                    clip_rect
                );
            } break;
            default: {
                auto name_of_type = render_command_type_strings[command->type];
                _debugprintf("unimplemented render command category (%.*s)", name_of_type.length, name_of_type.data);
            } break;
        }
    }
}

s32 thread_software_framebuffer_render_commands_tiles(void* context) {
    struct render_commands_job_details* job_details = (render_commands_job_details*)context;
    software_framebuffer_render_commands_tiled(job_details->framebuffer,
                                               job_details->commands,
                                               job_details->clip_rect);
    return 0;
}

void software_framebuffer_render_commands(struct software_framebuffer* framebuffer, struct render_commands* commands) {
    if (commands->should_clear_buffer) {
        software_framebuffer_clear_buffer(framebuffer, commands->clear_buffer_color);
    }

    /* move all things into clip space */
    V2 displacement      = camera_displacement_from_trauma(&commands->camera);

    for (unsigned index = 0; index < commands->command_count; ++index) {
        struct render_command* command = &commands->commands[index];
        transform_command_into_clip_space(V2(framebuffer->width, framebuffer->height), command, &commands->camera, displacement);
    }

#ifndef MULTITHREADED_EXPERIMENTAL
    software_framebuffer_render_commands_tiled(framebuffer, commands, rectangle_f32(0,0,framebuffer->width,framebuffer->height));
#else
    s32 JOB_W  = 4;
    s32 JOB_H  = 4;
    s32 TILE_W = framebuffer->width / JOB_W;
    s32 TILE_H = framebuffer->height / JOB_H;
    s32 TILE_W_remainder = framebuffer->width % TILE_W;
    s32 TILE_H_remainder = framebuffer->height % TILE_H;

    struct render_commands_job_details* job_details = (render_commands_job_details*)Global_Engine()->scratch_arena.push_unaligned(sizeof(*job_details) * (JOB_W*JOB_H));

    for (s32 y = 0; y < JOB_H; ++y) {
        for (s32 x = 0; x < JOB_W; ++x) {
            struct rectangle_f32 clip_rect = rectangle_f32(x * TILE_W, y * TILE_H, TILE_W, TILE_H);

            if (x == JOB_W) clip_rect.w += TILE_W_remainder;
            if (y == JOB_H) clip_rect.h += TILE_H_remainder;

            struct render_commands_job_details* current_details = &job_details[y*JOB_W+x];

            current_details->framebuffer = framebuffer;
            current_details->commands    = commands;
            current_details->clip_rect   = clip_rect;
            
            Thread_Pool::add_job(thread_software_framebuffer_render_commands_tiles, current_details);
        }
    }

    Thread_Pool::synchronize_tasks();
#endif
    commands->should_clear_buffer = 0;
    commands->command_count       = 0;
}
