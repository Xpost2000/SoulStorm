#ifndef IMAGE_BUFFER_H
#define IMAGE_BUFFER_H

// NOTE: only structure information.

#include "common.h"

struct image_buffer {
    u32 width;
    u32 height;
    
    // rescale when needed for
    // targets that only work POT
    // like OpenGL ES or WebGL...
    u32 pot_square_size; // This is a dumb POT scale. No rectangular textures here though. Bye bye memory!

    union {
        u8* pixels;
        u32* pixels_u32;
    };

    // TODO: add graphics_driver specific information.
    void* _driver_userdata;
};

#endif
