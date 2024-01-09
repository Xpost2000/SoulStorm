// TODO:
// I do not know how to use blending yet.
#include "graphics_driver_d3d11.h"
#include "engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

local const char* pixel_shader_source = R"shader(
    struct VertexShader_Result 
    {
        float4 position_clip : SV_POSITION;
    };
    
    float4 main(VertexShader_Result input) : SV_Target {
        return float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
)shader";

local const char* vertex_shader_source = R"shader(
    struct VertexShader_Params
    {
        float2 position : POS;
        float2 texcoord : TEX;
        float4 color    : COL;
    };

    struct VertexShader_Result
    {
        float4 position_clip : SV_POSITION;
    };

    VertexShader_Result main(VertexShader_Params input) {
        VertexShader_Result result;
        result.position_clip = float4(input.position, 1.0f, 1.0f);
        return result;
    }
)shader";

// D3D11 Helpers Begin

D3D11_Image d3d11_image_from_image_buffer(ID3D11Device* device, struct image_buffer* image_buffer) {
    D3D11_Image result;
    result.texture2d            = nullptr;
    result.shader_resource_view = nullptr;

    {
        D3D11_TEXTURE2D_DESC texture_description;
        zero_memory(&texture_description, sizeof(texture_description));

        auto image_width              = (image_buffer->pot_square_size)  ? image_buffer->pot_square_size : image_buffer->width;
        auto image_height             = (image_buffer->pot_square_size) ? image_buffer->pot_square_size : image_buffer->height;
        texture_description.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_description.Width     = image_width;
        texture_description.Height    = image_height;
        texture_description.ArraySize = 1;
        texture_description.MipLevels = 1;
        texture_description.Usage     = D3D11_USAGE_IMMUTABLE;
        texture_description.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texture_description.SampleDesc.Count = 1;

        D3D11_SUBRESOURCE_DATA texture_data;
        zero_memory(&texture_data, sizeof(texture_data));
        texture_data.pSysMem = image_buffer->pixels;
        texture_data.SysMemPitch = sizeof(uint32_t) * image_width;

        assertion(
            SUCCEEDED(
                device->CreateTexture2D(
                    &texture_description,
                    &texture_data,
                    &result.texture2d
                )
            ) &&
            "Texture2D creation failure?"
        );
    }

    {
        D3D11_SHADER_RESOURCE_VIEW_DESC texture_shader_resource_view_description;
        zero_memory(&texture_shader_resource_view_description, sizeof(texture_shader_resource_view_description));
        texture_shader_resource_view_description.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture_shader_resource_view_description.Texture2D.MipLevels = 1;
        texture_shader_resource_view_description.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

        assertion(
            SUCCEEDED(
                device->CreateShaderResourceView(
                    result.texture2d,
                    &texture_shader_resource_view_description,
                    &result.shader_resource_view
                )
            ) &&
            "Texture2D SRV creation failure?"
        );
    }

    return result;
}

ID3DBlob* d3d_compile_shader_blob(const char* shader_source, const char* shader_name, const char* target) {
    ID3DBlob* result;
    ID3DBlob* error_message;
    HRESULT hr = D3DCompile(
        shader_source,
        cstring_length(shader_source),
        shader_name,
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "main",
        target,
        D3DCOMPILE_ENABLE_STRICTNESS, 0,
        &result,
        &error_message
    );

    if (error_message) {
        _debugprintf("D3DCompile: %.*s", error_message->GetBufferSize(), error_message->GetBufferPointer());
    }

    assertion(SUCCEEDED(hr) && "D3DCompile failed to compile this blob.");

    return result;
}
// D3D11 Helpers End

// NOTE: need to recreate swap chain when initializing. LOL

void Direct3D11_Graphics_Driver::initialize(SDL_Window* window, int width, int height) {
    _debugprintf("Direct3D11 Initialize");
    if (game_window == nullptr) {
        game_window = window;
        quad_vertices = Fixed_Array<D3D11_Vertex_Format>(&Global_Engine()->main_arena, MAX_D3D11_VERTICES_FOR_QUAD_BUFFER);

        zero_array(images);
    }
    initialize_backbuffer(V2(width, height));
}

