#ifndef FONT_CACHE_H
#define FONT_CACHE_H

#include "common.h"
#include "image_buffer.h"

// NOTE: only structure information.
enum font_cache_type {
    FONT_CACHE_ATLAS_FIXED_ASCII,
    FONT_CACHE_ATLAS_VARIABLE, // NOTE: unused
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

#endif
