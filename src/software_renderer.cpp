#include "software_renderer.h"
#include "thread_pool.h"
#include "engine.h"

#include "game_ui.h"

inline local void _rotate_f32_xy_as_pseudo_zyx(f32* x, f32* y, f32 c=0, f32 s=0, f32 c1=0, f32 s1=0, f32 c2=0, f32 s2=1) {
    // z rot
    float _x = *x;
    float _y = *y;
    *x = floor(c * (_x) - s * (_y));
    *y = floor(s * (_x) + c * (_y));

#if 1
    // y rot
    *y = floor(c1 * (*y));
    // x rot
    *x = floor(s2 * (*x));
#endif
}

// The main software renderer code
struct software_framebuffer software_framebuffer_create(u32 width, u32 height) {
    u8* pixels = (u8*)malloc(width * height * sizeof(u32)+64);

    struct software_framebuffer result;
    result.width = width;
    result.height = height;
    result.pixels = pixels;

    software_framebuffer_clear_scissor(&result);

    return result;
}

struct software_framebuffer software_framebuffer_create_from_arena(Memory_Arena* arena, u32 width, u32 height) {
    u8* pixels = (u8*)arena->push_unaligned(width * height * sizeof(u32)+64);

    struct software_framebuffer result;
    result.width = width;
    result.height = height;
    result.pixels = pixels;

    software_framebuffer_clear_scissor(&result);

    return result;
}

void software_framebuffer_finish(struct software_framebuffer* framebuffer) {
    if (framebuffer->pixels) {
        free(framebuffer->pixels);
        framebuffer->width  = 0;
        framebuffer->height = 0;
        framebuffer->pixels = 0;
    }
}

void software_framebuffer_clear_scissor(struct software_framebuffer* framebuffer) {
    framebuffer->scissor_w = framebuffer->scissor_h = framebuffer->scissor_x = framebuffer->scissor_y = 0;
}

void software_framebuffer_set_scissor(struct software_framebuffer* framebuffer, s32 x, s32 y, s32 w, s32 h) {
    framebuffer->scissor_x = x;
    framebuffer->scissor_y = y;
    framebuffer->scissor_w = w;
    framebuffer->scissor_h = h;
}

void software_framebuffer_clear_buffer(struct software_framebuffer* framebuffer, union color32u8 rgba) {
    memory_set32(framebuffer->pixels, framebuffer->width * framebuffer->height * sizeof(u32), rgba.rgba_packed);
}

/* this is a macro because I don't know if this will be inlined... */
#define _BlendPixel_Scalar(FRAMEBUFFER, X, Y, RGBA, BLEND_MODE) do {    \
        u32  stride                   = FRAMEBUFFER->width;             \
        switch (BLEND_MODE) {                                           \
            case BLEND_MODE_NONE: {                                     \
                float alpha = RGBA.a / 255.0f;                          \
                FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] =  (RGBA.r * alpha); \
                FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] =  (RGBA.g * alpha); \
                FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] =  (RGBA.b * alpha); \
                FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 3] =  (RGBA.a); \
            } break;                                                    \
            case BLEND_MODE_ALPHA: {                                    \
                {                                                       \
                    float alpha = RGBA.a / 255.0f;                      \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] = (FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] * (1 - alpha)) + (RGBA.r * alpha); \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] = (FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] * (1 - alpha)) + (RGBA.g * alpha); \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] = (FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] * (1 - alpha)) + (RGBA.b * alpha); \
                }                                                       \
                FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 3] = 255;  \
            } break;                                                    \
            case BLEND_MODE_ADDITIVE: {                                 \
                {                                                       \
                    float alpha = RGBA.a / 255.0f;                      \
                    u32 added_r = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] + RGBA.r * alpha; \
                    u32 added_g = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] + RGBA.g * alpha; \
                    u32 added_b = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] + RGBA.b * alpha; \
                                                                        \
                    if (added_r > 255) added_r = 255;                   \
                    if (added_g > 255) added_g = 255;                   \
                    if (added_b > 255) added_b = 255;                   \
                                                                        \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] = added_r; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] = added_g; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] = added_b; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 3] = 255; \
                }                                                       \
            } break;                                                    \
            case BLEND_MODE_MULTIPLICATIVE: {                           \
                {                                                       \
                    u32 modulated_r = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] * RGBA.r/255.0f; \
                    u32 modulated_g = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] * RGBA.g/255.0f; \
                    u32 modulated_b = FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] * RGBA.b/255.0f; \
                                                                        \
                    if (modulated_r > 255) modulated_r = 255;           \
                    if (modulated_g > 255) modulated_g = 255;           \
                    if (modulated_b > 255) modulated_b = 255;           \
                                                                        \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 0] = modulated_r; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 1] = modulated_g; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 2] = modulated_b; \
                    FRAMEBUFFER->pixels[Y * stride * 4 + X * 4 + 3] = 255; \
                }                                                       \
            } break;                                                    \
                bad_case;                                               \
        }                                                               \
    } while(0)

local bool _framebuffer_scissor_cull(struct software_framebuffer* framebuffer, s32 x, s32 y) {
    if (framebuffer->scissor_w == 0 || framebuffer->scissor_h == 0) {
        return 
            (x < 0 ||
             x >= framebuffer->width ||
             y < 0 ||
             y >= framebuffer->height);
    }
    
    if (x < framebuffer->scissor_x                        ||
        y < framebuffer->scissor_y                        ||
        x >= framebuffer->scissor_x+framebuffer->scissor_w ||
        y >= framebuffer->scissor_y+framebuffer->scissor_h) {
        return true;
    }

    return false;
}

