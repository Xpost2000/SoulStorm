#ifndef GRAPHICS_DIRECT3D11_DRIVER_H
#define GRAPHICS_DIRECT3D11_DRIVER_H
#include "graphics_driver.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#include "fixed_array.h"
/*
  NOTE: obviously only works on windows lol
*/
#define MAX_D3D11_TEXTURES (512)
#define MAX_D3D11_BATCHED_QUADS (4096)
#define MAX_D3D11_VERTICES_FOR_QUAD_BUFFER (MAX_D3D11_BATCHED_QUADS * 6)
struct D3D11_Vertex_Format {
    V2         position;
    V2         texcoord;
    color32f32 color;
};


struct D3D11_Image {
    ID3D11Texture2D* texture2d = nullptr;
    ID3D11ShaderResourceView* shader_resource_view = nullptr;

    void Release(void);
};

_declspec(align(16))
struct D3D11_Matrix_Constant_Buffer {
    float projection_matrix[16];
    float view_matrix[16];
    float global_elapsed_time;
};

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
    void unload_texture(struct graphics_assets* assets, image_id image);
    void screenshot(char* where);
    const char* get_name(void);
private:
    void setup_viewport(void);
    void initialize_default_rendering_resources(void);

    void set_blend_mode(u8 blend_mode);
    void set_texture_id(D3D11_Image* image);
    void push_render_quad_vertices(rectangle_f32 destination, rectangle_f32 source, color32f32 color, struct image_buffer* image, s32 angle = 0, s32 angle_y = 0, u32 flags = NO_FLAGS, V2 rotation_origin = V2(0,0));

    // NOTE: could abstract into one file, but
    // these are the only two hardware paths. Might as well keep them.
    void render_command_draw_quad(const render_command& rc);
    void render_command_draw_image(const render_command& rc);
    void render_command_draw_line(const render_command& rc);
    void render_command_draw_text(const render_command& rc);
    void render_command_draw_set_scissor(const render_command& rc);
    void render_command_draw_clear_scissor(const render_command& rc);
    void flush_and_render_quads(void);

    bool find_first_free_image(D3D11_Image** result);
    void upload_image_buffer_to_gpu(struct image_buffer* image);

    bool initialized_resources = false;

    V2 real_resolution;
    V2 virtual_resolution;

    D3D11_Image* current_image;
    u8           current_blend_mode = -1;
    Fixed_Array<D3D11_Vertex_Format> quad_vertices;

    ID3D11Device*        device  = nullptr;
    ID3D11DeviceContext* context = nullptr;

    ID3D11Buffer*       matrix_constant_buffer = nullptr;
    ID3D11Buffer*       vertex_buffer          = nullptr;
    ID3D11VertexShader* vertex_shader          = nullptr;
    ID3D11PixelShader*  pixel_shader           = nullptr;

    IDXGISwapChain*         swapchain = nullptr;
    ID3D11RenderTargetView* rendertarget = nullptr;
    ID3D11Texture2D*        swapchain_framebuffer_texture = nullptr;

    ID3D11InputLayout* vertex_layout = nullptr;
    ID3D11SamplerState* nearest_neighbor_sampler_state = nullptr;

    D3D11_Image images[MAX_D3D11_TEXTURES];
    D3D11_Image white_pixel;

    // blend modes
    ID3D11BlendState* blending_states[BLEND_MODE_COUNT];
};

#endif
