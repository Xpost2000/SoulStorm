#ifndef LIGHTMASK_BUFFER_H
#define LIGHTMASK_BUFFER_H

#include "common.h"
/*
 * NOTE:
 *
 * This is kind of like a stencil-buffer for the software framebuffer renderer.
 *
 * It's technically an acceleration structure for the Legends' 2D lighting to avoid
 * overdraw because that game used a bloom effect which was expensive, and using this
 * allowed me to do rendering in one pass, which prevents lighting from being N^2 (kind of
 * like a weird 2D deferred renderer.)
 *
 * This is here in-case I ever want to reuse it quickly, but I don't expect to utilize this
 * effect.
 */

/*
  While this isn't strictly necessary for the graphics part,
  this is an auxiliary data structure used to keep my lighting happening in one pass.

  if a pixel is marked as 255, we will not light it, and draw it as full bright.
  Otherwise behavior is just lighting. Not sure what to do with non-255 values.

  Okay it really seems like I'm just using this as a stencil buffer...
*/
enum lightmask_draw_image_ex_flags {
    LIGHTMASK_DRAW_IMAGE_FLIP_HORIZONTALLY = BIT(0),
    LIGHTMASK_DRAW_IMAGE_FLIP_VERTICALLY = BIT(1),
};

enum lightmask_draw_blend {
    LIGHTMASK_BLEND_NONE,
    LIGHTMASK_BLEND_OR,
};
struct lightmask_buffer {
    uint8_t* mask_buffer;
    u32 width;
    u32 height;
};
/*
  NOTE: there is no render command/queueing system for the lightmask since it does not matter
  what order they go in.
 */
struct lightmask_buffer lightmask_buffer_create(u32 buffer_width, u32 buffer_height);
void                    lightmask_buffer_clear(struct lightmask_buffer* buffer);
void                    lightmask_buffer_blit_image_clipped(struct lightmask_buffer* buffer, struct rectangle_f32 clip_rect, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, u8 flags, u8 blend_mode, u8 v);
void                    lightmask_buffer_blit_rectangle_clipped(struct lightmask_buffer* buffer, struct rectangle_f32 clip_rect, struct rectangle_f32 destination, u8 blend_mode, u8 v);
void                    lightmask_buffer_blit_image(struct lightmask_buffer* buffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, u8 flags, u8 blend_mode, u8 v);
void                    lightmask_buffer_blit_rectangle(struct lightmask_buffer* buffer, struct rectangle_f32 destination, u8 blend_mode, u8 v);
bool                    lightmask_buffer_is_lit(struct lightmask_buffer* buffer, s32 x, s32 y);
f32                     lightmask_buffer_lit_percent(struct lightmask_buffer* buffer, s32 x, s32 y);
void                    lightmask_buffer_finish(struct lightmask_buffer* buffer);

#endif