/* NOTE: Since I rarely draw standard quads for *legit* reasons, these don't have the ability to be shaded. */
#ifdef USE_SIMD_OPTIMIZATIONS
void software_framebuffer_draw_quad_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect) {
    __m128i rect_edges_end   = _mm_set_epi32(clip_rect.x+clip_rect.w, clip_rect.y+clip_rect.h, clip_rect.x + clip_rect.w, clip_rect.y + clip_rect.h);
    __m128i rect_edges_start = _mm_set_epi32(clip_rect.x, clip_rect.y, clip_rect.x, clip_rect.y);
    union {
        s32 xyzw[4];
        __m128i vectors;
    } v;
    v.vectors =  _mm_set_epi32(destination.x, destination.y, destination.x+destination.w, destination.y+destination.h);
    v.vectors           = _mm_min_epi32(_mm_max_epi32(v.vectors, rect_edges_start), rect_edges_end);
    s32 start_x = (v.xyzw)[3];
    s32 start_y = (v.xyzw)[2];
    s32 end_x   = (v.xyzw)[1];
    s32 end_y   = (v.xyzw)[0];

    __m128 red_channels   = _mm_set1_ps((f32)rgba.r);
    __m128 green_channels = _mm_set1_ps((f32)rgba.g);
    __m128 blue_channels  = _mm_set1_ps((f32)rgba.b);
    __m128 alpha_channels = _mm_set1_ps((f32)rgba.a);
    __m128 inverse_255    = _mm_set1_ps(1.0 / 255.0f);

    alpha_channels = _mm_mul_ps(inverse_255, alpha_channels);

    __m128 zero           = _mm_set1_ps(0);
    __m128 two_fifty_five = _mm_set1_ps(255);

    s32 stride = framebuffer->width;
    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        for (s32 x_cursor = start_x; x_cursor < end_x; x_cursor += 4) {
            __m128 red_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 0]
            );
            __m128 green_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 1]
            );
            __m128 blue_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 2]
            );

            switch (blend_mode) {
                case BLEND_MODE_NONE: {
                    red_destination_channels   = _mm_mul_ps(red_channels,   alpha_channels);
                    green_destination_channels = _mm_mul_ps(green_channels, alpha_channels);
                    blue_destination_channels  = _mm_mul_ps(blue_channels,  alpha_channels);
                } break;
                case BLEND_MODE_ALPHA: {
                    __m128 one_minus_alpha     = _mm_sub_ps(_mm_set1_ps(1), alpha_channels);
                    red_destination_channels   = _mm_add_ps(_mm_mul_ps(red_destination_channels,   one_minus_alpha),   _mm_mul_ps(red_channels, alpha_channels));
                    green_destination_channels = _mm_add_ps(_mm_mul_ps(green_destination_channels, one_minus_alpha),   _mm_mul_ps(green_channels, alpha_channels));
                    blue_destination_channels  = _mm_add_ps(_mm_mul_ps(blue_destination_channels,  one_minus_alpha),   _mm_mul_ps(blue_channels, alpha_channels));
                } break;
                case BLEND_MODE_ADDITIVE: {
                    red_destination_channels   = _mm_add_ps(red_destination_channels,   _mm_mul_ps(red_channels,   alpha_channels));
                    green_destination_channels = _mm_add_ps(green_destination_channels, _mm_mul_ps(green_channels, alpha_channels));
                    blue_destination_channels  = _mm_add_ps(blue_destination_channels,  _mm_mul_ps(blue_channels,  alpha_channels));
                } break;
                case BLEND_MODE_MULTIPLICATIVE: {
                    red_destination_channels   = _mm_mul_ps(red_destination_channels,   _mm_mul_ps(_mm_mul_ps(red_channels, alpha_channels),   inverse_255));
                    green_destination_channels = _mm_mul_ps(green_destination_channels, _mm_mul_ps(_mm_mul_ps(green_channels, alpha_channels), inverse_255));
                    blue_destination_channels  = _mm_mul_ps(blue_destination_channels,  _mm_mul_ps(_mm_mul_ps(blue_channels,  alpha_channels), inverse_255));
                } break;
                    bad_case;
            }

#define castF32_M128(X) ((f32*)(&X))
            blue_destination_channels  = _mm_min_ps(_mm_max_ps(blue_destination_channels , zero), two_fifty_five);
            green_destination_channels = _mm_min_ps(_mm_max_ps(green_destination_channels, zero), two_fifty_five);
            red_destination_channels   = _mm_min_ps(_mm_max_ps(red_destination_channels  , zero), two_fifty_five);


            for (int i = 0; i < 4; ++i) {
                if ((x_cursor + i >= clip_rect.x+clip_rect.w)) break;
                if (_framebuffer_scissor_cull(framebuffer, x_cursor+i, y_cursor)) continue;

                framebuffer->pixels_u32[y_cursor * framebuffer->width + (x_cursor+i)] = packu32(
                    255,
                    castF32_M128(blue_destination_channels)[i],
                    castF32_M128(green_destination_channels)[i],
                    castF32_M128(red_destination_channels)[i]
                );
            }
#undef castF32_M128
        }
    }
}
#else
void software_framebuffer_draw_quad_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect) {
    software_framebuffer_draw_quad_ext_clipped(framebuffer, destination, rgba, blend_mode, clip_rect, V2(0, 0), 0, 0);
}
#endif

// NOTE: EXT does not have a SIMD version
// but that's also because drawing lots of small quads is sort of uncommon and also is way faster
// than images... So I don't really want to do SIMD for this.
void software_framebuffer_draw_quad_ext_clipped(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y) {
    s32 start_x           = clamp<s32>((s32)destination.x, clip_rect.x, clip_rect.x+clip_rect.w);
    s32 start_y           = clamp<s32>((s32)destination.y, clip_rect.y, clip_rect.y+clip_rect.h);
    s32 end_x             = clamp<s32>((s32)(destination.x + destination.w), clip_rect.x, clip_rect.x+clip_rect.w);
    s32 end_y             = clamp<s32>((s32)(destination.y + destination.h), clip_rect.y, clip_rect.y+clip_rect.h);

    s32 unclamped_start_x = (s32)(destination.x);
    s32 unclamped_start_y = (s32)(destination.y);
    s32 unclamped_end_x   = (s32)(destination.x + destination.w);
    s32 unclamped_end_y   = (s32)(destination.y + destination.h);

    u32* framebuffer_pixels_as_32 = (u32*)framebuffer->pixels;
    unused(framebuffer_pixels_as_32);

    f32 c = cosf(degree_to_radians(angle));
    f32 s = sinf(degree_to_radians(angle));
    f32 c1 = cosf(degree_to_radians(angle_y));
    f32 s1 = sinf(degree_to_radians(angle_y));

    s32 origin_off_x = (s32)(destination.w * origin.x);
    s32 origin_off_y = (s32)(destination.h * origin.y);

    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {

            if (angle == 0 && angle_y == 0) {
                if (_framebuffer_scissor_cull(framebuffer, x_cursor, y_cursor)) continue;
                _BlendPixel_Scalar(framebuffer, x_cursor, y_cursor, rgba, blend_mode);
            } else {
                s32 adjx  = x_cursor - (unclamped_start_x + origin_off_x);
                s32 adjy  = y_cursor - (unclamped_start_y + origin_off_y);
                f32 dx    = adjx;
                f32 dy    = adjy;
                _rotate_f32_xy_as_pseudo_zyx(&dx, &dy, c, s, c1, s1);

                dx       += (unclamped_start_x + origin_off_x);
                dy       += (unclamped_start_y + origin_off_y);

                static const V2 subsamples[] = {
                    V2(0, 0), V2(0.5, 0), V2(0.5, 0.5), V2(0, 0.5), V2(-0.5, 0.0), V2(0.0, -0.5), V2(-0.5, -0.5)
                };

                for (auto& sample : subsamples) {
                    f32 sample_approx_x = dx + sample.x;
                    f32 sample_approx_y = dy + sample.y;

                    if (_framebuffer_scissor_cull(framebuffer, sample_approx_x, sample_approx_y)) continue;
                    _BlendPixel_Scalar(framebuffer, (s32)(sample_approx_x), (s32)(sample_approx_y), rgba, blend_mode);
                }
            }
        }
    }
}

void software_framebuffer_draw_quad(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode) {
    software_framebuffer_draw_quad_clipped(framebuffer, destination, rgba, blend_mode, rectangle_f32(0, 0, framebuffer->width, framebuffer->height));
}

void software_framebuffer_draw_image_ex(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode) {
    software_framebuffer_draw_image_ex_clipped(framebuffer, image, destination, src, modulation, flags, blend_mode, rectangle_f32(0,0,framebuffer->width,framebuffer->height), 0, 0);
}

void software_framebuffer_draw_image_ex_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, shader_fn shader, void* shader_ctx) {
    software_framebuffer_draw_image_ext_clipped(framebuffer, image, destination, src, modulation, flags, blend_mode, clip_rect, V2(0, 0), 0, 0, shader, shader_ctx);
}

