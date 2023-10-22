#ifndef GRAPHICS_COMMON_H
#define GRAPHICS_COMMON_H

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

#endif
