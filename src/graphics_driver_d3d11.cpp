// TODO:
// I do not know how to use blending yet.
#include "graphics_driver_d3d11.h"
#include "engine.h"
#undef far
#undef near
// why windows.h do you do this to me.

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

void D3D11_Image::Release() {
    _debugprintf("Unloading D3D11 Image Object");
    if (texture2d) {
        texture2d->Release();
        texture2d = nullptr;
    }

    if (shader_resource_view) {
        shader_resource_view->Release();
        shader_resource_view = nullptr;
    }
}

local const char* pixel_shader_source = R"shader(
    struct VertexShader_Result 
    {
        float4 position_clip : SV_POSITION;
        float4 color         : COLOR;
        float2 texcoord      : TEXCOORD;
    };
    
    Texture2D sprite_tex2D : register(t0);
    SamplerState sprite_sampler : register(s0);

    float4 main(VertexShader_Result input) : SV_Target {
        return sprite_tex2D.Sample(sprite_sampler, input.texcoord) * input.color;
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
        float4 color         : COLOR;
        float2 texcoord      : TEXCOORD;
    };

    cbuffer MatrixBuffer : register(b0)
    {
          float4x4 projection_matrix;
          float4x4 view_matrix;
          float    global_elapsed_time;
    };

    VertexShader_Result main(VertexShader_Params input) {
        VertexShader_Result result;
        // NOTE:
        // I want to use the same matrices I do with the opengl side
        // so just flip all the matrix multiplication math I guess.
        result.position_clip = mul(projection_matrix, mul(view_matrix, float4(input.position, 0.0f, 1.0f)));
        // result.position_clip = float4(input.position, 1.0f, 1.0f);
        // result.position_clip = mul(view_matrix, float4(input.position, 1.0f, 1.0f));
        // result.position_clip = mul(float4(input.position, 1.0f, 1.0f), view_matrix);
        result.color = input.color;
        result.texcoord = input.texcoord;
        return result;
    }
)shader";

// D3D11 Helpers Begin

D3D11_Image d3d11_image_from_image_buffer(ID3D11Device* device, ID3D11DeviceContext* context, struct image_buffer* image_buffer) {
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
        context->GenerateMips(result.shader_resource_view);
    }

    _debugprintf("D3D11 texture: %p srv %p", result.texture2d, result.shader_resource_view);
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
        zero_array(blending_states);
    }
    initialize_backbuffer(V2(width, height));
}

