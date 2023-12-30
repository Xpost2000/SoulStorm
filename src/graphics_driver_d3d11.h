#ifndef GRAPHICS_DIRECT3D11_DRIVER_H
#define GRAPHICS_DIRECT3D11_DRIVER_H
#include "graphics_driver.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
/*
  NOTE: obviously only works on windows lol
*/
#define MAX_D3D11_TEXTURES (512)
class Direct3D11_Graphics_Driver : public Graphics_Driver {
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
    const char* get_name(void);
private:
    ID3D11Device* device;
    ID3D11DeviceContext* context;

    int texture_count = 0;
    ID3D11Texture2D* textures[MAX_D3D11_TEXTURES];
    ID3D11ShaderResourceView* texture_shader_resource_view[MAX_D3D11_TEXTURES];
};

#endif
