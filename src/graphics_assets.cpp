#include "graphics_assets.h"
#include "memory_arena.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Image Buffer

/* doesn't take an allocator because of the way stb image works... */
struct image_buffer image_buffer_load_from_file(string filepath) {
    s32 width;
    s32 height;
    s32 components;

#ifndef EXPERIMENTAL_VFS
    u8* image_buffer = stbi_load(filepath.data, &width, &height, &components, 4);
#else
    /* STBIDEF stbi_uc *stbi_load_from_memory   (stbi_uc           const *buffer, int len   , int *x, int *y, int *channels_in_file, int desired_channels); */
    u8* image_buffer = NULL;
    {
        struct file_buffer buffer = read_entire_file(memory_arena_allocator(&game_arena), filepath);
        _debugprintf("%p, %d", buffer.buffer, (s32)buffer.length);
        image_buffer = stbi_load_from_memory(buffer.buffer, buffer.length, &width, &height, &components, 4);
        file_buffer_free(&buffer);
    }
#endif
    _debugprintf("tried to load: \"%.*s\"", filepath.length, filepath.data);
    if (!image_buffer) {
        _debugprintf("Failed to load \"%.*s\"", filepath.length, filepath.data);
    }
    assertion(image_buffer && "image load failed!");
    struct image_buffer result;
    result.pixels = image_buffer;
    result.width  = width;
    result.height = height;
    return result;
}

void image_buffer_write_to_disk(struct image_buffer* image, string as) {
    char filename[256] = {};
#if 0
    snprintf(filename, 256, "%s.bmp", as.data);
    stbi_write_bmp(filename, image->width, image->height, 4, image->pixels);
#else
    snprintf(filename, 256, "%s.jpg", as.data);
    stbi_write_jpg(filename, image->width, image->height, 4, image->pixels, 3);
#endif
    _debugprintf("screenshot produced.");
}

void image_buffer_free(struct image_buffer* image) {
    if (image->pixels) {
        free(image->pixels);
        image->pixels = 0;
    }
}

// Font Cache
struct font_cache font_cache_load_bitmap_font(string filepath, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns) {
    struct font_cache result = {};
    result.type        = FONT_CACHE_ATLAS_FIXED_ASCII;
    result.tile_width  = tile_width;
    result.tile_height = tile_height;
    result.atlas_rows  = atlas_rows;
    result.atlas_cols  = atlas_columns;

    struct image_buffer atlas_image = image_buffer_load_from_file(filepath);

    result.width  = atlas_image.width;
    result.height = atlas_image.height;
    result.pixels = atlas_image.pixels;

    return result;
}

void font_cache_free(struct font_cache* font_cache) {
    image_buffer_free((struct image_buffer*) font_cache); 
}

f32 font_cache_text_height(struct font_cache* font_cache) {
    return font_cache->tile_height;
}

f32 font_cache_calculate_height_of(struct font_cache* font_cache, string str, f32 width_bounds, f32 scale) {
    f32 font_height = font_cache_text_height(font_cache);

    s32 units = 1;

    if (width_bounds == 0.0f) {
        for (s32 index = 0; index < str.length; ++index) {
            if (str.data[index] == '\n')
                units++;
        }
    } else {
        f32 cursor_x = 0;
        for (s32 index = 0; index < str.length; ++index) {
            cursor_x += font_cache->tile_width * scale;

            if (str.data[index] == '\n') {
                cursor_x = 0;
                units++;
            }

            if (cursor_x >= width_bounds) {
                cursor_x = 0;
                units++;
            }
        }
    }

    return units * font_height * scale;
}

f32 font_cache_text_width(struct font_cache* font_cache, string text, f32 scale) {
    /* NOTE font_caches are monospaced */
    return font_cache->tile_width * text.length * scale;
}

// Graphics Assets

struct graphics_assets graphics_assets_create(Memory_Arena* arena, u32 font_limit, u32 image_limit) {
    struct graphics_assets assets = {
        .font_capacity  = font_limit,
        .image_capacity = image_limit,
    };

    assets.arena              = arena;
    assets.images             = (image_buffer*)arena->push_unaligned(sizeof(*assets.images) * image_limit);
    assets.image_file_strings = (string*)arena->push_unaligned(sizeof(*assets.image_file_strings) *image_limit);
    assets.fonts              = (font_cache*)arena->push_unaligned(sizeof(*assets.fonts)  * font_limit);

    return assets;
}

void graphics_assets_finish(struct graphics_assets* assets) {
    for (unsigned image_index = 0; image_index < assets->image_count; ++image_index) {
        struct image_buffer* image = assets->images + image_index;
        _debugprintf("destroying img: %p (%d) (%dx%d %p)", image, image_index, image->width, image->height, image->pixels);
        image_buffer_free(image);
    }
    for (unsigned font_index = 0; font_index < assets->font_count; ++font_index) {
        struct font_cache* font = assets->fonts + font_index;
        _debugprintf("destroying font: %p (%d)", font, font_index);
        font_cache_free(font);
    }
}

image_id graphics_assets_load_image(struct graphics_assets* assets, string path) {
    for (s32 index = 0; index < assets->image_count; ++index) {
        string filepath = assets->image_file_strings[index];

        if (string_equal(path, filepath)) {
            return image_id{.index = index+1};
        }
    }

    image_id new_id = image_id { .index = (s32)assets->image_count + 1 };
    _debugprintf("img loaded: %.*s", path.length, path.data);

    struct image_buffer* new_image           = &assets->images[assets->image_count];
    string*              new_filepath_string = &assets->image_file_strings[assets->image_count++];
    *new_filepath_string                     = memory_arena_push_string(assets->arena, path);
    *new_image                               = image_buffer_load_from_file(*new_filepath_string);

    return new_id;
}

font_id graphics_assets_load_bitmap_font(struct graphics_assets* assets, string path, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns) {
    font_id new_id = font_id { .index = (s32)assets->font_count + 1 };

    struct font_cache* new_font   = &assets->fonts[assets->font_count++];
    *new_font                     = font_cache_load_bitmap_font(path, tile_width, tile_height, atlas_rows, atlas_columns);

    return new_id;
}

struct font_cache* graphics_assets_get_font_by_id(struct graphics_assets* assets, font_id font) {
    assertion(font.index > 0 && font.index <= assets->font_count);
    return &assets->fonts[font.index-1];
}

struct image_buffer* graphics_assets_get_image_by_id(struct graphics_assets* assets, image_id image) {
    assertion(image.index > 0 && image.index <= assets->image_count);
    return &assets->images[image.index-1];
}

image_id graphics_assets_get_image_by_filepath(struct graphics_assets* assets, string filepath) {
    assertion(filepath.data && filepath.length && "Bad string?");
    return graphics_assets_load_image(assets, filepath);
}

