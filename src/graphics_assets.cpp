#include "graphics_driver.h"
#include "graphics_assets.h"
#include "memory_arena.h"
#include "virtual_file_system.h"

#include "engine.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Image Buffer

// NOTE: I wish I could get these to work with the memory arena system
// but for now that would require changing a lot of code. If I feel like it, I will
// change this in the future.
//
// However, it's really more important that persistent game state uses the arena since it's
// not really deallocated. A lot of this is primarily cruft from Legends anyway, but it does work
// and relatively well enough considering that was more complicated than this, which is why I'm not
// changing much...

/* doesn't take an allocator because of the way stb image works... */

void image_buffer_pad_to_POT(struct image_buffer* image) {
    assert(image->pixels && "This should only be called on an existing image.");

    if (image->pot_square_size != 0) {
        _debugprintf("Image already POT & square, no need to pad");
        return;
    }

    image->pot_square_size = max<u32>(
        nearest_pot32(image->width),
        nearest_pot32(image->height)
    );

    assert(image->pot_square_size >= image->width &&
           image->pot_square_size >= image->height &&
           "Uh, I'm not sure how this happened, but the padded image should be bigger.");

    u8* rescaled_image_buffer = (u8*)malloc(image->pot_square_size * image->pot_square_size * 4);

    zero_memory(rescaled_image_buffer, image->pot_square_size * image->pot_square_size * 4);
    for (u32 y = 0; y < image->height; ++y) {
        for (u32 x = 0; x < image->width; ++x) {
            rescaled_image_buffer[y * (image->pot_square_size*4) + (x*4) + 0] = image->pixels[y * (image->height * 4) + (x * 4) + 0];
            rescaled_image_buffer[y * (image->pot_square_size*4) + (x*4) + 1] = image->pixels[y * (image->height * 4) + (x * 4) + 1];
            rescaled_image_buffer[y * (image->pot_square_size*4) + (x*4) + 2] = image->pixels[y * (image->height * 4) + (x * 4) + 2];
            rescaled_image_buffer[y * (image->pot_square_size*4) + (x*4) + 3] = image->pixels[y * (image->height * 4) + (x * 4) + 3];
        }
    }

    free(image->pixels);
    image->pixels = rescaled_image_buffer;
}

local void _image_buffer_auto_fill_pot_square_size(struct image_buffer* image) {
    if (image->width == image->height && is_pot32(image->width)) {
        image->pot_square_size = image->width;
    }
}

struct image_buffer image_buffer_create_blank(u32 width, u32 height) {
    struct image_buffer result;
    zero_memory(&result, sizeof(result));
    result.width = width;
    result.height = height;
    result.pixels = (u8*)malloc(width * height * 4);
    result.pot_square_size = 0;
    zero_memory(result.pixels, width * height * 4);
    _image_buffer_auto_fill_pot_square_size(&result);
    return result;
}

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
        // NOTE: memory allocation scheme is not complicated enough to select different arenas for allocation
        // so these are heap allocated instead of being tied to the main engine's arena allocators
        // I could fix this, but since assets are "big" memory... This is not a big deal to me.
        // struct file_buffer buffer = VFS_read_entire_file(memory_arena_allocator(&game_arena), filepath);
        struct file_buffer buffer = VFS_read_entire_file(heap_allocator(), filepath);
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
    zero_memory(&result, sizeof(result));
    result.pixels = image_buffer;
    result.width  = width;
    result.height = height;
    result.pot_square_size = 0;
    _image_buffer_auto_fill_pot_square_size(&result);
    return result;
}