void software_framebuffer_draw_image_ext_clipped_scalar(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y, shader_fn shader, void* shader_ctx) {
    if ((destination.x == 0) && (destination.y == 0) && (destination.w == 0) && (destination.h == 0)) {
        destination.w = framebuffer->width;
        destination.h = framebuffer->height;
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

    s32 unclamped_start_x = (s32)(destination.x);
    s32 unclamped_start_y = (s32)(destination.y);
    s32 unclamped_end_x = (s32)(destination.x + destination.w);
    s32 unclamped_end_y = (s32)(destination.y + destination.h);

    s32 origin_off_x = (s32)(destination.w * origin.x);
    s32 origin_off_y = (s32)(destination.h * origin.y);

    u32* framebuffer_pixels_as_32 = (u32*)framebuffer->pixels;
    unused(framebuffer_pixels_as_32);

    s32 stride       = framebuffer->width;
    s32 image_stride = image->width;

    if (image->pot_square_size) {
        image_stride = image->pot_square_size;
    }

    f32 c = cosf(degree_to_radians(angle));
    f32 s = sinf(degree_to_radians(angle));
    f32 c1 = cosf(degree_to_radians(angle_y));
    f32 s1 = sinf(degree_to_radians(angle_y));
    
#if 0
    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        s32 image_sample_y = (s32)fabs(fmodf((src.y + src.h) - ((unclamped_end_y - y_cursor) * scale_ratio_h), image->height));

        if ((flags & DRAW_IMAGE_FLIP_VERTICALLY))
            image_sample_y = (s32)((((unclamped_end_y-1) - y_cursor) * scale_ratio_h) + src.y);

        for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {
            s32 image_sample_x = (s32)fabs(fmodf((src.x + src.w) - ((unclamped_end_x - x_cursor) * scale_ratio_w), image->width));

            if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY))
                image_sample_x = (s32)((((unclamped_end_x-1) - x_cursor) * scale_ratio_w) + src.x);

            union color32f32 sampled_pixel = color32f32(image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 0] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 1] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 2] / 255.0f,
                                                        image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 3] / 255.0f);

            if (shader) {
                sampled_pixel = shader(framebuffer, sampled_pixel, V2(x_cursor, y_cursor), shader_ctx);
            }
            sampled_pixel.r *= 255.0f;
            sampled_pixel.g *= 255.0f;
            sampled_pixel.b *= 255.0f;
            sampled_pixel.a *= 255.0f;

            sampled_pixel.r *= modulation.r;
            sampled_pixel.g *= modulation.g;
            sampled_pixel.b *= modulation.b;
            sampled_pixel.a *= modulation.a;

            if (angle == 0 && angle_y == 0) {
                if (_framebuffer_scissor_cull(framebuffer, x_cursor, y_cursor)) continue;
                _BlendPixel_Scalar(framebuffer, x_cursor, y_cursor, sampled_pixel, blend_mode);
            } else {
                s32 adjx  = x_cursor - (unclamped_start_x + origin_off_x);
                s32 adjy  = y_cursor - (unclamped_start_y + origin_off_y);
                f32 dx    = adjx;
                f32 dy    = adjy;
                _rotate_f32_xy_as_pseudo_zyx(&dx, &dy, c, s, c1, s1);

                dx       += (unclamped_start_x + origin_off_x);
                dy       += (unclamped_start_y + origin_off_y);

                // dx = clamp<f32>((f32)dx, clip_rect.x, clip_rect.x+clip_rect.w);
                // dy = clamp<f32>((f32)dy, clip_rect.y, clip_rect.y+clip_rect.h);

                /*
                  for rotation need more sampling
                */
                static const V2 subsamples[] = {
                    V2(0, 0), V2(0.5, 0), V2(0.5, 0.5), V2(0, 0.5), V2(-0.5, 0.0), V2(0.0, -0.5), V2(-0.5, -0.5)
                };

                for (auto& sample : subsamples) {
                    f32 sample_approx_x = ceilf(dx + sample.x);
                    f32 sample_approx_y = ceilf(dy + sample.y);

                    // if (sample_approx_x < clip_rect.x || sample_approx_x >= clip_rect.x + clip_rect.w ||
                    //     sample_approx_y < clip_rect.y || sample_approx_y >= clip_rect.y + clip_rect.h) {
                    //     continue;
                    // }

                    // f32 sample_approx_xf = ceilf(dx + sample.x);
                    // f32 sample_approx_yf = ceilf(dy + sample.y);
                    // s32 sample_approx_x = clamp<s32>((s32)sample_approx_xf, clip_rect.x, clip_rect.x+clip_rect.w);
                    // s32 sample_approx_y = clamp<s32>((s32)sample_approx_yf, clip_rect.y, clip_rect.y+clip_rect.h);
                    if (_framebuffer_scissor_cull(framebuffer, sample_approx_x, sample_approx_y)) continue;
                    _BlendPixel_Scalar(framebuffer, (s32)(sample_approx_x), (s32)(sample_approx_y), sampled_pixel, blend_mode);
                }
            }

        }
    }
#else
    if (angle == 0 && angle_y == 0) {
        for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
            s32 image_sample_y = (s32)fabs(fmodf((src.y + src.h) - ((unclamped_end_y - y_cursor) * scale_ratio_h), image->height));

            if ((flags & DRAW_IMAGE_FLIP_VERTICALLY))
                image_sample_y = (s32)((((unclamped_end_y-1) - y_cursor) * scale_ratio_h) + src.y);

            for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {
                s32 image_sample_x = (s32)fabs(fmodf((src.x + src.w) - ((unclamped_end_x - x_cursor) * scale_ratio_w), image->width));

                if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY))
                    image_sample_x = (s32)((((unclamped_end_x-1) - x_cursor) * scale_ratio_w) + src.x);

                union color32f32 sampled_pixel = color32f32(image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 0] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 1] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 2] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 3] / 255.0f);

                if (shader) {
                    sampled_pixel = shader(framebuffer, sampled_pixel, V2(x_cursor, y_cursor), shader_ctx);
                }
                sampled_pixel.r *= 255.0f;
                sampled_pixel.g *= 255.0f;
                sampled_pixel.b *= 255.0f;
                sampled_pixel.a *= 255.0f;

                sampled_pixel.r *= modulation.r;
                sampled_pixel.g *= modulation.g;
                sampled_pixel.b *= modulation.b;
                sampled_pixel.a *= modulation.a;

                if (_framebuffer_scissor_cull(framebuffer, x_cursor, y_cursor)) continue;
                _BlendPixel_Scalar(framebuffer, x_cursor, y_cursor, sampled_pixel, blend_mode);

            }
        }
    } else {
        // NOTE: this fix needs to be applied to flat quads.
        f32 clamp_start_diff_y = unclamped_start_y - start_y;
        f32 clamp_start_diff_x = unclamped_start_x - start_x;

        for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
            s32 image_sample_y = (s32)fabs(fmodf((src.y + src.h) - ((unclamped_end_y - y_cursor) * scale_ratio_h), image->height));

            if ((flags & DRAW_IMAGE_FLIP_VERTICALLY))
                image_sample_y = (s32)((((unclamped_end_y-1) - y_cursor) * scale_ratio_h) + src.y);

            for (s32 x_cursor = start_x; x_cursor < end_x; ++x_cursor) {
                s32 image_sample_x = (s32)fabs(fmodf((src.x + src.w) - ((unclamped_end_x - x_cursor) * scale_ratio_w), image->width));

                if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY))
                    image_sample_x = (s32)((((unclamped_end_x-1) - x_cursor) * scale_ratio_w) + src.x);

                union color32f32 sampled_pixel = color32f32(image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 0] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 1] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 2] / 255.0f,
                                                            image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 3] / 255.0f);

                if (shader) {
                    sampled_pixel = shader(framebuffer, sampled_pixel, V2(x_cursor, y_cursor), shader_ctx);
                }
                sampled_pixel.r *= 255.0f;
                sampled_pixel.g *= 255.0f;
                sampled_pixel.b *= 255.0f;
                sampled_pixel.a *= 255.0f;

                sampled_pixel.r *= modulation.r;
                sampled_pixel.g *= modulation.g;
                sampled_pixel.b *= modulation.b;
                sampled_pixel.a *= modulation.a;

                s32 adjx  = x_cursor - (unclamped_start_x + origin_off_x);
                s32 adjy  = y_cursor - (unclamped_start_y + origin_off_y);
                f32 dx    = adjx;
                f32 dy    = adjy;
                _rotate_f32_xy_as_pseudo_zyx(&dx, &dy, c, s, c1, s1);

                dx       += (unclamped_start_x + origin_off_x);
                dy       += (unclamped_start_y + origin_off_y);

                /*
                  for rotation need more sampling
                */
                static const V2 subsamples[] = {
                    V2(0, 0),
                    V2(0.5, 0),
                    // V2(0.5, 0.5),
                    V2(0, 0.5),
                    V2(-0.5, 0.0),
                    V2(0.0, -0.5)
                    // V2(-0.5, -0.5)
                };

                for (auto& sample : subsamples) {
                    f32 sample_approx_x = ceilf(dx + sample.x);
                    f32 sample_approx_y = ceilf(dy + sample.y);

                    if (_framebuffer_scissor_cull(framebuffer, sample_approx_x, sample_approx_y)) continue;
                    _BlendPixel_Scalar(framebuffer, (s32)(sample_approx_x), (s32)(sample_approx_y), sampled_pixel, blend_mode);
                }

            }
        }
    }
