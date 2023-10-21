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
#include "prng.h"

#include "string.h"
#include "v2.h"

#include "camera.h"
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

struct software_framebuffer;
struct image_buffer {
    u32 width;
    u32 height;
    union {
        u8* pixels;
        u32* pixels_u32;
    };
};

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

typedef color32f32 (*shader_fn)(software_framebuffer* framebuffer, color32f32 source_pixel, V2 pixel_position, void* context);

#define color32u8(R,G,B,A)  color32u8{(u8)(R),(u8)(G),(u8)(B),(u8)(A)}
#define color32f32(R,G,B,A) color32f32{(f32)(R),(f32)(G),(f32)(B),(f32)(A)}
#define color32s32(R,G,B,A) color32s32{(s32)(R),(s32)(G),(s32)(B),(s32)(A)}
color32f32 color32s32_to_color32f32(color32u8 source);
color32f32 color32u8_to_color32f32(color32u8 source);
color32s32 color32u8_to_color32s32(color32u8 source);
color32s32 color32f32_to_color32s32(color32f32 source);
color32u8 color32f32_to_color32u8(color32f32 source);

#define color32u8_WHITE  color32u8(255,255,255,255)
#define color32u8_BLACK  color32u8(0,0,0,255)
#define color32f32_WHITE color32f32(1,1,1,1)
#define color32f32_BLACK color32f32(0,0,0,1)

/*
  While this isn't strictly necessary for the graphics part,
  this is an auxiliary data structure used to keep my lighting happening in one pass.

  if a pixel is marked as 255, we will not light it, and draw it as full bright.
  Otherwise behavior is just lighting. Not sure what to do with non-255 values.

  Okay it really seems like I'm just using this as a stencil buffer...
*/
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

enum font_cache_type {
    FONT_CACHE_ATLAS_FIXED_ASCII,
    FONT_CACHE_ATLAS_VARIABLE,
};

/* This currently only works for monospace fonts. */
/* which is fine for this type of game, but a proper font cache is a bit more complicated */
/* and requires a lot more packing */
struct font_cache : public image_buffer {
    s8 type;
    union {
        struct {
            s32 tile_width;
            s32 tile_height;
            s32 atlas_rows;
            s32 atlas_cols;
        };
        struct rectangle_s16 glyphs[128];
    };
};

