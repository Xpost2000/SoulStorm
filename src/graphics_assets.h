#ifndef GRAPHICS_ASSETS_H
#define GRAPHICS_ASSETS_H

#include "common.h"

#include "image_buffer.h"
#include "font_cache.h"

#include "string.h"
#include "memory_arena.h"

#include "v2.h"

typedef struct image_id { s32 index=0; } image_id;
typedef struct font_id  { s32 index=0; } font_id;
typedef struct sprite_id { s32 index=0; } sprite_id;

struct Texture_Atlas_Sub_Image {
    image_id original_asset;     // Should be unloaded (not paged in memory), but metadata is still present
    rectangle_f32 subrectangle;
};

// Obtain subrectangles by using the original asset as a key. This allows me to gradually migrate the code
// to start using the atlases.

/*
  NOTE:

  These are not intended to be a separate asset type. This is just "semantic" information
  attached to an image_id.

  Which is a bit unusual, but I do not see the need to complicate the implementation of graphics
  driver by separating textures and atlases.
*/
struct Texture_Atlas { 
    image_id                 atlas_image_id;
    s32                      subimage_count;
    Texture_Atlas_Sub_Image* subimages;

    rectangle_f32 get_subrect(image_id subimage);
};

// This is new code, compared to the stuff below, LOL
struct Sprite_Frame {
    image_id             img;
    // NOTE: you can query all information from using the image_id...
    struct rectangle_f32 source_rect = RECTANGLE_F32_NULL;
    f32                  frame_length = 0.0f;
};

struct Sprite {
    u32           frame_count;
    Sprite_Frame* frames;
};

struct graphics_assets;
struct Sprite_Instance {
    sprite_id id;
    V2        offset;
    V2        scale;
    s32       frame;
    f32       frame_timer;

    // if anim_t == -1.0, use the time in the frame data.
    // otherwise, we override with our own fixed timer.
    // start and end, are the frames we animate between.
    void animate(struct graphics_assets* graphics_assets, f32 dt, f32 anim_t=-1.0f, s32 start = -1, s32 end = -1);
};

Sprite_Instance sprite_instance(sprite_id id);

// the principle unit is the image_buffer,
// since it's the only "real" thing being loaded.
// NOTE: for the needs of the game, I only need to unload assets that
// the engine loads from a level.
enum asset_status {
    ASSET_STATUS_UNLOADED,
    ASSET_STATUS_LOADED,
    ASSET_STATUS_PERMENANTLY_LOADED,
};

class Graphics_Driver;
struct graphics_assets {
    Memory_Arena* arena;

    u32                  font_capacity;
    u32                  font_count;
    u32                  image_count;
    u32                  image_capacity;
    struct image_buffer* images;
    u8*                  image_asset_status;
    struct font_cache*   fonts;
    string*              image_file_strings;

    u32                  sprite_capacity;
    u32                  sprite_count;
    Sprite*              sprites;

    Graphics_Driver* graphics_driver;
};

struct font_cache   font_cache_load_bitmap_font(string filepath, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
void                font_cache_free(struct font_cache* font_cache);
f32                 font_cache_text_height(struct font_cache* font_cache);
f32                 font_cache_text_width(struct font_cache* font_cache, string text, f32 scale);
f32                 font_cache_calculate_height_of(struct font_cache* font_cache, string str, f32 width_bounds, f32 scale);

struct image_buffer image_buffer_create_blank(u32 width, u32 height);
struct image_buffer image_buffer_load_from_file(string file_path);
void                image_buffer_pad_to_POT(struct image_buffer* image);
void                image_buffer_write_to_disk(struct image_buffer* image, string as);
void                image_buffer_free(struct image_buffer* image);

struct graphics_assets graphics_assets_create(Memory_Arena* arena, u32 font_limit, u32 image_limit, u32 sprite_limit);

sprite_id              graphics_assets_alloc_sprite(struct graphics_assets* assets, u32 frames);
Sprite*                graphics_get_sprite_by_id(struct graphics_assets* assets, sprite_id id);
Sprite_Frame*          sprite_get_frame(Sprite* sprite, s32 index);

void                   graphics_assets_finish(struct graphics_assets* assets);
image_id               graphics_assets_load_image(struct graphics_assets* assets, string path);
void                   graphics_assets_unload_image(struct graphics_assets* assets, image_id img);

void                   graphics_assets_update_graphics_driver(struct graphics_assets* assets, Graphics_Driver* driver);

image_id               graphics_assets_get_image_by_filepath(struct graphics_assets* assets, string filepath);
font_id                graphics_assets_load_bitmap_font(struct graphics_assets* assets, string path, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
struct font_cache*     graphics_assets_get_font_by_id(struct graphics_assets* assets, font_id font);
struct image_buffer*   graphics_assets_get_image_by_id(struct graphics_assets* assets, image_id image);

// Texture Atlas procedures
/*
 * NOTE: in reality, I should've also made Sprites work the same way, however it's annoying to change this right now,
 * and it's not needed. However, I would probably just keep graphics_assets to basically only images and fonts, since those are distinct
 * enough to really matter.
 *
 * Most other assets are compound assets so they would not matter so much.
 *
 * NOTE: unloading a texture atlas is undefined since this is intended to be used on 
 *       permenant assets, which are never unloaded.
 *
 *       It is possible to reload an atlas technically as long as you keep the original atlas object, but
 *       that would be very odd to use.
 */
Texture_Atlas          graphics_assets_construct_texture_atlas_image(struct graphics_assets* assets, image_id* images, size_t image_count);
void                   graphics_assets_texture_atlas_unload_original_subimages(struct graphics_assets* assets, const Texture_Atlas& texture_atlas);

#endif
