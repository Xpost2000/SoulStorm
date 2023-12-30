#include "graphics_driver_d3d11.h"

void Direct3D11_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    _debugprintf("Direct3D11 Initialize");
    game_window = window;
}

void Direct3D11_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("Direct3D11 Initialize Backbuffer");
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::swap_and_present() {
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::finish() {
    _debugprintf("Direct3D11 Driver Finish");
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::clear_color_buffer(color32u8 color) {
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    unimplemented("Not done");
}

V2 Direct3D11_Graphics_Driver::resolution() {
    return V2(640, 480);
}

void Direct3D11_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("Direct3D11 driver upload texture is nop");
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    _debugprintf("Direct3D11 driver upload font is nop");
    unimplemented("Not done");
}

void Direct3D11_Graphics_Driver::screenshot(char* where) {
    _debugprintf("Direct3D11 driver screenshot is nop");
    unimplemented("Not done");
}


const char* Direct3D11_Graphics_Driver::get_name(void) {
    unimplemented("Not done");
    return "(Drect3D11)";
}