void Direct3D11_Graphics_Driver::initialize_default_rendering_resources(void) {
    if (initialized_resources) {
        return;
    }

    // Create D3D11 Resources here
    {
        {
            D3D11_BLEND_DESC blend_description;
            zero_memory(&blend_description, sizeof(blend_description));
            blend_description.RenderTarget[0].BlendEnable = false;
            blend_description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            assertion(
                SUCCEEDED(
                    device->CreateBlendState(&blend_description, &blending_states[BLEND_MODE_NONE])
                )
            );
        }
        {
            D3D11_BLEND_DESC blend_description;
            zero_memory(&blend_description, sizeof(blend_description));
            blend_description.RenderTarget[0].BlendEnable = true;

            blend_description.RenderTarget[0].SrcBlend  = D3D11_BLEND_SRC_ALPHA;
            blend_description.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            blend_description.RenderTarget[0].BlendOp   = D3D11_BLEND_OP_ADD;

            blend_description.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
            blend_description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            blend_description.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;

            blend_description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            assertion(
                SUCCEEDED(
                    device->CreateBlendState(&blend_description, &blending_states[BLEND_MODE_ALPHA])
                )
            );
        }

        {
            D3D11_BLEND_DESC blend_description;
            zero_memory(&blend_description, sizeof(blend_description));
            blend_description.RenderTarget[0].BlendEnable = true;

            blend_description.RenderTarget[0].SrcBlend  = D3D11_BLEND_SRC_ALPHA;
            blend_description.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            blend_description.RenderTarget[0].BlendOp   = D3D11_BLEND_OP_ADD;

            blend_description.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
            blend_description.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
            blend_description.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;

            blend_description.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

            assertion(
                SUCCEEDED(
                    device->CreateBlendState(&blend_description, &blending_states[BLEND_MODE_ADDITIVE])
                )
            );
        }
    }
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

        // Allocate constant buffer
        {
            D3D11_BUFFER_DESC matrix_constant_buffer_description;
            zero_memory(&matrix_constant_buffer_description, sizeof(matrix_constant_buffer_description));
            matrix_constant_buffer_description.Usage          = D3D11_USAGE_DYNAMIC;
            matrix_constant_buffer_description.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
            matrix_constant_buffer_description.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            matrix_constant_buffer_description.ByteWidth      = sizeof(D3D11_Matrix_Constant_Buffer);

            D3D11_Matrix_Constant_Buffer initial_constant_buffer_data;
            float identity4x4[] = {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1,
            };
            {
                memory_copy(identity4x4, initial_constant_buffer_data.view_matrix, sizeof(identity4x4));
                memory_copy(identity4x4, initial_constant_buffer_data.projection_matrix, sizeof(identity4x4));
            }
            
            initial_constant_buffer_data.global_elapsed_time = Global_Engine()->global_elapsed_time;

            D3D11_SUBRESOURCE_DATA initial_data;
            zero_memory(&initial_data, sizeof(initial_data));
            initial_data.pSysMem = &initial_constant_buffer_data;
            assertion(
                SUCCEEDED(
                    device->CreateBuffer(&matrix_constant_buffer_description, &initial_data, &matrix_constant_buffer)
                ) &&
                "Initial matrix constant buffer setup failure?"
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

        // white pixel
        {
            struct image_buffer white_pixel_image_buffer = image_buffer_create_blank(1, 1);
            *white_pixel_image_buffer.pixels_u32         = 0xFFFFFFFF;
            white_pixel                                  = d3d11_image_from_image_buffer(device, context, &white_pixel_image_buffer);
            image_buffer_free(&white_pixel_image_buffer);
        }
    }

    initialized_resources = true;
}

void Direct3D11_Graphics_Driver::setup_viewport(void) {
    {
        D3D11_VIEWPORT viewport = {
            0.0f, 0.0f,
            real_resolution.x, real_resolution.y,
            0.0f, 1.0f
        };

        context->RSSetViewports(1, &viewport);
    }
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

    if (!initialized_resources) {
        DXGI_SWAP_CHAIN_DESC swapchain_description;
        zero_memory(&swapchain_description, sizeof(swapchain_description));
        swapchain_description.BufferDesc.Width  = real_resolution.x;
        swapchain_description.BufferDesc.Height = real_resolution.y;
        swapchain_description.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

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
        swapchain_description.Windowed = !Global_Engine()->fullscreen; // TODO: check windowed status from engine later.

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

    } else {
        // Update swapchain and reset rendertarget view texture
        _debugprintf("Resizing swap chain to %d x %d", (s32)real_resolution.x, (s32)real_resolution.y);

        if (swapchain_framebuffer_texture) {
            swapchain_framebuffer_texture->Release();
            swapchain_framebuffer_texture = nullptr;
        }

        if (rendertarget) {
            rendertarget->Release();
            rendertarget = nullptr;
        }

        // swapchain->ResizeBuffers(2, (s32)real_resolution.x, (s32)real_resolution.y, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        assertion(SUCCEEDED(swapchain->ResizeBuffers(2, (s32)real_resolution.x, (s32)real_resolution.y, DXGI_FORMAT_B8G8R8A8_UNORM, 0)) && "? failure to resize backbuffer?");
        assertion(SUCCEEDED(swapchain->SetFullscreenState(Global_Engine()->fullscreen, nullptr) && "Failure to set fullscreen state for swapchain?"));
    }

    // swapchain_framebuffer_texture holds backbuffer
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&swapchain_framebuffer_texture);
    device->CreateRenderTargetView(
        swapchain_framebuffer_texture,
        nullptr,
        &rendertarget
    );

    setup_viewport();
    context->OMSetRenderTargets(1, &rendertarget, nullptr);

    initialize_default_rendering_resources();
}

