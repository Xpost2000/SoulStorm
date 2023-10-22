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

#include "common.h"
#include "string.h"
#include "v2.h"

// hack: there's a macro to allow for constant constructions of color32f32s,
// which breaks the function ptr decl syntax.
struct software_framebuffer;
union color32f32;
typedef union color32f32(*shader_fn)(software_framebuffer* framebuffer, color32f32 source_pixel, V2 pixel_position, void* context);

#include "camera.h"

#include "lightmask_buffer.h"
#include "graphics_assets.h"
#include "render_commands.h"
/*
  This game isn't using hardware acceleration,
  as this means I can own my graphics stack, and
  also because I already have a working software renderer.

  As this happens to be a large swathe of code and is relatively
  low-level ish.

  This just like the Input system will remain with it's C style API.

  There's a lot of troubles that come from porting what is fundamentally a C engine
  designed in C as opposed to writing a fresh C++ engine in terms of design.

  The gameplay code is intended to be written in C++, and will be reflected as such,
  but there's no value in changing code that works and is relatively static.

  I.E. I don't really want to cause myself anymore headaches than trying to basically reimplement
  the same thing is already giving me...

  Since I'm basically just turning

  a_do_b(&a);

  into

  a.b();

  which doesn't really offer much benefit imo.
*/
#include "color.h"

enum draw_image_ex_flags {
    DRAW_IMAGE_FLIP_HORIZONTALLY = BIT(0),
    DRAW_IMAGE_FLIP_VERTICALLY   = BIT(1),
};

enum blit_blend_mode {
    BLEND_MODE_NONE,
    BLEND_MODE_ALPHA,
    BLEND_MODE_ADDITIVE,
    BLEND_MODE_MULTIPLICATIVE,
    BLEND_MODE_COUNT,
};

struct software_framebuffer : public image_buffer {
    s32 scissor_x;
    s32 scissor_y;
    s32 scissor_w;
    s32 scissor_h;
};

// NOTE: CPU shaders and GPU shaders are too different to share the same API, so I'll have to think carefully about how the API boundary is expressed...
struct software_framebuffer software_framebuffer_create(u32 width, u32 height);
struct software_framebuffer software_framebuffer_create_from_arena(Memory_Arena* arena, u32 width, u32 height);
void                        software_framebuffer_finish(struct software_framebuffer* framebuffer);
void                        software_framebuffer_copy_into(struct software_framebuffer* target, struct software_framebuffer* source);
void                        software_framebuffer_clear_scissor(struct software_framebuffer* framebuffer);
void                        software_framebuffer_set_scissor(struct software_framebuffer* framebuffer, s32 x, s32 y, s32 w, s32 h);
void                        software_framebuffer_clear_buffer(struct software_framebuffer* framebuffer, union color32u8 rgba);
void                        software_framebuffer_draw_quad(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode);
void                        software_framebuffer_draw_quad_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect);
void                        software_framebuffer_draw_quad_ext_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle);
void                        software_framebuffer_draw_image_ex(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode);
void                        software_framebuffer_draw_image_ex_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, shader_fn shader, void* shader_ctx);
// NOTE: origin is supposed to be ([0.0,  1.0], [0.0, 1.0])
void                        software_framebuffer_draw_image_ext_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, shader_fn shader, void* shader_ctx);
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