#endif
}

#ifndef USE_SIMD_OPTIMIZATIONS
void software_framebuffer_draw_image_ext_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y, shader_fn shader, void* shader_ctx) {
    software_framebuffer_draw_image_ext_clipped_scalar(framebuffer, image, destination, src, modulation, flags, blend_mode, clip_rect, origin, angle, angle_y, shader, shader_ctx);
}
#else
/*
  NOTE:
  The SIMD software renderer is only slightly faster, but definitely more stable in framerate,
  however if I need to rotate, I fall back to using the scalar version because my rotation logic is a bit
  harder for me to think of a real SIMD solution for, and I want to make the game so...
*/
void software_framebuffer_draw_image_ext_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, V2 origin, s32 angle, s32 angle_y, shader_fn shader, void* shader_ctx) {
    if (angle_y != 0 || angle != 0) {
        // scalar fallback
        software_framebuffer_draw_image_ext_clipped_scalar(framebuffer, image, destination, src, modulation, flags, blend_mode, clip_rect, origin, angle, angle_y, shader, shader_ctx);
        return;
    }

    if ((destination.x == 0) && (destination.y == 0) && (destination.w == 0) && (destination.h == 0)) {
        destination.w = clip_rect.w;
        destination.h = clip_rect.h;
    }

    if (!rectangle_f32_intersect(destination, clip_rect)) {
        return;
    }

    if ((src.x == 0) && (src.y == 0) && (src.w == 0) && (src.h == 0)) {
        src.w = image->width;
        src.h = image->height;
    }

    f32 scale_ratio_w = (f32)src.w / destination.w;
    f32 scale_ratio_h = (f32)src.h / destination.h;

    __m128i rect_edges_end   = _mm_set_epi32(clip_rect.x+clip_rect.w, clip_rect.y+clip_rect.h, clip_rect.x + clip_rect.w, clip_rect.y + clip_rect.h);
    __m128i rect_edges_start = _mm_set_epi32(clip_rect.x, clip_rect.y, clip_rect.x, clip_rect.y);
    union {
        s32 xyzw[4];
        __m128i vectors;
    } v;
    v.vectors = _mm_set_epi32(destination.x, destination.y, destination.x+destination.w, destination.y+destination.h);
    v.vectors = _mm_min_epi32(_mm_max_epi32(v.vectors, rect_edges_start), rect_edges_end);
    s32 start_x = (v.xyzw)[3];
    s32 start_y = (v.xyzw)[2];
    s32 end_x   = (v.xyzw)[1];
    s32 end_y   = (v.xyzw)[0];

    s32 unclamped_end_x = (s32)(destination.x + destination.w);
    s32 unclamped_end_y = (s32)(destination.y + destination.h);

    u32* framebuffer_pixels_as_32 = (u32*)framebuffer->pixels;
    unused(framebuffer_pixels_as_32);

    s32 stride       = framebuffer->width;
    s32 image_stride = image->width;

    if (image->pot_square_size) {
        image_stride = image->pot_square_size;
    }

    __m128 modulation_r = _mm_load1_ps(&modulation.r);
    __m128 modulation_g = _mm_load1_ps(&modulation.g);
    __m128 modulation_b = _mm_load1_ps(&modulation.b);
    __m128 modulation_a = _mm_load1_ps(&modulation.a);

    // Constants
    __m128 inverse_255    = _mm_set1_ps(1.0 / 255.0f);
    __m128 two_fifty_five = _mm_set1_ps(255);
    __m128 zero           = _mm_set1_ps(0);

    for (s32 y_cursor = start_y; y_cursor < end_y; ++y_cursor) {
        s32 image_sample_y = (s32)fabs(fmodf((src.y + src.h) - ((unclamped_end_y - y_cursor) * scale_ratio_h), image->height));

        if ((flags & DRAW_IMAGE_FLIP_VERTICALLY))
            image_sample_y = (s32)(((unclamped_end_y - y_cursor) * scale_ratio_h) + src.y);

        for (s32 x_cursor = start_x; x_cursor < end_x; x_cursor += 4) {
            s32 image_sample_x  = fmodf((src.x + src.w) - ((unclamped_end_x - (x_cursor))   * scale_ratio_w), image->width);
            s32 image_sample_x1 = fmodf((src.x + src.w) - ((unclamped_end_x - (x_cursor+1)) * scale_ratio_w), image->width);
            s32 image_sample_x2 = fmodf((src.x + src.w) - ((unclamped_end_x - (x_cursor+2)) * scale_ratio_w), image->width);
            s32 image_sample_x3 = fmodf((src.x + src.w) - ((unclamped_end_x - (x_cursor+3)) * scale_ratio_w), image->width);

            if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY)) {
                image_sample_x  = (s32)(((unclamped_end_x - x_cursor) * scale_ratio_w) + src.x);
                image_sample_x1 = (s32)(((unclamped_end_x - (x_cursor + 1)) * scale_ratio_w) + src.x);
                image_sample_x2 = (s32)(((unclamped_end_x - (x_cursor + 2)) * scale_ratio_w) + src.x);
                image_sample_x3 = (s32)(((unclamped_end_x - (x_cursor + 3)) * scale_ratio_w) + src.x);
            }

            union {
                __m128 v;
                union color32f32 c;
            } sampled_pixels[4], destination_pixels[4];

            // Load
            sampled_pixels[0].c = color32f32(
                (f32)image->pixels[image_sample_y * image_stride * 4 + image_sample_x3 * 4 + 0],
                (f32)image->pixels[image_sample_y * image_stride * 4 + image_sample_x3 * 4 + 1],
                (f32)image->pixels[image_sample_y * image_stride * 4 + image_sample_x3 * 4 + 2],
                (f32)image->pixels[image_sample_y * image_stride * 4 + image_sample_x3 * 4 + 3]
            );
            sampled_pixels[1].c = color32f32(
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x2 * 4 + 0],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x2 * 4 + 1],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x2 * 4 + 2],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x2 * 4 + 3]
            );
            sampled_pixels[2].c = color32f32(
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x1 * 4 + 0],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x1 * 4 + 1],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x1 * 4 + 2],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x1 * 4 + 3]
            );
            sampled_pixels[3].c = color32f32(
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 0],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 1],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 2],
                image->pixels[image_sample_y * image_stride * 4 + image_sample_x * 4 + 3]
            );

            destination_pixels[3].c = color32f32(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor) * 4 + 3]
            );
            destination_pixels[2].c = color32f32(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 3]
            );
            destination_pixels[1].c = color32f32(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 3]
            );
            destination_pixels[0].c = color32f32(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 3]
            );

            // Shader Calculation
            // NOTE: outdated sort of, since I don't use it in this engine since it's a very very slow path
            // only kept because it was in legends. Shaders do not assume SIMD unfortunately.
            if (shader) {
                {
                    sampled_pixels[0].v = _mm_mul_ps(sampled_pixels[0].v, inverse_255);
                    sampled_pixels[1].v = _mm_mul_ps(sampled_pixels[1].v, inverse_255);
                    sampled_pixels[2].v = _mm_mul_ps(sampled_pixels[2].v, inverse_255);
                    sampled_pixels[3].v = _mm_mul_ps(sampled_pixels[3].v, inverse_255);
                }
                sampled_pixels[0].c = shader(framebuffer, sampled_pixels[0].c, V2(x_cursor+3, y_cursor), shader_ctx);
                sampled_pixels[1].c = shader(framebuffer, sampled_pixels[1].c, V2(x_cursor+2, y_cursor), shader_ctx);
                sampled_pixels[2].c = shader(framebuffer, sampled_pixels[2].c, V2(x_cursor+1, y_cursor), shader_ctx);
                sampled_pixels[3].c = shader(framebuffer, sampled_pixels[3].c, V2(x_cursor, y_cursor), shader_ctx);
                {
                    sampled_pixels[0].v = _mm_mul_ps(sampled_pixels[0].v, two_fifty_five);
                    sampled_pixels[1].v = _mm_mul_ps(sampled_pixels[1].v, two_fifty_five);
                    sampled_pixels[2].v = _mm_mul_ps(sampled_pixels[2].v, two_fifty_five);
                    sampled_pixels[3].v = _mm_mul_ps(sampled_pixels[3].v, two_fifty_five);
                }
            }

            __m128 red_channels   = _mm_set_ps(sampled_pixels[0].c.r, sampled_pixels[1].c.r, sampled_pixels[2].c.r, sampled_pixels[3].c.r);
            __m128 green_channels = _mm_set_ps(sampled_pixels[0].c.g, sampled_pixels[1].c.g, sampled_pixels[2].c.g, sampled_pixels[3].c.g);
            __m128 blue_channels  = _mm_set_ps(sampled_pixels[0].c.b, sampled_pixels[1].c.b, sampled_pixels[2].c.b, sampled_pixels[3].c.b);
            __m128 alpha_channels = _mm_set_ps(sampled_pixels[0].c.a, sampled_pixels[1].c.a, sampled_pixels[2].c.a, sampled_pixels[3].c.a);