struct font_cache font_cache_load_bitmap_font(string filepath, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
void              font_cache_free(struct font_cache* font_cache);
f32               font_cache_text_height(struct font_cache* font_cache);
f32               font_cache_text_width(struct font_cache* font_cache, string text, f32 scale);
f32               font_cache_calculate_height_of(struct font_cache* font_cache, string str, f32 width_bounds, f32 scale);

struct image_buffer image_buffer_load_from_file(string file_path);
void                image_buffer_write_to_disk(struct image_buffer* image, string as);
void                image_buffer_free(struct image_buffer* image);

typedef struct image_id { s32 index; } image_id;
typedef struct font_id  { s32 index; } font_id;

struct graphics_assets {
    Memory_Arena* arena;

    u32                  font_capacity;
    u32                  font_count;
    u32                  image_count;
    u32                  image_capacity;
    struct image_buffer* images;
    struct font_cache*   fonts;
    string*              image_file_strings;
};

struct graphics_assets graphics_assets_create(Memory_Arena* arena, u32 font_limit, u32 image_limit);
void                   graphics_assets_finish(struct graphics_assets* assets);
image_id               graphics_assets_load_image(struct graphics_assets* assets, string path);
image_id               graphics_assets_get_image_by_filepath(struct graphics_assets* assets, string filepath);
font_id                graphics_assets_load_bitmap_font(struct graphics_assets* assets, string path, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
struct font_cache*     graphics_assets_get_font_by_id(struct graphics_assets* assets, font_id font);
struct image_buffer*   graphics_assets_get_image_by_id(struct graphics_assets* assets, image_id image);

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

struct software_framebuffer software_framebuffer_create(u32 width, u32 height);
struct software_framebuffer software_framebuffer_create_from_arena(Memory_Arena* arena, u32 width, u32 height);
void                        software_framebuffer_finish(struct software_framebuffer* framebuffer);
void                        software_framebuffer_copy_into(struct software_framebuffer* target, struct software_framebuffer* source);
void                        software_framebuffer_clear_scissor(struct software_framebuffer* framebuffer);
void                        software_framebuffer_set_scissor(struct software_framebuffer* framebuffer, s32 x, s32 y, s32 w, s32 h);
void                        software_framebuffer_clear_buffer(struct software_framebuffer* framebuffer, union color32u8 rgba);
void                        software_framebuffer_draw_quad(struct software_framebuffer* framebuffer, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode);
void                        software_framebuffer_draw_image_ex(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode);
void                        software_framebuffer_draw_image_ex_clipped(struct software_framebuffer* framebuffer, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 src, union color32f32 modulation, u32 flags, u8 blend_mode, struct rectangle_f32 clip_rect, shader_fn shader, void* shader_ctx);
void                        software_framebuffer_draw_glyph(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, char glyph, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_line(struct software_framebuffer* framebuffer, V2 start, V2 end, union color32u8 rgba, u8 blend_mode);
void                        software_framebuffer_draw_text(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, string text, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_text_bounds(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, V2 xy, f32 bounds_w, string cstring, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_draw_text_bounds_centered(struct software_framebuffer* framebuffer, struct font_cache* font, f32 scale, struct rectangle_f32 bounds, string text, union color32f32 modulation, u8 blend_mode);
void                        software_framebuffer_kernel_convolution_ex(Memory_Arena* arena, struct software_framebuffer* framebuffer, f32* kernel, s16 width, s16 height, f32 divisor, f32 blend_t, s32 passes);
void                        software_framebuffer_run_shader(struct software_framebuffer* framebuffer, struct rectangle_f32 src_rect, shader_fn shader, void* context);

enum render_command_type{
    RENDER_COMMAND_DRAW_QUAD,
    RENDER_COMMAND_DRAW_IMAGE,
    RENDER_COMMAND_DRAW_TEXT,
    RENDER_COMMAND_DRAW_LINE
};
#define ALWAYS_ON_TOP (INFINITY)
struct render_command {
    /* easier to mix using a floating point value. */
    shader_fn shader;
    void*     shader_ctx;

    union {
        struct {
            struct font_cache*   font;
            string          text;
        };
        struct image_buffer* image;
    };

    struct rectangle_f32 destination;
    struct rectangle_f32 source;
    union {
        V2 start;
        V2 xy;
    };

    V2 end;

    f32 scale;
    u32 flags;
    union {
        union color32u8  modulation_u8;
    };

    s16 type;
    u8  blend_mode;
};

struct render_commands {
    render_commands() {
        clear_buffer_color = color32u8_BLACK;
        should_clear_buffer = command_count = command_capacity = 0;
    }
    struct camera          camera;
    u8                     should_clear_buffer;
    union color32u8        clear_buffer_color;
    struct render_command* commands;
    s32                    command_count;
    s32                    command_capacity;
};

struct render_commands render_commands(Memory_Arena* arena, s32 capacity, struct camera camera);

void render_commands_push_quad(struct render_commands* commands, struct rectangle_f32 destination, union color32u8 rgba, u8 blend_mode);
void render_commands_push_image(struct render_commands* commands, struct image_buffer* image, struct rectangle_f32 destination, struct rectangle_f32 source, union color32f32 rgba, u32 flags, u8 blend_mode);
void render_commands_push_line(struct render_commands* commands, V2 start, V2 end, union color32u8 rgba, u8 blend_mode);
void render_commands_push_text(struct render_commands* commands, struct font_cache* font, f32 scale, V2 xy, string cstring, union color32f32 rgba, u8 blend_mode);

void render_commands_set_shader(struct render_commands* commands, shader_fn shader, void* context);
void render_commands_clear(struct render_commands* commands);

void software_framebuffer_kernel_convolution_ex_bounded(struct software_framebuffer before, struct software_framebuffer* framebuffer, f32* kernel, s16 kernel_width, s16 kernel_height, f32 divisor, f32 blend_t, s32 passes, struct rectangle_f32 clip);

void software_framebuffer_render_commands(struct software_framebuffer* framebuffer, struct render_commands* commands);

#endif
