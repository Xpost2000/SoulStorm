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
