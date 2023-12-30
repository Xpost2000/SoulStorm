#ifndef COLOR_H
#define COLOR_H

#include "common.h"

union color32u8 {
    struct { u8 r, g, b, a; };
    u8  rgba[4];
    u32 rgba_packed;
};

/* for percentage and modulations */
union color32f32 {
    struct { f32 r, g, b, a; };
    f32  rgba[4];
};
/* NOTE: for optimizing out floating point operations with close enough integer ops... */
union color32s32 {
    struct { s32 r, g, b, a; };
    s32  rgba[4];
};

#define color32u8(R,G,B,A)  color32u8{(u8)(R),(u8)(G),(u8)(B),(u8)(A)}
#define color32f32(R,G,B,A) color32f32{(f32)(R),(f32)(G),(f32)(B),(f32)(A)}
#define color32s32(R,G,B,A) color32s32{(s32)(R),(s32)(G),(s32)(B),(s32)(A)}
color32f32 color32s32_to_color32f32(color32u8 source);
color32f32 color32u8_to_color32f32(color32u8 source);
color32s32 color32u8_to_color32s32(color32u8 source);
color32s32 color32f32_to_color32s32(color32f32 source);
color32u8 color32f32_to_color32u8(color32f32 source);
color32u8 multi_linear_gradient_blend(Slice<color32u8> gradients, f32 normalized_t);

#define color32u8_WHITE  color32u8(255,255,255,255)
#define color32u8_BLACK  color32u8(0,0,0,255)
#define color32f32_WHITE color32f32(1,1,1,1)
#define color32f32_BLACK color32f32(0,0,0,1)

#define color32f32_LEGENDS_AQUAGREEN (color32f32(17/255.0f, 112/255.0f, 76/255.0f, 1.0))
#define color32f32_LEGENDS_BLUE      (color32f32(34/255.0f, 37/255.0f, 143/255.0f, 1.0))
#define color32f32_DEFAULT_UI_COLOR  color32f32_LEGENDS_BLUE
/* #define color32f32_DEFAULT_UI_COLOR  color32f32_LEGENDS_AQUAGREEN */

#endif