void image_buffer_write_to_disk(struct image_buffer* image, string as) {
    char filename[256] = {};
#if 0
    snprintf(filename, 256, "%s.bmp", as.data);
    stbi_write_bmp(filename, image->width, image->height, 4, image->pixels);
#else
    // snprintf(filename, 256, "%s.jpg", as.data);
    // stbi_write_jpg(filename, image->width, image->height, 4, image->pixels, 0);

    snprintf(filename, 256, "%s.png", as.data);
    stbi_write_png(filename, image->width, image->height, 4, image->pixels, image->width*4);
#endif
    _debugprintf("wrote \"%.*s\"", as.length, as.data);
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
    zero_memory(&result, sizeof(result));
    result.type        = FONT_CACHE_ATLAS_FIXED_ASCII;
    result.tile_width  = tile_width;
    result.tile_height = tile_height;
    result.atlas_rows  = atlas_rows;
    result.atlas_cols  = atlas_columns;

    struct image_buffer atlas_image = image_buffer_load_from_file(filepath);
   // image_buffer_pad_to_POT(&atlas_image);

    result.width  = atlas_image.width;
    result.height = atlas_image.height;
    result.pixels = atlas_image.pixels;
   // result.pot_square_size = atlas_image.pot_square_size;

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

struct graphics_assets graphics_assets_create(Memory_Arena* arena, u32 font_limit, u32 image_limit, u32 sprite_limit) {
    struct graphics_assets assets = {
        .font_capacity  = font_limit,
        .image_capacity = image_limit,
        .sprite_capacity = sprite_limit,
    };

    assets.arena              = arena;
    assets.images             = (image_buffer*)arena->push_unaligned(sizeof(*assets.images) * image_limit);
    assets.image_asset_status = (u8*)arena->push_unaligned(sizeof(*assets.image_asset_status) * image_limit);
    assets.image_file_strings = (string*)arena->push_unaligned(sizeof(*assets.image_file_strings) *image_limit);
    assets.fonts              = (font_cache*)arena->push_unaligned(sizeof(*assets.fonts)  * font_limit);
    assets.sprites            = (Sprite*)arena->push_unaligned(sizeof(*assets.sprites) * sprite_limit);

    return assets;
}

void graphics_assets_finish(struct graphics_assets* assets) {
    _debugprintf("freeing all assets");
    DEBUG_graphics_assets_dump_all_images(assets, string_literal("DEBUG_dump_graphics_assets/"));

    for (unsigned image_index = 0; image_index < assets->image_count; ++image_index) {
        if (assets->image_asset_status[image_index] == ASSET_STATUS_UNLOADED)
            continue;
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
        string filepath       = assets->image_file_strings[index];
        u8*     status_field  = &assets->image_asset_status[index];

        if (string_equal(path, filepath)) {
            auto new_image_id = image_id{.index = index+1};
            if (*status_field == ASSET_STATUS_UNLOADED) {
                struct image_buffer* new_image           = &assets->images[index];
                *new_image                               = image_buffer_load_from_file(filepath);
                *status_field                            = ASSET_STATUS_LOADED;

                assert(assets->graphics_driver && "Hmm, how do you have no graphics driver?");
                _debugprintf("Reupload existing texture");
                assets->graphics_driver->upload_texture(assets, new_image_id);
            }
            return new_image_id;
        }
    }

    image_id new_id = image_id { .index = (s32)assets->image_count + 1 };
    _debugprintf("img loaded: %.*s", path.length, path.data);

    struct image_buffer* new_image           = &assets->images[assets->image_count];
    u8*                  status_field        = &assets->image_asset_status[assets->image_count];
    string*              new_filepath_string = &assets->image_file_strings[assets->image_count++];
    *new_filepath_string                     = memory_arena_push_string(assets->arena, path);
    *new_image                               = image_buffer_load_from_file(*new_filepath_string);
    *status_field                            = ASSET_STATUS_LOADED;

    assert(assets->graphics_driver && "Hmm, how do you have no graphics driver?");
    _debugprintf("Upload new texture");
    assets->graphics_driver->upload_texture(assets, new_id);

    return new_id;
}

void graphics_assets_unload_image(struct graphics_assets* assets, image_id img) {
    if (img.index == 0) {
        return;
    }

    s32 index = img.index-1;
    auto img_buffer = graphics_assets_get_image_by_id(assets, img);

    if (assets->image_asset_status[index] == ASSET_STATUS_PERMENANTLY_LOADED) {
        _debugprintf("Cannot unload image id %d, because it is a PERMENANT asset", img.index);
        return;
    }

    if (assets->image_asset_status[index] != ASSET_STATUS_UNLOADED) {
        assets->graphics_driver->unload_texture(assets, img);
        _debugprintf("Unloading image id %d\n", img.index);
        image_buffer_free(img_buffer);
        assets->image_asset_status[index] = ASSET_STATUS_UNLOADED;

        assert(assets->graphics_driver && "Hmm, how do you have no graphics driver?");
    }
}

font_id graphics_assets_load_bitmap_font(struct graphics_assets* assets, string path, s32 tile_width, s32 tile_height, s32 atlas_rows, s32 atlas_columns) {
    font_id new_id = font_id { .index = (s32)assets->font_count + 1 };

    struct font_cache* new_font   = &assets->fonts[assets->font_count++];
    *new_font                     = font_cache_load_bitmap_font(path, tile_width, tile_height, atlas_rows, atlas_columns);

    assert(assets->graphics_driver && "Hmm, how do you have no graphics driver?");
    assets->graphics_driver->upload_font(assets, new_id);

    return new_id;
}

struct font_cache* graphics_assets_get_font_by_id(struct graphics_assets* assets, font_id font) {
    assertion(font.index > 0 && font.index <= assets->font_count);
    return &assets->fonts[font.index-1];
}

struct image_buffer* graphics_assets_get_image_by_id(struct graphics_assets* assets, image_id image) {
    assertion(image.index > 0 && image.index <= assets->image_count);
    s32 index = image.index - 1;

    string filepath     = assets->image_file_strings[index];
    u8*    status_field = &assets->image_asset_status[index];

    if (*status_field == ASSET_STATUS_UNLOADED) {
        graphics_assets_load_image(assets, filepath);
    }

    return &assets->images[index];
}

void graphics_assets_update_graphics_driver(struct graphics_assets* assets, Graphics_Driver* driver) {
    if (assets->graphics_driver != driver && assets->graphics_driver) {
        _debugprintf("Updating from previous graphics driver");
        _debugprintf("NOTE: reupload all resources to new driver.");

        for (u32 image_index = 0; image_index < assets->image_count; ++image_index) {
            auto& image = assets->images[image_index];
            image._driver_userdata = nullptr;
        }

        for (u32 font_index = 0; font_index < assets->font_count; ++font_index) {
            auto& font = assets->fonts[font_index];
            font._driver_userdata = nullptr;
        }
    } else {
        _debugprintf("First graphics driver initialization. Nothing to do");
    }
    assets->graphics_driver = driver;
}

image_id graphics_assets_get_image_by_filepath(struct graphics_assets* assets, string filepath) {
    assertion(filepath.data && filepath.length && "Bad string?");
    return graphics_assets_load_image(assets, filepath);
}

sprite_id graphics_assets_alloc_sprite(struct graphics_assets* assets, u32 frames) {
    sprite_id result    = {.index = (s32)assets->sprite_count+1};
    auto      sprite    = &assets->sprites[assets->sprite_count++];
    sprite->frame_count = frames;
    sprite->frames      = (Sprite_Frame*)assets->arena->push_unaligned(sizeof(*sprite->frames) * frames);
    return result;
}

Sprite* graphics_get_sprite_by_id(struct graphics_assets* assets, sprite_id id) {
    // Special case added, because I kind of care when it *is* null,
    // or rather if I changed this, I'm not sure what else would need to change.
    if (id.index == 0) {
        return nullptr;
    }

    assertion(id.index > 0 && id.index <= assets->sprite_count);
    return &assets->sprites[id.index-1];
}

s32 sprite_get_unique_image_count(Sprite* sprite) {
    assertion(sprite);
    s32 count = 0;
    s32* image_ids_encountered =
        (s32*)Global_Engine()->scratch_arena.push_unaligned(sizeof(*image_ids_encountered) * sprite->frame_count);

    for (s32 index = 0; index < sprite->frame_count; ++index) {
        bool unique = true;
        auto frame = sprite_get_frame(sprite, index);

        for (s32 encountered_index = 0; encountered_index < count; ++encountered_index) {
            if (image_ids_encountered[encountered_index] == frame->img.index) {
                unique = false;
                break;
            }
        }

        if (unique) {
            image_ids_encountered[count++] = frame->img.index;
        }
    }

    return count;
}

Sprite_Frame* sprite_get_frame(Sprite* sprite, s32 index) {
    //_debugprintf("sprite get frame: (%p) %d/%d", sprite, index, sprite->frame_count);
    assertion(sprite);
    assertion(index >= 0 && index < sprite->frame_count);
    return &sprite->frames[index];
}

s32 sprite_copy_all_images_into_image_array(Sprite* sprite, image_id* destination_array, s32 length) {
    assertion(sprite);

    s32 frame_count = sprite->frame_count;
    s32 written     = 0;

    for (s32 frame_index = 0; frame_index < frame_count; ++frame_index) {
        auto& frame = sprite->frames[frame_index];
        auto img_id = frame.img;

        assertion(img_id.index && "Why is this image ID null?");
        if (written >= length)
            break;

#if 1
        // Skip including the same image multiple times.
        bool skip_iter = false;

        // No this isn't super optimal but whatever.
        for (s32 destination_index = 0; destination_index < written; ++destination_index) {
            if (destination_array[destination_index].index == img_id.index) {
                skip_iter = true;
                break;
            }
        }

        if (skip_iter) {
            continue;
        }
#endif

        destination_array[written++] = img_id;
    }

    return written;
}

Sprite_Instance sprite_instance(sprite_id id) {
    Sprite_Instance result = {};
    result.id = id;
    result.frame = 0;
    result.frame_timer = 0.0f;
    result.modulation = color32f32(1, 1, 1, 1);
    result.offset = V2(0,0);
    result.scale = V2(1, 1);
    return result;
}

void Sprite_Instance::reset(void) {
    loop_completions = 0;
    frame_timer      = 0;
    frame            = 0;
}

void Sprite_Instance::animate(struct graphics_assets* graphics_assets, f32 dt, f32 anim_t, s32 start, s32 end) {
    assert(id.index != 0 && "You should have a valid sprite when calling this.");

    auto sprite_object = graphics_get_sprite_by_id(graphics_assets, id);

    if (start == -1) start = 0;
    if (end == -1) end = sprite_object->frame_count;
    if (frame < start || frame >= end) frame = start;

    auto sprite_frame  = sprite_get_frame(sprite_object, clamp<s32>(frame, start, end-1));

    if (anim_t == -1.0f) anim_t = sprite_frame->frame_length;
    frame_timer += dt;

    if (frame_timer > anim_t) {
        frame += 1;

        if (frame < start || frame >= end) {
            frame = start;  
            loop_completions += 1;
        } 
        frame_timer = 0;
    }
}

const rectangle_f32 Texture_Atlas::get_subrect(const image_id subimage) const {
    for (s32 index = 0; index < subimage_count; ++index) {
        if (subimages[index].original_asset.index == subimage.index) {
            return subimages[index].subrectangle;
        }
    } 

    return RECTANGLE_F32_NULL;
}

const rectangle_f32 Texture_Atlas::get_subrect(const image_id subimage, rectangle_f32 existing_subrectangle) const {
    auto image_subrectangle = get_subrect(subimage);
    rectangle_f32 result = image_subrectangle;

    // NOTE: cannot provide wrapping behavior because it's a texture atlas.
    // NOTE: the naive texture atlas packing algorithm doesn't do any rescaling, but
    // it might be a neat thing to consider in future iterations of it.

    result.x += existing_subrectangle.x;
    result.y += existing_subrectangle.y;

    if (existing_subrectangle.w != 0) result.w = existing_subrectangle.w;
    if (existing_subrectangle.h != 0) result.h = existing_subrectangle.h;

    return result;
}

Texture_Atlas graphics_assets_construct_texture_atlas_image(struct graphics_assets* assets, image_id* images, size_t image_count) {
    _debugprintf("Constructing texture atlas from %d images", image_count);

    // I will try to tightly fit these, but otherwise just assemble the atlas in a "not so smart" way
    Texture_Atlas result;

    result.atlas_image_id;
    result.subimage_count = image_count;
    result.subimages      = (Texture_Atlas_Sub_Image*)assets->arena->push_unaligned(sizeof(*result.subimages) * image_count);

    u32 image_pot_size = (1 << 7); // 128 default size test...

    u32 write_x = 0;
    u32 write_y = 0;

    {
        bool complete = false;
        while (!complete) {
            write_x = 0;
            write_y = 0;
            bool request_resize = false;
            s32  height_of_budger = 0;

            bool new_row = false;
            for (u32 index = 0; index < image_count; ++index) {
                struct image_buffer* img             = graphics_assets_get_image_by_id(assets, images[index]);
                auto&                subimage_object = result.subimages[index];

                subimage_object.subrectangle   = rectangle_f32(write_x, write_y, img->width, img->height);
                _debugprintf("Fitting image index %d", index);

                if (request_resize) {
                    break;
                }

                if (new_row) {
                    _debugprintf("Make new row");

                    // Slow? Probably. Correct? More probably.
                    // This only really needs to happen once so I'm not too concerned about this...
                    for (u32 index1 = 0; index1 < index; ++index1) {
                        if (index == index1) continue;
                        auto& subimage_object1 = result.subimages[index1];

                        if (rectangle_f32_intersect(subimage_object1.subrectangle, subimage_object.subrectangle)) {
                            write_x += subimage_object1.subrectangle.w;
                            subimage_object.subrectangle.x = write_x;
                            // work against any collisions...
                        }
                    }
                    new_row = false;
                }

                _debugprintf("rectangle(%d) (%d, %d, %d, %d)", index, write_x, write_y, img->width, img->height);

                if (((s32)image_pot_size-(s32)write_x) <= (s32)img->width) {
                    write_y += img->height;
                    write_x = 0;
                    new_row = true;
                    index-=1; // try to replace sprite since it obviously doesn't fit here.
                } else {
                    write_x += img->width;
                    if (write_x >= image_pot_size) {
                        write_y += img->height;
                        write_x = 0;
                        new_row = true;
                    }
                }

                if (subimage_object.subrectangle.y >= image_pot_size ||
                    subimage_object.subrectangle.y + subimage_object.subrectangle.h > image_pot_size) {
                    request_resize = true;
                    height_of_budger = subimage_object.subrectangle.h;
                }
            }

            if (request_resize) {
                u32 last_image_pot_size = image_pot_size;
                image_pot_size = nearest_pot32(write_y+height_of_budger);
                _debugprintf("Requesting resisze from %d to %d (write_cursor=%d, height_of_budger=%d)", last_image_pot_size, image_pot_size, write_y, height_of_budger);
            } else {
                complete = true;
                _debugprintf("Texture atlas simplest fit determined to be %dx%d", image_pot_size, image_pot_size);
            }
        }
    }

    _debugprintf("Assembling final atlas");

    {
#if 1
        image_id             new_id    = image_id { .index = (s32)assets->image_count + 1 };
        struct image_buffer* new_image = &assets->images[assets->image_count];
        u8*                  status_field        = &assets->image_asset_status[assets->image_count];
        string*              new_filepath_string = &assets->image_file_strings[assets->image_count++];

        *new_filepath_string                     = memory_arena_push_string(assets->arena, string_from_cstring(format_temp("___atlasid%d", assets->image_count-1)));
        *new_image                               = image_buffer_create_blank(image_pot_size, image_pot_size);
        *status_field                            = ASSET_STATUS_PERMENANTLY_LOADED;

        for (u32 index = 0; index < image_count; ++index) {
            struct image_buffer* img             = graphics_assets_get_image_by_id(assets, images[index]);
            auto&               subimage_object = result.subimages[index];

            subimage_object.original_asset = images[index];

            {
                _debugprintf("writing: %d, %d, %d, %d", (s32)subimage_object.subrectangle.x, (s32)subimage_object.subrectangle.y,(s32)subimage_object.subrectangle.w,(s32)subimage_object.subrectangle.h);
                for (u32 y = 0; y < img->height; ++y) {
                    u32 final_write_y = subimage_object.subrectangle.y + y;
                    assert(final_write_y < image_pot_size);

                    for (u32 x = 0; x < img->width; ++x) {
                        u32 final_write_x = subimage_object.subrectangle.x + x;
                        assert(final_write_x < image_pot_size);

                        new_image->pixels[final_write_y * image_pot_size*4 + (final_write_x*4) + 0] = img->pixels[y * img->width*4 + (x * 4) + 0];
                        new_image->pixels[final_write_y * image_pot_size*4 + (final_write_x*4) + 1] = img->pixels[y * img->width*4 + (x * 4) + 1];
                        new_image->pixels[final_write_y * image_pot_size*4 + (final_write_x*4) + 2] = img->pixels[y * img->width*4 + (x * 4) + 2];
                        new_image->pixels[final_write_y * image_pot_size*4 + (final_write_x*4) + 3] = img->pixels[y * img->width*4 + (x * 4) + 3];
                    }
                }
            }
        }

        result.atlas_image_id = new_id;
        assets->graphics_driver->upload_texture(assets, new_id);
#endif
    }
    _debugprintf("Atlas writing completed");
    return result;
}

void graphics_assets_texture_atlas_unload_original_subimages(struct graphics_assets* assets, const Texture_Atlas& texture_atlas) {
    _debugprintf("Unloading all original assets to reduce redundancy");
    for (s32 index = 0; index < texture_atlas.subimage_count; ++index) {
        auto& subimage = texture_atlas.subimages[index];
        graphics_assets_unload_image(assets, subimage.original_asset);
    }
}

// I need this to verify that the texture atlas is actually doing what it's supposed to...
void DEBUG_graphics_assets_dump_all_images(struct graphics_assets* assets, string directory) {
#ifdef RELEASE
#else
    _debugprintf("dumping all loaded image assets.");
    OS_create_directory(directory);

    for (unsigned index = 0; index < assets->image_count; ++index) {
        auto& image = assets->images[index];
        if (assets->image_asset_status[index] == ASSET_STATUS_UNLOADED) {
            continue;
        }
        auto file_string = assets->image_file_strings[index];

        char finalpath[260];
        {
            int  write = 0;
            int  i = 0;

            while (i < file_string.length) {
                switch (file_string.data[i]) {
                    case '\\':
                    case '/':
                        finalpath[write++] = '_';
                        i++;
                        break;
                    default:
                        finalpath[write++] = file_string.data[i++];
                        break;
                }
            }
            finalpath[write++] = '\0';
        }

        image_buffer_write_to_disk(
            &image,
            string_from_cstring(
                format_temp("%.*s/%s",
                            directory.length, directory.data,
                            finalpath
                )
            )
        );
    }
#endif
}
