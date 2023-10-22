#include "lightmask_buffer.h"
#include "color.h"
#include "image_buffer.h"

struct lightmask_buffer lightmask_buffer_create(u32 buffer_width, u32 buffer_height) {
    struct lightmask_buffer result;
    result.width       = buffer_width;
    result.height      = buffer_height;
    result.mask_buffer = (u8*)malloc(buffer_width * buffer_height);
    lightmask_buffer_clear(&result);
    return result;
}

void lightmask_buffer_clear(struct lightmask_buffer* buffer) {
    zero_memory(buffer->mask_buffer, buffer->width*buffer->height);
}

static inline u8 lightmask_buffer_get_pixel(struct lightmask_buffer* buffer, s32 x, s32 y) {
    u32 index = y * buffer->width + x;
    return buffer->mask_buffer[index];
}

static inline void lightmask_buffer_put_pixel(struct lightmask_buffer* buffer, s32 x, s32 y, u8 value, u8 blend_mode) {
    u32 index = y * buffer->width + x;

    switch (blend_mode) {
        case LIGHTMASK_BLEND_NONE: {
            buffer->mask_buffer[index] = value;
        } break;
        case LIGHTMASK_BLEND_OR: {
            if (lightmask_buffer_get_pixel(buffer, x, y) == 255) {
                // ...
            } else {
                buffer->mask_buffer[index] = value;
            }
        } break;
    }
}

/* NOTE: this is not updated to understand rotation yet. If I use it, I'll update it! */
/*
  Technically this is the same as a software_framebuffer renderer with only 8bits of information though,
  however the software_framebuffer just like the image_buffer is built to assume 32bit RGBA pixel information
  for simplicity.

  So there is a little duplicated code :/
*/
void lightmask_buffer_blit_image_clipped(struct lightmask_buffer* buffer, struct rectangle_f32 clip_rect, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, u8 flags, u8 blend_mode, u8 v) {
    if ((destination.x == 0) && (destination.y == 0) && (destination.w == 0) && (destination.h == 0)) {
        destination.w = buffer->width;
        destination.h = buffer->height;
    }

    if (!rectangle_f32_intersect(destination, clip_rect)) {
        return;
    }

    if ((src.x == 0) && (src.y == 0) && (src.w == 0) && (src.h == 0)) {
        src.w = image->width;
        src.h = image->height;
    }

    f32 scale_ratio_w = (f32)src.w  / destination.w;
    f32 scale_ratio_h = (f32)src.h  / destination.h;

    s32 start_x = clamp<s32>((s32)destination.x, clip_rect.x, clip_rect.x+clip_rect.w);
    s32 start_y = clamp<s32>((s32)destination.y, clip_rect.y, clip_rect.y+clip_rect.h);
    s32 end_x   = clamp<s32>((s32)(destination.x + destination.w), clip_rect.x, clip_rect.x+clip_rect.w);
    s32 end_y   = clamp<s32>((s32)(destination.y + destination.h), clip_rect.y, clip_rect.y+clip_rect.h);

    s32 unclamped_end_x = (s32)(destination.x + destination.w);
    s32 unclamped_end_y = (s32)(destination.y + destination.h);

    s32 stride       = buffer->width;
    s32 image_stride = image->width;

    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {
            s32 image_sample_x = (s32)((src.x + src.w) - ((unclamped_end_x - x_cursor) * scale_ratio_w));
            s32 image_sample_y = (s32)((src.y + src.h) - ((unclamped_end_y - y_cursor) * scale_ratio_h));

            if ((flags & LIGHTMASK_DRAW_IMAGE_FLIP_HORIZONTALLY))
                image_sample_x = (s32)(((unclamped_end_x - x_cursor) * scale_ratio_w) + src.x);

            if ((flags & LIGHTMASK_DRAW_IMAGE_FLIP_VERTICALLY))
                image_sample_y = (s32)(((unclamped_end_y - y_cursor) * scale_ratio_h) + src.y);

            union color32f32 sampled_pixel = color32f32(image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 0] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 1] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 2] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 3] / 255.0f);

            if (sampled_pixel.a == 1.0) {
                lightmask_buffer_put_pixel(buffer, x_cursor, y_cursor, v, blend_mode);
            }
        }
    }
}

void lightmask_buffer_blit_rectangle_clipped(struct lightmask_buffer* buffer, struct rectangle_f32 clip_rect, struct rectangle_f32 destination, u8 blend_mode, u8 v) {
    s32 start_x = clamp<s32>((s32)destination.x, clip_rect.x, clip_rect.x+clip_rect.w);
    s32 start_y = clamp<s32>((s32)destination.y, clip_rect.y, clip_rect.y+clip_rect.h);
    s32 end_x   = clamp<s32>((s32)(destination.x + destination.w), clip_rect.x, clip_rect.x+clip_rect.w);
    s32 end_y   = clamp<s32>((s32)(destination.y + destination.h), clip_rect.y, clip_rect.y+clip_rect.h);

    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {
            lightmask_buffer_put_pixel(buffer, x_cursor, y_cursor, v, blend_mode);
        }
    }
}

void lightmask_buffer_blit_image(struct lightmask_buffer* buffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, u8 flags, u8 blend_mode, u8 v) {
    lightmask_buffer_blit_image_clipped(buffer, rectangle_f32(0, 0, buffer->width, buffer->height), image, destination, src, flags, blend_mode, v);
}

void lightmask_buffer_blit_rectangle(struct lightmask_buffer* buffer, struct rectangle_f32 destination, u8 blend_mode, u8 v) {
    lightmask_buffer_blit_rectangle_clipped(buffer, rectangle_f32(0, 0, buffer->width, buffer->height), destination, blend_mode, v);
}

f32 lightmask_buffer_lit_percent(struct lightmask_buffer* buffer, s32 x, s32 y) {
    u32 index = y * buffer->width + x;
    u8 value = buffer->mask_buffer[index];

    return (value) / 255.0f;
}

bool lightmask_buffer_is_lit(struct lightmask_buffer* buffer, s32 x, s32 y) {
    u32 index = y * buffer->width + x;

    if (buffer->mask_buffer[index] == 255) {
        return 1;
    }

    return 0;
}

void lightmask_buffer_finish(struct lightmask_buffer* buffer) {
    if (buffer->mask_buffer) {
        free(buffer->mask_buffer);
        buffer->mask_buffer = NULL;

        buffer->width  = 0;
        buffer->height = 0;
    }
}
