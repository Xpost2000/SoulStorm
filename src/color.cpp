#include "color.h"

// color helpers
color32f32 color32s32_to_color32f32(color32u8 source) {
    return color32f32(source.r / 255.0f, source.g / 255.0f, source.b / 255.0f, source.a / 255.0f);
}
color32f32 color32u8_to_color32f32(color32u8 source) {
    return color32f32(source.r / 255.0f, source.g / 255.0f, source.b / 255.0f, source.a / 255.0f);
}
color32s32 color32u8_to_color32s32(color32u8 source) {
    return color32s32(source.r, source.g, source.b, source.a);
}

color32s32 color32f32_to_color32s32(color32f32 source) {
    return color32s32(source.r * 255, source.g * 255, source.b * 255, source.a * 255);
}
color32u8 color32f32_to_color32u8(color32f32 source) {
    return color32u8(source.r * 255, source.g * 255, source.b * 255, source.a * 255);
}

color32u8 multi_linear_gradient_blend(Slice<color32u8> gradients, f32 normalized_t) {
    normalized_t = clamp<f32>(normalized_t, 0.0f, 1.0f);
    f32 slice_per_color = 1.0f / gradients.length;
    s32 start_index     = min((s32)floorf(normalized_t / slice_per_color), gradients.length-1);
    s32 next_index      = min<s32>(start_index+1, gradients.length-1);

    auto blend0 = gradients[start_index];
    auto blend1 = gradients[next_index];

    f32 blend_t = (fmodf(normalized_t, slice_per_color)) / slice_per_color;

    return color32u8(
        lerp_s32(blend0.r, blend1.r, blend_t),
        lerp_s32(blend0.g, blend1.g, blend_t),
        lerp_s32(blend0.b, blend1.b, blend_t),
        lerp_s32(blend0.a, blend1.a, blend_t)
    );
}