void Direct3D11_Graphics_Driver::swap_and_present() {
    swapchain->Present(1, 0);
}

void Direct3D11_Graphics_Driver::finish() {
    _debugprintf("Direct3D11 Driver Finish");
    initialized_resources = false;

    quad_vertices.clear();

    {
        matrix_constant_buffer->Release();
        matrix_constant_buffer = nullptr;
    }
    {
        vertex_buffer->Release();
        vertex_buffer = nullptr;
    }
    {
        current_image = nullptr;
        current_blend_mode = -1;
    }
    {
        vertex_shader->Release();
        vertex_shader = nullptr;
    }
    {
        pixel_shader->Release();
        pixel_shader = nullptr;
    }
    {
        swapchain->Release();
        swapchain = nullptr;
    }
    {
        rendertarget->Release();
        rendertarget = nullptr;
    }
    {
        swapchain_framebuffer_texture = nullptr; // NOTE: should be owned by swapchain?
    }
    {
        vertex_layout->Release();
        vertex_layout = nullptr;
    }
    {
        nearest_neighbor_sampler_state->Release();
        nearest_neighbor_sampler_state = nullptr;
    }

    for (unsigned image_index = 0; image_index < MAX_D3D11_TEXTURES; ++image_index) {
        auto& image = images[image_index];
        image.Release();
    }

    white_pixel.Release();

    for (unsigned blend_mode_index = 0; blend_mode_index < BLEND_MODE_COUNT; ++blend_mode_index) {
        if (blending_states[blend_mode_index]) {
            blending_states[blend_mode_index]->Release();
            blending_states[blend_mode_index] = nullptr;
        }
    }


    context->Release();
    device->Release();
    device = nullptr;
    context = nullptr;
}

void Direct3D11_Graphics_Driver::clear_color_buffer(color32u8 color) {
    // unimplemented("Not done");
    float rgba[] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    context->ClearRenderTargetView(rendertarget, rgba);
}

void Direct3D11_Graphics_Driver::set_texture_id(D3D11_Image* image) {
    // return;
    if (current_image != image) {
        flush_and_render_quads();
        // bind the new image
        current_image = image;

        context->PSSetSamplers(0, 1, &nearest_neighbor_sampler_state);
        context->PSSetShaderResources(0, 1, &current_image->shader_resource_view);
    }
}