#if 1
            __m128 red_destination_channels = _mm_set_ps(destination_pixels[0].c.r, destination_pixels[1].c.r, destination_pixels[2].c.r, destination_pixels[3].c.r);
            __m128 green_destination_channels = _mm_set_ps(destination_pixels[0].c.g, destination_pixels[1].c.g, destination_pixels[2].c.g, destination_pixels[3].c.g);
            __m128 blue_destination_channels = _mm_set_ps(destination_pixels[0].c.b, destination_pixels[1].c.b, destination_pixels[2].c.b, destination_pixels[3].c.b);
#else
            __m128 red_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 0],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 0]
            );
            __m128 green_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 1],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 1]
            );
            __m128 blue_destination_channels = _mm_set_ps(
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+3) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+2) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+1) * 4 + 2],
                framebuffer->pixels[y_cursor * stride * 4 + (x_cursor+0) * 4 + 2]
            );
#endif

            red_channels   = _mm_mul_ps(modulation_r, red_channels);
            green_channels = _mm_mul_ps(modulation_g, green_channels);
            blue_channels  = _mm_mul_ps(modulation_b, blue_channels);
            alpha_channels = _mm_mul_ps(modulation_a, _mm_mul_ps(inverse_255, alpha_channels));

            /* NOTE this is the only simd optimized procedure since it's the most expensive one */
            /* might not be able to macro the blend modes anyway */

            switch (blend_mode) {
                case BLEND_MODE_NONE: {
                    red_destination_channels   = _mm_mul_ps(red_channels,   alpha_channels);
                    green_destination_channels = _mm_mul_ps(green_channels, alpha_channels);
                    blue_destination_channels  = _mm_mul_ps(blue_channels,  alpha_channels);
                } break;
                case BLEND_MODE_ALPHA: {
                    __m128 one_minus_alpha     = _mm_sub_ps(_mm_set1_ps(1), alpha_channels);
                    red_destination_channels   = _mm_add_ps(_mm_mul_ps(red_destination_channels,   one_minus_alpha),   _mm_mul_ps(red_channels, alpha_channels));
                    green_destination_channels = _mm_add_ps(_mm_mul_ps(green_destination_channels, one_minus_alpha),   _mm_mul_ps(green_channels, alpha_channels));
                    blue_destination_channels  = _mm_add_ps(_mm_mul_ps(blue_destination_channels,  one_minus_alpha),   _mm_mul_ps(blue_channels, alpha_channels));
                } break;
                case BLEND_MODE_ADDITIVE: {
                    red_destination_channels   = _mm_add_ps(red_destination_channels,   _mm_mul_ps(red_channels,   alpha_channels));
                    green_destination_channels = _mm_add_ps(green_destination_channels, _mm_mul_ps(green_channels, alpha_channels));
                    blue_destination_channels  = _mm_add_ps(blue_destination_channels,  _mm_mul_ps(blue_channels,  alpha_channels));
                } break;
                case BLEND_MODE_MULTIPLICATIVE: {
                    red_destination_channels   = _mm_mul_ps(red_destination_channels,   _mm_mul_ps(_mm_mul_ps(red_channels, alpha_channels),   inverse_255));
                    green_destination_channels = _mm_mul_ps(green_destination_channels, _mm_mul_ps(_mm_mul_ps(green_channels, alpha_channels), inverse_255));
                    blue_destination_channels  = _mm_mul_ps(blue_destination_channels,  _mm_mul_ps(_mm_mul_ps(blue_channels,  alpha_channels), inverse_255));
                } break;
                    bad_case;
            }

#define castF32_M128(X) ((f32*)(&X))
            blue_destination_channels  = _mm_min_ps(_mm_max_ps(blue_destination_channels , zero), two_fifty_five);
            green_destination_channels = _mm_min_ps(_mm_max_ps(green_destination_channels, zero), two_fifty_five);
            red_destination_channels   = _mm_min_ps(_mm_max_ps(red_destination_channels  , zero), two_fifty_five);

            for (int i = 0; i < 4; ++i) {
                if (_framebuffer_scissor_cull(framebuffer, x_cursor+i, y_cursor)) continue;
                if ((x_cursor + i >= clip_rect.x+clip_rect.w) || (((src.x + src.w) - ((unclamped_end_x - (x_cursor+i)) * scale_ratio_w))) >= src.x+src.w) break;

                framebuffer->pixels_u32[y_cursor * framebuffer->width + (x_cursor+i)] = packu32(
                    255,
                    castF32_M128(blue_destination_channels)[i],
                    castF32_M128(green_destination_channels)[i],
                    castF32_M128(red_destination_channels)[i]
                );
            }
