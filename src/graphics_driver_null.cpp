#include "graphics_driver_null.h"

void Null_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    _debugprintf("Null Initialize");
    game_window = window;
}

void Null_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("Null Initialize Backbuffer");
}

void Null_Graphics_Driver::swap_and_present() {
    
}

void Null_Graphics_Driver::finish() {
    _debugprintf("Null Driver Finish");
}

void Null_Graphics_Driver::clear_color_buffer(color32u8 color) {
    
}

void Null_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    
}

V2 Null_Graphics_Driver::resolution() {
    return V2(640, 480);
}

void Null_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    
    _debugprintf("Null driver upload texture is nop");
}

void Null_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    _debugprintf("Null driver upload font is nop");
}

void Null_Graphics_Driver::screenshot(char* where) {
    _debugprintf("Null driver screenshot is nop");
}


const char* Null_Graphics_Driver::get_name(void) {
    return "(NULL Graphics Driver)";
}