// NOTE: copied from the opengl driver. They're basically the same tbh.
void Direct3D11_Graphics_Driver::push_render_quad_vertices(
    rectangle_f32        destination,
    rectangle_f32        source,
    color32f32           color,
    struct image_buffer* image,
    s32                  angle,
    s32                  angle_y,
    u32                  flags,
    V2                   rotation_origin
) {
    if (image) {
        u32 image_width   = (image->pot_square_size) ? image->pot_square_size : image->width;
        u32 image_height  = (image->pot_square_size) ? image->pot_square_size : image->height;

        if ((source.x == 0) && (source.y == 0) && (source.w == 0) && (source.h == 0)) {
            source.w = image_width;
            source.h = image_height;
        }

        // normalize coordinates.
        source.x         /= image_width;
        source.y         /= image_height;
        source.w         /= image_width;
        source.h         /= image_height;


        if (image->_driver_userdata == nullptr) {
            _debugprintf("Warning? No d3d11 texture image id? Uploading dynamically.");
            upload_image_buffer_to_gpu(image);
        }
        set_texture_id((D3D11_Image*)image->_driver_userdata);
    } else {
        set_texture_id(&white_pixel);
    }

    f32 c  = cosf(degree_to_radians(angle));
    f32 s  = sinf(degree_to_radians(angle));
    f32 c1 = cosf(degree_to_radians(angle_y));
    f32 s1 = sinf(degree_to_radians(angle_y));

    // rotate the basis axes
    // instead, since I cannot rotate each vertex.

    // although I do still have to rotate the top left vertex based on the basis differences.
    V2 down_basis  = V2_rotate(V2(0, destination.h), angle, angle_y);
    V2 right_basis = V2_rotate(V2(destination.w, 0), angle, angle_y);

    D3D11_Vertex_Format top_left;
    {
        V2 displacement = V2(destination.x + ((rotation_origin.x) * destination.w), destination.y + ((rotation_origin.y) * destination.h));
        top_left.position =
            V2_rotate(
                V2(destination.x, destination.y) - displacement,
                angle, angle_y)
            + displacement;
        top_left.color    = color;
    }

    D3D11_Vertex_Format top_right;
    {
        top_right.position = top_left.position + right_basis;
        top_right.color    = color;
    }
    D3D11_Vertex_Format bottom_left;
    {
        bottom_left.position = top_left.position + down_basis;
        bottom_left.color    = color;
    }
    D3D11_Vertex_Format bottom_right;
    {
        bottom_right.position = top_left.position + right_basis + down_basis;
        bottom_right.color    = color;
    }


#if 1
    top_left.texcoord     = V2(source.x, source.y);
    top_right.texcoord    = V2(source.x + source.w, source.y);
    bottom_left.texcoord  = V2(source.x, source.y + source.h);
    bottom_right.texcoord = V2(source.x + source.w, source.y + source.h);
#else
    top_left.texcoord     = V2(source.x, source.y + source.h);
    top_right.texcoord    = V2(source.x + source.w, source.y + source.h);
    bottom_left.texcoord  = V2(source.x, source.y);
    bottom_right.texcoord = V2(source.x + source.w, source.y);
#endif

    if ((flags & DRAW_IMAGE_FLIP_HORIZONTALLY)) {
        top_left.texcoord.x = source.x + source.w;
        top_right.texcoord.x = source.x;
        bottom_left.texcoord.x = source.x + source.w;
        bottom_right.texcoord.x = source.x;
    }

    if ((flags & DRAW_IMAGE_FLIP_VERTICALLY)) {
        top_left.texcoord.y = source.y + source.h;
        top_right.texcoord.y = source.y + source.h;
        bottom_left.texcoord.y = source.y;
        bottom_right.texcoord.y = source.y;
    }

    quad_vertices.push(top_left);
    quad_vertices.push(top_right);
    quad_vertices.push(bottom_left);
    quad_vertices.push(bottom_left);
    quad_vertices.push(top_right);
    quad_vertices.push(bottom_right);

    if (quad_vertices.size >= quad_vertices.capacity) {
        flush_and_render_quads();
    }
}

