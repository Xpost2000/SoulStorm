#ifndef GRAPHICS_NULL_DRIVER_H
#define GRAPHICS_NULL_DRIVER_H
#include "graphics_driver.h"

/*
  NOTE: this driver will not do anything,

  However, it is implemented in such a way that the game logic
  should just "work".
*/
class Null_Graphics_Driver : public Graphics_Driver {
public:
    void initialize(SDL_Window* window, int width, int height);
    void initialize_backbuffer(V2 resolution);
    void swap_and_present();
    void finish();
    void clear_color_buffer(color32u8 color);
    void consume_render_commands(struct render_commands* commands);
    V2   resolution();
    void upload_texture(struct graphics_assets* assets, image_id image);
    void upload_font(struct graphics_assets* assets, font_id font);
    void screenshot(char* where);
};

#endif
