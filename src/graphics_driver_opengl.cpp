#include "graphics_driver_opengl.h"

void OpenGL_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    _debugprintf("OpenGL Initialize");
    
    game_window = window;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    context = SDL_GL_CreateContext(game_window);
    SDL_GL_MakeCurrent(game_window, context);
    assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress) && "OpenGL functions could not be loaded. That's bad.");
}

void OpenGL_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("OpenGL Initialize Backbuffer");
    unimplemented("Not done");
}

void OpenGL_Graphics_Driver::swap_and_present() {
    SDL_GL_SetSwapInterval(-1);
    SDL_GL_SwapWindow(game_window);
}

void OpenGL_Graphics_Driver::finish() {
    _debugprintf("OpenGL Driver Finish");
    unimplemented("Not done");
}

void OpenGL_Graphics_Driver::clear_color_buffer(color32u8 color) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void OpenGL_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    unimplemented("Not done");
}

V2 OpenGL_Graphics_Driver::resolution() {
    return V2(640, 480);
}

void OpenGL_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("OpenGL driver upload texture is nop");
    unimplemented("Not done");
}

void OpenGL_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    _debugprintf("OpenGL driver upload font is nop");
    unimplemented("Not done");
}

void OpenGL_Graphics_Driver::screenshot(char* where) {
    _debugprintf("OpenGL driver screenshot is nop");
    unimplemented("Not done");
}


const char* OpenGL_Graphics_Driver::get_name(void) {
    unimplemented("Not done");
    return "(OpenGL 3.3)";
}