void Direct3D11_Graphics_Driver::render_command_draw_quad(const render_command& rc) {
    auto destination_rect = rc.destination;
    auto source_rect      = rc.source;
    auto color            = rc.modulation_u8;
    auto image_buffer     = rc.image;
    set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void Direct3D11_Graphics_Driver::render_command_draw_image(const render_command& rc) {
    auto destination_rect = rc.destination;
    auto source_rect      = rc.source;
    auto color            = rc.modulation_u8;
    auto image_buffer     = rc.image;
    set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void Direct3D11_Graphics_Driver::render_command_draw_line(const render_command& rc) {
    auto start = rc.start;
    auto end   = rc.end;
    auto color = color32u8_to_color32f32(rc.modulation_u8);
    set_blend_mode(rc.blend_mode);
    set_texture_id(&white_pixel);

    auto line_normal = V2_perpendicular(V2_direction(start, end));
    {
        D3D11_Vertex_Format top_left;
        {
            top_left.position = V2(start.x, start.y);
            top_left.color    = color;
        }
        D3D11_Vertex_Format top_right;
        {
            top_right.position = V2(start.x, start.y) + line_normal * 1;
            top_right.color    = color;
        }
        D3D11_Vertex_Format bottom_left;
        {
            bottom_left.position = V2(end.x, end.y);
            bottom_left.color    = color;
        }
        D3D11_Vertex_Format bottom_right;
        {
            bottom_right.position = V2(end.x, end.y) + line_normal * 1;
            bottom_right.color    = color;
        }

        quad_vertices.push(top_left);
        quad_vertices.push(top_right);
        quad_vertices.push(bottom_left);
        quad_vertices.push(bottom_left);
        quad_vertices.push(top_right);
        quad_vertices.push(bottom_right);

        if (quad_vertices.size >= quad_vertices.capacity) {
            flush_and_render_quads();
        }
    }
}

void Direct3D11_Graphics_Driver::render_command_draw_text(const render_command& rc) {
    auto font     = rc.font;
    auto scale    = rc.scale;
    auto position = rc.xy;
    auto text     = rc.text;
    auto color    = rc.modulation_u8;
    set_blend_mode(rc.blend_mode);

    {
        f32 x_cursor = position.x;
        f32 y_cursor = position.y;
        for (unsigned index = 0; index < text.length; ++index) {
            if (text.data[index] == '\n') {
                y_cursor += font->tile_height * scale;
                x_cursor =  position.x;
            } else {
                s32 character = text.data[index] - 32;
                auto destination_rect = rectangle_f32(
                    x_cursor, y_cursor,
                    font->tile_width  * scale,
                    font->tile_height * scale
                );

                auto source_rect = rectangle_f32(
                    (character % font->atlas_cols) * font->tile_width,
                    (character / font->atlas_cols) * font->tile_height,
                    font->tile_width, font->tile_height
                );

                push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), (struct image_buffer*)font);
                x_cursor += font->tile_width * scale;
            }
        }
    }
}

void Direct3D11_Graphics_Driver::render_command_draw_set_scissor(const render_command& rc) {
    unimplemented("set scissor not implemented");
}

void Direct3D11_Graphics_Driver::render_command_draw_clear_scissor(const render_command& rc) {
    unimplemented("clear scissor not implemented");
}