#undef castF32_M128
        }
    }
}
#endif


void software_framebuffer_draw_line_clipped(struct software_framebuffer* framebuffer, V2 start, V2 end, union color32u8 rgba, u8 blend_mode, struct rectangle_f32 clip_rect) {
    u32 stride = framebuffer->width;

    if (start.y == end.y) {
        if (start.x > end.x) {
            Swap(start.x, end.x, f32);
        }

        for (s32 x_cursor = start.x; x_cursor < end.x; x_cursor++) {
            if (_framebuffer_scissor_cull(framebuffer, x_cursor, start.y)) continue;
            if (x_cursor < clip_rect.w+clip_rect.x && x_cursor >= clip_rect.x &&
                start.y  < clip_rect.h+clip_rect.y && start.y >= clip_rect.y) {
                _BlendPixel_Scalar(framebuffer, x_cursor, (s32)floor(start.y), rgba, blend_mode);
            }
        }
    } else if (start.x == end.x) {
        if (start.y > end.y) {
            Swap(start.y, end.y, f32);
        }
        
        for (s32 y_cursor = start.y; y_cursor < end.y; y_cursor++) {
            if (_framebuffer_scissor_cull(framebuffer, start.x, y_cursor)) continue;
            if (start.x < clip_rect.x+clip_rect.w && start.x    >= clip_rect.x &&
                y_cursor  < clip_rect.y+clip_rect.h && y_cursor >= clip_rect.y) {
                _BlendPixel_Scalar(framebuffer, (s32)floor(start.x), y_cursor, rgba, blend_mode);
            }
        }
    } else {
        s32 x1 = start.x;
        s32 x2 = end.x;
        s32 y1 = start.y;
        s32 y2 = end.y;

        s32 delta_x = abs(x2 - x1);
        s32 delta_y = -abs(y2 - y1);
        s32 sign_x  = 0;
        s32 sign_y  = 0;

        if (x1 < x2) sign_x = 1;
        else         sign_x = -1;

        if (y1 < y2) sign_y = 1;
        else         sign_y = -1;

        s32 error_accumulator = delta_x + delta_y;
        float alpha = rgba.a / 255.0f;

        for (;;) {
            if (x1 < clip_rect.x+clip_rect.w   && x1 >= clip_rect.x && y1 < clip_rect.y+clip_rect.h && y1 >= clip_rect.y) {
                if (_framebuffer_scissor_cull(framebuffer, x1, y1)) continue;
                _BlendPixel_Scalar(framebuffer, x1, y1, rgba, blend_mode);
            }

            if (x1 == x2 && y1 == y2) return;

            s32 old_error_x2 = 2 * error_accumulator;

            if (old_error_x2 >= delta_y) {
                if (x1 != x2) {
                    error_accumulator += delta_y;
                    x1 += sign_x;
                }
            }

            if (old_error_x2 <= delta_x) {
                if (y1 != y2) {
                    error_accumulator += delta_x;
                    y1 += sign_y;
                }
            }
        }
    }
}
void software_framebuffer_draw_line(struct software_framebuffer* framebuffer, V2 start, V2 end, union color32u8 rgba, u8 blend_mode) {
    software_framebuffer_draw_line_clipped(framebuffer, start, end, rgba, blend_mode, rectangle_f32(0, 0, framebuffer->width, framebuffer->height));
}

local void software_framebuffer_draw_glyph_clipped(struct software_framebuffer* framebuffer, struct font_cache* font, float scale, V2 xy, s32 character, union color32f32 modulation, u8 blend_mode, struct rectangle_f32 clip_rect) {
    f32 x_cursor = xy.x;
    f32 y_cursor = xy.y;

    software_framebuffer_draw_image_ex_clipped(
        framebuffer, (struct image_buffer*)font,
        rectangle_f32(
            x_cursor, y_cursor,
            font->tile_width  * scale,
            font->tile_height * scale
        ),
        rectangle_f32(
            (character % font->atlas_cols) * font->tile_width,
            (character / font->atlas_cols) * font->tile_height,
            font->tile_width, font->tile_height
        ),
        modulation,
        NO_FLAGS,
        blend_mode,
        clip_rect,
        0,0
    );
}

local void software_framebuffer_draw_text_clipped(struct software_framebuffer* framebuffer, struct font_cache* font, float scale, V2 xy, string text, union color32f32 modulation, u8 blend_mode, u32 flags, struct rectangle_f32 clip_rect) {
    f32 x_cursor = xy.x;
    f32 y_cursor = xy.y;

    for (unsigned index = 0; index < text.length; ++index) {
        if (text.data[index] == '\n') {
            y_cursor += font->tile_height * scale;
            x_cursor =  xy.x;
        } else {
            s32 character_index = text.data[index] - 32;

            software_framebuffer_draw_glyph_clipped(
                framebuffer,
                font,
                scale, V2(x_cursor, y_cursor + (flags & BIT(0)) * GameUI::get_wobbly_factor(index, hash_bytes_fnv1a((u8*) text.data, text.length))),
                character_index,
                modulation,
                blend_mode,
                clip_rect
            );
            x_cursor += font->tile_width * scale;
        }
    }
}

void software_framebuffer_draw_text(struct software_framebuffer* framebuffer, struct font_cache* font, float scale, V2 xy, string text, union color32f32 modulation, u8 blend_mode)  {
    software_framebuffer_draw_text_clipped(framebuffer, font, scale, xy, text, modulation, blend_mode, 0, rectangle_f32(0, 0, framebuffer->width, framebuffer->height));
}

void software_framebuffer_draw_glyph(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, char glyph, union color32f32 modulation, u8 blend_mode) {
    software_framebuffer_draw_glyph_clipped(framebuffer, font, scale, xy, glyph, modulation, blend_mode, rectangle_f32(0, 0, framebuffer->width, framebuffer->height));
}

/* TODO: provide clipped versions */
void software_framebuffer_draw_text_bounds(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, f32 bounds_w, string text, union color32f32 modulation, u8 blend_mode) {
    f32 x_cursor = xy.x;
    f32 y_cursor = xy.y;

    for (unsigned index = 0; index < text.length; ++index) {
        if (text.data[index] == '\n') {
            y_cursor += font->tile_height * scale;
            x_cursor = xy.x;
        } else {
            s32 character_index = text.data[index] - 32;

            software_framebuffer_draw_glyph_clipped(framebuffer, font, scale, V2(x_cursor, y_cursor), character_index, modulation, blend_mode, rectangle_f32(0,0, framebuffer->width, framebuffer->height));
            x_cursor += font->tile_width * scale;

            if (x_cursor >= xy.x+bounds_w) {
                x_cursor = xy.x;
                y_cursor += font->tile_height * scale;
            }
        }
    }
}

void software_framebuffer_draw_text_bounds_centered(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, struct rectangle_f32 bounds, string text, union color32f32 modulation, u8 blend_mode) {
    f32 text_width  = font_cache_text_width(font, text, scale);
    f32 text_height = font_cache_calculate_height_of(font, text, bounds.w, scale);
    V2 centered_starting_position = V2(0,0);

    centered_starting_position.x = bounds.x + (bounds.w/2) - (text_width/2);
    centered_starting_position.y = bounds.y + (bounds.h/2) - (text_height/2);

    software_framebuffer_draw_text_bounds(framebuffer, font, scale, centered_starting_position, bounds.w, text, modulation, blend_mode);
}

