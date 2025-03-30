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

#include "graphics_assets.h"

struct SDL_Window;

// NOTE: due to the interesting behavior requirements of the game,
//       hardware drivers are likely going to be rendering at a lower
//       resolution then they actually have to, so I might change something
//       to circumvent this...
//       (Or use a specific projection matrix to make sure it uses the same coordinate system, as
//        the software renderer which is the "reference")
struct Resolution_Mode {
    // I don't really care about different display modes outside of resolution
    // to be honest, I'm not sure if 16bit or 24bit color depth matters much in 2023 anymore...
    int width;
    int height;
};

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

    /*
     * Will keep track of a device specific item,
     * and write into a context ptr.
     */
    virtual void upload_texture(struct graphics_assets* assets, image_id image) = 0;
    virtual void upload_font(struct graphics_assets* assets, font_id font)      = 0;

    // NOTE: while you should implement this,
    // since a lot of memory in this game is sort of expected to be "permenant"
    // this is generally reasonable for now.
    virtual void unload_texture(struct graphics_assets* assets, image_id image);
    virtual void unload_font(struct graphics_assets* assets, font_id image);

    // NOTE: will not change the back buffer. You have to do that manually.
    void change_resolution(s32 new_resolution_x, s32 new_resolution_y);

    // static methods for "globally" shared resources or associated functions.
    static void                   populate_display_mode_list(SDL_Window* game_window);
    static Slice<Resolution_Mode> get_display_modes(void);
    static s32                    find_index_of_resolution(s32 w, s32 h);
    //                     if it is an enumerable display mode.
    virtual void screenshot(char* where) = 0;

    void set_vsync(bool v);
    bool get_vsync(void) const;

    virtual const char* get_name(void);

    bool is_initialized() { return initialized; }
protected:
    SDL_Window* game_window = nullptr;
    bool        initialized = false;
    bool        vsync_enabled = false;
private:
    static bool already_have_resolution(s32 w, s32 h);
    /*
     * I would like to know how I would even get close to half this number in the next decade.
     * I just don't want to dynamically allocate anything really.
     */
    static Resolution_Mode display_modes[128];
    static int display_mode_count;
};

#endif