void Direct3D11_Graphics_Driver::flush_and_render_quads(void) {
    if (quad_vertices.size == 0) {
        return;
    }

    {
        // map the vertex buffer and write all the quad data to it...
        {
            D3D11_MAPPED_SUBRESOURCE mapped;
            assertion(SUCCEEDED(context->Map(vertex_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && "Could not map vertex buffer for writing?");
            memory_copy(quad_vertices.data, mapped.pData, quad_vertices.size * sizeof(*quad_vertices.data));
            context->Unmap(vertex_buffer, 0);
        }
        context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->IASetInputLayout(vertex_layout);
        {
            unsigned int stride = sizeof(D3D11_Vertex_Format);
            unsigned int offset = 0;
            context->IASetVertexBuffers(0, 1, &vertex_buffer, &stride, &offset);
        }
        context->Draw(quad_vertices.size, 0);
    }

    quad_vertices.clear();
}

void Direct3D11_Graphics_Driver::set_blend_mode(u8 blend_mode) {
    if (blend_mode != current_blend_mode) {
        flush_and_render_quads();
        current_blend_mode = blend_mode;
        context->OMSetBlendState(blending_states[blend_mode], 0, 0xffffffff);
    }
}

void Direct3D11_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    set_blend_mode(BLEND_MODE_ALPHA);

    if (commands->should_clear_buffer) {
        clear_color_buffer(commands->clear_buffer_color);
    }
    // unimplemented("Not done");

    {
        // write shader constant buffer
        {
            {
                D3D11_MAPPED_SUBRESOURCE mapped;
                assertion(SUCCEEDED(context->Map(matrix_constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)) && "Could not map vertex buffer for writing?");
                D3D11_Matrix_Constant_Buffer* constant_buffer = (D3D11_Matrix_Constant_Buffer*)mapped.pData;
                {
                    // projection matrix
                    {
                        float right  = virtual_resolution.x;
                        float left   = 0;
                        float top    = 0;
                        float bottom = virtual_resolution.y;

                        float far = 100.0f;
                        float near = 0.1f;
                        float orthographic_matrix[] = {
                            2 / (right - left), 0, 0, 0,
                            0, 2 / (top - bottom), 0, 0,
                            0, 0, 2 / (far - near), 0,
                                                                                    // NOTE: different from the OpenGL matrix, Z convention is different in DirectX's NDC.
                            -(right+left)/(right-left), -(top+bottom)/(top-bottom), (far-near)/(far+near), 1
                        };

                        memory_copy(orthographic_matrix, constant_buffer->projection_matrix, sizeof(orthographic_matrix));
                    }
                    // view matrix
                    {
                        const auto& camera = commands->camera;
                        float transform_x = -(camera.xy.x + camera.trauma_displacement.x);
                        float transform_y = -(camera.xy.y + camera.trauma_displacement.y);
                        float scale_x     = camera.zoom;
                        float scale_y     = camera.zoom;

                        if (camera.centered) {
                            transform_x += commands->screen_width/2;
                            transform_y += commands->screen_height/2;
                        }

                        float view_matrix[] = {
                            scale_x, 0,       0,                          0,
                            0,       scale_y, 0,                          0,
                            0,       0,       1.0f,                       0.0,
                            transform_x,       transform_y,       0,    1.0
                        };

                        memory_copy(view_matrix, constant_buffer->view_matrix, sizeof(view_matrix));
                    }
                    constant_buffer->global_elapsed_time = Global_Engine()->global_elapsed_time;
                }
                context->Unmap(matrix_constant_buffer, 0);
            }
        }
        context->VSSetConstantBuffers(0, 1, &matrix_constant_buffer);
    }

    context->VSSetShader(vertex_shader, 0, 0);
    context->PSSetShader(pixel_shader, 0, 0);
    for (unsigned command_index = 0; command_index < commands->command_count; ++command_index) {
        auto& command = commands->commands[command_index];

        switch (command.type) {
            case RENDER_COMMAND_DRAW_QUAD: {
                render_command_draw_quad(command);
            } break;
            case RENDER_COMMAND_DRAW_IMAGE: {
                render_command_draw_image(command);
            } break;
            case RENDER_COMMAND_DRAW_TEXT: {
                render_command_draw_text(command);
            } break;
            case RENDER_COMMAND_DRAW_LINE: {
                render_command_draw_line(command);
            } break;
            case RENDER_COMMAND_SET_SCISSOR: {
                render_command_draw_set_scissor(command);
            } break;
            case RENDER_COMMAND_CLEAR_SCISSOR: {
                render_command_draw_clear_scissor(command);
            } break;
            case RENDER_COMMAND_POSTPROCESS_APPLY_SHADER: {
                unimplemented("I haven't had to use this, and I hope I'm not going to use it.");
            } break;
        }
    }

    flush_and_render_quads();
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
    *free_image = d3d11_image_from_image_buffer(device, context, backing_image_buffer);
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
    auto backing_image_buffer = graphics_assets_get_image_by_id(assets, image);
    auto image_object = (D3D11_Image*)backing_image_buffer->_driver_userdata;
    if (image_object) {
        image_object->Release();
        backing_image_buffer->_driver_userdata = nullptr;
    }
}

void Direct3D11_Graphics_Driver::screenshot(char* where) {
    _debugprintf("Direct3D11 driver screenshot is nop");
    // unimplemented("Not done");
}


const char* Direct3D11_Graphics_Driver::get_name(void) {
    return "(Direct3D11)";
}