void software_framebuffer_copy_into(struct software_framebuffer* target, struct software_framebuffer* source) {
    if (target->width == source->width && target->height == source->height) {
        memory_copy(source->pixels, target->pixels, target->width * target->height * sizeof(u32));
    } else {
        software_framebuffer_draw_image_ex(target, (struct image_buffer*)source, RECTANGLE_F32_NULL, RECTANGLE_F32_NULL, color32f32(1,1,1,1), NO_FLAGS, 0);
    }
}

/* requires an arena because we need an original copy of our framebuffer. */
/* NOTE technically a test of performance to see if this is doomed */
struct postprocess_job_shared {
    /* 
       should allow for job types such as generic shader 
       or specialized cases such as this
    */
    f32* kernel;
    s32  kernel_width;
    s32  kernel_height;

    f32 divisor;
    s32 passes;

    struct software_framebuffer* unaltered_framebuffer_copy;
    struct software_framebuffer* framebuffer;

    f32                          blend_t;
};
struct postprocess_job_details {
    struct postprocess_job_shared* shared;
    struct rectangle_f32         clip_rect;
};

s32 thread_software_framebuffer_kernel_convolution(void* job) {
    struct postprocess_job_details* details = (postprocess_job_details*)job;
    software_framebuffer_kernel_convolution_ex_bounded(*details->shared->unaltered_framebuffer_copy,
                                                       details->shared->framebuffer,
                                                       details->shared->kernel,
                                                       details->shared->kernel_width,
                                                       details->shared->kernel_height,
                                                       details->shared->divisor,
                                                       details->shared->blend_t,
                                                       details->shared->passes,
                                                       details->clip_rect);
    return 0;
}

void software_framebuffer_kernel_convolution_ex(Memory_Arena* arena, struct software_framebuffer* framebuffer, f32* kernel, s16 kernel_width, s16 kernel_height, f32 divisor, f32 blend_t, s32 passes) {
#ifndef MULTITHREADED_EXPERIMENTAL
    struct software_framebuffer unaltered_copy = software_framebuffer_create_from_arena(arena, framebuffer->width, framebuffer->height);
    software_framebuffer_copy_into(&unaltered_copy, framebuffer);
    software_framebuffer_kernel_convolution_ex_bounded(unaltered_copy, framebuffer, kernel, kernel_width, kernel_height, divisor, blend_t, passes, rectangle_f32(0,0,framebuffer->width,framebuffer->height));
#else
    struct software_framebuffer unaltered_buffer = software_framebuffer_create_from_arena(arena, framebuffer->width, framebuffer->height);
    software_framebuffer_copy_into(&unaltered_buffer, framebuffer);

    /* We don't handle un-even divisions, which is kind of bad. This is mostly a "proof of concept" */
    s32 JOBS_W      = 4;
    s32 JOBS_H      = 4;
    s32 CLIP_W      = (framebuffer->width)/JOBS_W;
    s32 CLIP_H      = (framebuffer->height)/JOBS_H;
    s32 REMAINDER_W = (framebuffer->width) % JOBS_W;
    s32 REMAINDER_H = (framebuffer->height) % JOBS_H;

    struct postprocess_job_shared shared_buffer;
    shared_buffer.kernel                     = kernel;
    shared_buffer.kernel_width               = kernel_width;
    shared_buffer.kernel_height              = kernel_height;
    shared_buffer.unaltered_framebuffer_copy = &unaltered_buffer;
    shared_buffer.framebuffer                = framebuffer;
    shared_buffer.blend_t                    = blend_t;
    shared_buffer.divisor                    = divisor;
    shared_buffer.passes                     = passes;

    struct postprocess_job_details* job_buffers = (postprocess_job_details*)arena->push_unaligned(sizeof(*job_buffers) * (JOBS_W*JOBS_H));

#if 0
    _debugprintf("%d, %d (%d r, %d r)", framebuffer->width, framebuffer->height, REMAINDER_W, REMAINDER_H);
#endif
    for (s32 y = 0; y < JOBS_H; ++y) {
        for (s32 x = 0; x < JOBS_W; ++x) {
            struct rectangle_f32            clip_rect      = rectangle_f32((f32)x * CLIP_W, (f32)y * CLIP_H, (f32)CLIP_W, (f32)CLIP_H);
            struct postprocess_job_details* current_buffer = &job_buffers[y*JOBS_W+x];

            if (x == JOBS_W-1) {
                clip_rect.w += REMAINDER_W;
            }

            if (y == JOBS_H-1) {
                clip_rect.h += REMAINDER_H;
            }

            {
                current_buffer->shared                     = &shared_buffer;
                current_buffer->clip_rect                  = clip_rect;
            }

            Thread_Pool::add_job(thread_software_framebuffer_kernel_convolution, current_buffer);
        }
    }

    Thread_Pool::synchronize_tasks();
#endif
}
/* does not thread itself. */
#ifdef USE_SIMD_OPTIMIZATIONS
void software_framebuffer_kernel_convolution_ex_bounded(struct software_framebuffer unaltered_copy, struct software_framebuffer* framebuffer, f32* kernel, s16 kernel_width, s16 kernel_height, f32 divisor, f32 blend_t, s32 passes, struct rectangle_f32 clip) {
#define castF32_M128(X) ((f32*)(&X))
    if (divisor == 0.0) divisor = 1;

    s32 framebuffer_width  = framebuffer->width;
    s32 framebuffer_height = framebuffer->height;

    s32 kernel_half_width =  kernel_width/2;
    s32 kernel_half_height = kernel_height/2;

    __m128 zero             = _mm_set1_ps(0);
    __m128 two_fifty_five   = _mm_set1_ps(255);
    __m128 one_over_divisor = _mm_set1_ps(1/divisor);
    __m128 one_minus_blend_t = _mm_set1_ps(1-blend_t);

    for (s32 pass = 0; pass < passes; pass++) {
        for (s32 y_cursor = clip.y; y_cursor < clip.y+clip.h; ++y_cursor) {
            for (s32 x_cursor = clip.x; x_cursor < clip.w+clip.x; ++x_cursor) {
                __m128 accumulation = _mm_set1_ps(0);

                for (s32 y_cursor_kernel = -kernel_half_height; y_cursor_kernel <= kernel_half_height; ++y_cursor_kernel) {
                    for (s32 x_cursor_kernel = -kernel_half_width; x_cursor_kernel <= kernel_half_width; ++x_cursor_kernel) {
                        s32 sample_x = x_cursor_kernel + x_cursor;
                        s32 sample_y = y_cursor_kernel + y_cursor;

                        __m128 kernel_value = _mm_set1_ps(kernel[(y_cursor_kernel+1) * kernel_width + (x_cursor_kernel+1)]);
                        __m128 pixel_val    = _mm_set_ps(0,
                                                         unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 2],
                                                         unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 1],
                                                         unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 0]);

                        if (sample_x >= 0 && sample_x < framebuffer_width &&
                            sample_y >= 0 && sample_y < framebuffer_height) {
                            accumulation = _mm_add_ps(accumulation, _mm_mul_ps(pixel_val, kernel_value));
                        }
                    }
                }

                accumulation = _mm_min_ps(_mm_max_ps(_mm_mul_ps(accumulation, one_over_divisor), zero), two_fifty_five);

                __m128 pixel_val = _mm_set_ps(0,
                                              framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 2],
                                              framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 1],
                                              framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 0]);
                pixel_val = _mm_add_ps(_mm_mul_ps(pixel_val, one_minus_blend_t), _mm_mul_ps(_mm_set1_ps(blend_t), accumulation));

                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 0] = castF32_M128(pixel_val)[0];
                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 1] = castF32_M128(pixel_val)[1];
                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 2] = castF32_M128(pixel_val)[2];
            }
        }
    }
