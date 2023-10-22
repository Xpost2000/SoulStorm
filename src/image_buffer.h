#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

// NOTE: only structure information.

#include "common.h"

struct image_buffer {
    u32 width;
    u32 height;
    union {
        u8* pixels;
        u32* pixels_u32;
    };
};

#endif
