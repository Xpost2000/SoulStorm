#ifndef GRAPHICS_ASSETS_H
#define GRAPHICS_ASSETS_H

#include "common.h"

#include "image_buffer.h"
#include "font_cache.h"

#include "string.h"
#include "memory_arena.h"

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

struct font_cache   font_cache_load_bitmap_font(string filepath, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
void                font_cache_free(struct font_cache* font_cache);
f32                 font_cache_text_height(struct font_cache* font_cache);
f32                 font_cache_text_width(struct font_cache* font_cache, string text, f32 scale);
f32                 font_cache_calculate_height_of(struct font_cache* font_cache, string str, f32 width_bounds, f32 scale);

struct image_buffer image_buffer_load_from_file(string file_path);
void                image_buffer_write_to_disk(struct image_buffer* image, string as);
void                image_buffer_free(struct image_buffer* image);

typedef struct image_id { s32 index; } image_id;
typedef struct font_id  { s32 index; } font_id;

struct graphics_assets graphics_assets_create(Memory_Arena* arena, u32 font_limit, u32 image_limit);
void                   graphics_assets_finish(struct graphics_assets* assets);
image_id               graphics_assets_load_image(struct graphics_assets* assets, string path);
image_id               graphics_assets_get_image_by_filepath(struct graphics_assets* assets, string filepath);
font_id                graphics_assets_load_bitmap_font(struct graphics_assets* assets, string path, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns);
struct font_cache*     graphics_assets_get_font_by_id(struct graphics_assets* assets, font_id font);
struct image_buffer*   graphics_assets_get_image_by_id(struct graphics_assets* assets, image_id image);

#endif
