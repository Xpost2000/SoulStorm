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

    // NOTE: This field is modified by the graphics_driver implementations
    //       to store internal pointers to the actual backing graphics driver
    //       resource.
    //
    //
    //       This engine was made with a software renderer first and this was technically
    //       the only way this stuff was meant to be used.
    void* _driver_userdata;
};

#endif
