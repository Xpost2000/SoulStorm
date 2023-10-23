#ifndef GRAPHICS_DRIVER_H
#define GRAPHICS_DRIVER_H
/*
 * This is an interface for other renderer plugins
 *
 * the simplest graphics driver interface that could
 * work for a decent amount of 2D games would just be
 *
 * a render command consumer.
 */

#include "graphics_common.h"
#include "render_commands.h"

struct SDL_Window;
class Graphics_Driver {
public:
    // TODO: for hardware drivers I would need to register
    // resources.

    // The graphics_assets stuff is meant to be CPU-side resources
    // only, and was completely fine back when I was only doing software
    // rendering.

    // I'll probably try to keep a list of pointers to the graphics resources
    // that I can hand out Graphics_Driver devices for.

    // NOTE: Renderers should not be swapped at runtime. Frankly I plan to
    //       just distribute separate executables...
    //     
    //       I'll see though...
    //       Changing devices at runtime might require reorganization of some
    //       more game code.

    // Window is needed to create a device context
    virtual void initialize(SDL_Window* window, int width, int height)     = 0;
    virtual void initialize_backbuffer(V2 resolution)                      = 0;
    virtual void swap_and_present()                                        = 0;
    virtual void finish()                                                  = 0;
    virtual void clear_color_buffer(color32u8 color)                       = 0;
    virtual void consume_render_commands(struct render_commands* commands) = 0;

    // To satisfy the demands of the game's expected conventions
    // this is the resolution of the back buffer.
    // the input system will automatically scale coordinates to client mode.
    virtual V2   resolution()                                              = 0;
};

#endif