void Direct3D11_Graphics_Driver::initialize_backbuffer(V2 resolution) {
    _debugprintf("Direct3D11 Initialize Backbuffer");

    /*
      TODO:

      For now, I'll do it the dumb way, and just use CreateDeviceAndSwapChain

      I will assume I don't resize as I need to just be able to "render" anything
      at all...
    */
    real_resolution = V2(Global_Engine()->real_screen_width, Global_Engine()->real_screen_height);
    virtual_resolution = resolution;

    DXGI_SWAP_CHAIN_DESC swapchain_description;
    zero_memory(&swapchain_description, sizeof(swapchain_description));
    swapchain_description.BufferDesc.Width  = real_resolution.x;
    swapchain_description.BufferDesc.Height = real_resolution.y;
    swapchain_description.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

    // no vsync
    swapchain_description.BufferDesc.RefreshRate.Numerator   = 0;
    swapchain_description.BufferDesc.RefreshRate.Denominator = 0;

    // no aa
    swapchain_description.SampleDesc.Count   = 1;
    swapchain_description.SampleDesc.Quality = 0;

    {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);
        SDL_GetWindowWMInfo(game_window, &wmInfo);
        HWND hwnd = wmInfo.info.win.window;
        swapchain_description.OutputWindow = hwnd;
    }
    swapchain_description.Windowed = true; // TODO: check windowed status from engine later.

    swapchain_description.BufferCount = 2;
    swapchain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    D3D_FEATURE_LEVEL feature_level;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifndef RELEASE
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        0, 0,
        D3D11_SDK_VERSION,
        &swapchain_description,
        &swapchain,
        &device,
        &feature_level,
        &context
    );

    // swapchain_framebuffer_texture holds backbuffer
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&swapchain_framebuffer_texture);
    device->CreateRenderTargetView(
        swapchain_framebuffer_texture,
        nullptr,
        &rendertarget
    );

    // Create D3D11 Resources here
    {
        // Create Default Shaders
        auto ps_blob = d3d_compile_shader_blob(pixel_shader_source, "PixelShaderDefault", "ps_4_0");
        auto vs_blob = d3d_compile_shader_blob(vertex_shader_source, "VertexShaderDefault", "vs_4_0");
        {
            assertion(SUCCEEDED(device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), 0, &pixel_shader)));
            assertion(SUCCEEDED(device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), 0, &vertex_shader)));
        }
        // Create Vertex Layout
        {
            D3D11_INPUT_ELEMENT_DESC vertex_layout_description[] = {
                {"POS", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"TEX", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            assertion(
                SUCCEEDED(
                    device->CreateInputLayout(
                        vertex_layout_description,
                        3,
                        vs_blob->GetBufferPointer(),
                        vs_blob->GetBufferSize(),
                        &vertex_layout
                    )
                ) &&
                "Vertex layout creation failure."
            );
        }

        // Allocate vertex buffer
        {
            D3D11_BUFFER_DESC vertex_buffer_description = {};
            zero_memory(&vertex_buffer_description, sizeof(vertex_buffer_description));
            vertex_buffer_description.ByteWidth      = quad_vertices.capacity * sizeof(D3D11_Vertex_Format);
            vertex_buffer_description.Usage          = D3D11_USAGE_DYNAMIC;
            vertex_buffer_description.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
            vertex_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

            assertion(
                SUCCEEDED(
                    device->CreateBuffer(&vertex_buffer_description, nullptr, &vertex_buffer)
                ) &&
                "Failed to allocate orphaned vertex buffer"
            );
        }

        // create sampler
        {
            D3D11_SAMPLER_DESC sampler_description = {};
            sampler_description.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            sampler_description.AddressU = sampler_description.AddressV = sampler_description.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            sampler_description.MaxLOD = D3D11_FLOAT32_MAX;
            sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;

            assertion(
                SUCCEEDED(
                    device->CreateSamplerState(&sampler_description, &nearest_neighbor_sampler_state)
                ) &&
                "Failed to create sampler state?"
            );
        }
    }
}

void Direct3D11_Graphics_Driver::swap_and_present() {
    {
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,
            real_resolution.x, real_resolution.y,
            0.0f, 1.0f
        };

        context->RSSetViewports(1, &viewport);
        context->OMSetRenderTargets(1, &rendertarget, nullptr);
    }
    swapchain->Present(1, 0);
}

void Direct3D11_Graphics_Driver::finish() {
    _debugprintf("Direct3D11 Driver Finish");
    context->Release();
    device->Release();

    // TODO: free all resources

    vertex_shader = nullptr;
    pixel_shader = nullptr;

    device = nullptr;
    context = nullptr;
}

void Direct3D11_Graphics_Driver::clear_color_buffer(color32u8 color) {
    // unimplemented("Not done");
    float rgba[] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    context->ClearRenderTargetView(rendertarget, rgba);
}

void Direct3D11_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    if (commands->should_clear_buffer) {
        clear_color_buffer(commands->clear_buffer_color);
    }
    // unimplemented("Not done");
}

V2 Direct3D11_Graphics_Driver::resolution() {
    return virtual_resolution;
}

bool Direct3D11_Graphics_Driver::find_first_free_image(D3D11_Image** result) {
    for (unsigned index = 0; index < MAX_D3D11_TEXTURES; ++index) {
        auto& current_image = images[index];

        if (current_image.texture2d == nullptr && current_image.shader_resource_view == nullptr) {
            *result = &current_image;
            return true;
        }
    }

    return false;
}

void Direct3D11_Graphics_Driver::upload_image_buffer_to_gpu(struct image_buffer* image) {
    auto backing_image_buffer         = image;
    auto image_buffer_driver_resource = (D3D11_Image*)(backing_image_buffer->_driver_userdata);

    if (image_buffer_driver_resource) {
        _debugprintf("Existing driver resource found for (%p)(%p). Do not need to reload.", image, image->_driver_userdata);
        return;
    }

    D3D11_Image* free_image;
    assertion(find_first_free_image(&free_image) && "No more free texture ids. Bump the number");
    *free_image = d3d11_image_from_image_buffer(device, backing_image_buffer);
    backing_image_buffer->_driver_userdata = free_image;
}

void Direct3D11_Graphics_Driver::upload_texture(struct graphics_assets* assets, image_id image) {
    auto backing_image_buffer = graphics_assets_get_image_by_id(assets, image);
    upload_image_buffer_to_gpu(backing_image_buffer);
}

void Direct3D11_Graphics_Driver::upload_font(struct graphics_assets* assets, font_id font) {
    auto backing_image_buffer = (struct image_buffer*)graphics_assets_get_font_by_id(assets, font);
    upload_image_buffer_to_gpu(backing_image_buffer);
}

void Direct3D11_Graphics_Driver::unload_texture(struct graphics_assets* assets, image_id image) {
    _debugprintf("NOP right now.");
}

void Direct3D11_Graphics_Driver::screenshot(char* where) {
    _debugprintf("Direct3D11 driver screenshot is nop");
    // unimplemented("Not done");
}


const char* Direct3D11_Graphics_Driver::get_name(void) {
    return "(Direct3D11)";
}