#undef castF32_M128
}
#else
void software_framebuffer_kernel_convolution_ex_bounded(struct software_framebuffer unaltered_copy, struct software_framebuffer* framebuffer, f32* kernel, s16 kernel_width, s16 kernel_height, f32 divisor, f32 blend_t, s32 passes, struct rectangle_f32 clip) {
    if (divisor == 0.0) divisor = 1;
    s32 framebuffer_width  = framebuffer->width;
    s32 framebuffer_height = framebuffer->height;

    s32 kernel_half_width =  kernel_width/2;
    s32 kernel_half_height = kernel_height/2;

    for (s32 pass = 0; pass < passes; pass++) {
        for (s32 y_cursor = clip.y; y_cursor < clip.y+clip.h; ++y_cursor) {
            for (s32 x_cursor = clip.x; x_cursor < clip.w+clip.x; ++x_cursor) {
                s32 accumulation[3] = {};

                for (s32 y_cursor_kernel = -kernel_half_height; y_cursor_kernel <= kernel_half_height; ++y_cursor_kernel) {
                    for (s32 x_cursor_kernel = -kernel_half_width; x_cursor_kernel <= kernel_half_width; ++x_cursor_kernel) {
                        s32 sample_x = x_cursor_kernel + x_cursor;
                        s32 sample_y = y_cursor_kernel + y_cursor;

                        if (sample_x >= 0 && sample_x < framebuffer_width &&
                            sample_y >= 0 && sample_y < framebuffer_height)
                        {
                            accumulation[0] += unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 0] * kernel[(y_cursor_kernel+1) * kernel_width + (x_cursor_kernel+1)];
                            accumulation[1] += unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 1] * kernel[(y_cursor_kernel+1) * kernel_width + (x_cursor_kernel+1)];
                            accumulation[2] += unaltered_copy.pixels[sample_y * framebuffer_width * 4 + sample_x * 4 + 2] * kernel[(y_cursor_kernel+1) * kernel_width + (x_cursor_kernel+1)];
                        }
                    }
                }

                accumulation[0] = clamp<s32>(accumulation[0] / divisor, 0, 255);
                accumulation[1] = clamp<s32>(accumulation[1] / divisor, 0, 255);
                accumulation[2] = clamp<s32>(accumulation[2] / divisor, 0, 255);

                /* NOTE does not blend any pixels, other than what's in blend_t, but that's a convenience thing sort of. */
                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 0] = framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 0] * (1 - blend_t) + (blend_t * accumulation[0]);
                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 1] = framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 1] * (1 - blend_t) + (blend_t * accumulation[1]);
                framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 2] = framebuffer->pixels[y_cursor * framebuffer_width * 4 + x_cursor * 4 + 2] * (1 - blend_t) + (blend_t * accumulation[2]);
            }
        }
    }
}
#endif

struct run_shader_job_details_shared {
    struct software_framebuffer* framebuffer;
    shader_fn shader;
    void* context;
};

struct run_shader_job_details {
    struct run_shader_job_details_shared* shared;
    struct rectangle_f32 src_rect;
};

s32 thread_software_framebuffer_run_shader(void* context) {
    struct run_shader_job_details* job_details = (run_shader_job_details*)context;
    struct rectangle_f32 src_rect = job_details->src_rect;

    for (s32 y = src_rect.y; y < src_rect.y+src_rect.h; ++y) {
        for (s32 x = src_rect.x; x < src_rect.x+src_rect.w; ++x) {
            union color32f32 source_pixel = color32f32(
                job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 0] / 255.0f,
                job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 1] / 255.0f,
                job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 2] / 255.0f,
                job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 3] / 255.0f
            );

            union color32f32 new_pixel = job_details->shared->shader(job_details->shared->framebuffer, source_pixel, V2(x, y), job_details->shared->context);

            new_pixel.r = clamp<f32>(new_pixel.r, 0, 1);
            new_pixel.g = clamp<f32>(new_pixel.g, 0, 1);
            new_pixel.b = clamp<f32>(new_pixel.b, 0, 1);
            new_pixel.a = clamp<f32>(new_pixel.a, 0, 1);

            job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 0] = new_pixel.r * 255.0f;
            job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 1] = new_pixel.g * 255.0f;
            job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 2] = new_pixel.b * 255.0f;
            job_details->shared->framebuffer->pixels[y * job_details->shared->framebuffer->width * 4 + x * 4 + 3] = new_pixel.a * 255.0f;
        }
    }

    return 0;
}

void software_framebuffer_run_shader(struct software_framebuffer* framebuffer, struct rectangle_f32 src_rect, shader_fn shader, void* context) {
#ifndef MULTITHREADED_EXPERIMENTAL
    for (s32 y = src_rect.y; y < src_rect.y+src_rect.h; ++y) {
        for (s32 x = src_rect.x; x < src_rect.x+src_rect.w; ++x) {
            union color32f32 source_pixel = color32f32(
                framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 0] / 255.0f,
                framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 1] / 255.0f,
                framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 2] / 255.0f,
                framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 3] / 255.0f
            );

            union color32f32 new_pixel = shader(framebuffer, source_pixel, V2(x, y), context);

            new_pixel.r = clamp<f32>(new_pixel.r, 0, 1);
            new_pixel.g = clamp<f32>(new_pixel.g, 0, 1);
            new_pixel.b = clamp<f32>(new_pixel.b, 0, 1);
            new_pixel.a = clamp<f32>(new_pixel.a, 0, 1);

            framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 0] = new_pixel.r * 255.0f;
            framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 1] = new_pixel.g * 255.0f;
            framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 2] = new_pixel.b * 255.0f;
            framebuffer->pixels[y * framebuffer->width * 4 + x * 4 + 3] = new_pixel.a * 255.0f;
        }
    }
#else
    s32 JOBS_W      = 4;
    s32 JOBS_H      = 4;
    s32 CLIP_W      = (framebuffer->width)/JOBS_W;
    s32 CLIP_H      = (framebuffer->height)/JOBS_H;
    s32 REMAINDER_W = (framebuffer->width) % JOBS_W;
    s32 REMAINDER_H = (framebuffer->height) % JOBS_H;

    struct run_shader_job_details_shared shared_buffer;
    shared_buffer.framebuffer = framebuffer;
    shared_buffer.context     = context;
    shared_buffer.shader      = shader;

    struct run_shader_job_details* job_buffers = (run_shader_job_details*)Global_Engine()->scratch_arena.push_unaligned(sizeof(*job_buffers) * (JOBS_W*JOBS_H));

    for (s32 y = 0; y < JOBS_H; ++y) {
        for (s32 x = 0; x < JOBS_W; ++x) {
            struct rectangle_f32            clip_rect = rectangle_f32(x * CLIP_W, y * CLIP_H, CLIP_W, CLIP_H);
            struct run_shader_job_details*  current_buffer = &job_buffers[y*JOBS_W+x];

            if (x == JOBS_W-1) {
                clip_rect.w += REMAINDER_W;
            }

            if (y == JOBS_H-1) {
                clip_rect.h += REMAINDER_H;
            }

            {
                current_buffer->shared                     = &shared_buffer;
                current_buffer->src_rect                  = clip_rect;
            }

            Thread_Pool::add_job(thread_software_framebuffer_run_shader, current_buffer);
        }
    }

    Thread_Pool::synchronize_tasks();
#endif
}

#include "software_render_commands_implementation.cpp"

#undef _BlendPixel_Scalar
