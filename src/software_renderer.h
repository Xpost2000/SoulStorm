/*
 * NOTE:
 *
 * This renderer is almost exactly verbatim copied from Legends-JRPG,
 *
 * it works exactly the same as it did back then.
 *
 * Multithreaded software renderer
 */
#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifndef __EMSCRIPTEN__
#define USE_SIMD_OPTIMIZATIONS
#define MULTITHREADED_EXPERIMENTAL
#endif

#include "graphics_common.h"

struct software_framebuffer : public image_buffer {
    s32 scissor_x;
    s32 scissor_y;
    s32 scissor_w;
    s32 scissor_h;
};

// NOTE: CPU shaders and GPU shaders are too different to share the same API, so I'll have to think carefully about how the API boundary is expressed...
// NOTE: do not directly use any of these
struct software_framebuffer software_framebuffer_create(u32 width, u32 height);
struct software_framebuffer software_framebuffer_create_from_arena(Memory_Arena* arena, u32 width, u32 height);
void                        software_framebuffer_finish(struct software_framebuffer* framebuffer);
void                        software_framebuffer_copy_into(struct software_framebuffer* target, struct software_framebuffer* source);
void                        software_framebuffer_clear_scissor(struct software_framebuffer* framebuffer);
void                        software_framebuffer_set_scissor(struct software_framebuffer* framebuffer, s32 x, s32 y, s32 w, s32 h);
void                        software_framebuffer_clear_buffer(struct software_framebuffer* framebuffer, union color32u8 rgba);
void                        software_framebuffer_draw_quad(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode);
void                        software_framebuffer_draw_quad_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect);
void                        software_framebuffer_draw_quad_ext_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y);
void                        software_framebuffer_draw_image_ex(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode);
void                        software_framebuffer_draw_image_ex_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, shader_fn shader, void* shader_ctx);
// NOTE: origin is supposed to be ([0.0,  1.0], [0.0, 1.0])
void                        software_framebuffer_draw_image_ext_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y, shader_fn shader, void* shader_ctx);
void                        software_framebuffer_draw_glyph(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, char glyph, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_line(struct software_framebuffer* framebuffer, V2 start, V2 end, union color32u8 rgba, u8 blend_mode);
void                        software_framebuffer_draw_text(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, string text, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_text_bounds(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, f32 bounds_w, string cstring, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_text_bounds_centered(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, struct rectangle_f32 bounds, string text, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_kernel_convolution_ex(Memory_Arena* arena, struct software_framebuffer* framebuffer, f32* kernel, s16 width, s16 height, f32 divisor, f32 blend_t, s32 passes);
void                        software_framebuffer_run_shader(struct software_framebuffer* framebuffer, struct rectangle_f32 src_rect, shader_fn shader, void* context);

void software_framebuffer_kernel_convolution_ex_bounded(struct software_framebuffer before, struct software_framebuffer* framebuffer, f32* kernel, s16 kernel_width, s16 kernel_height, f32 divisor, f32 blend_t, s32 passes, struct rectangle_f32 clip);
void software_framebuffer_render_commands(struct software_framebuffer* framebuffer, struct render_commands* commands);

#endif
