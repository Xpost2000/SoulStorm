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
        // assertion(image->_driver_userdata && "How does this image not have a OpenGL texture id?");
        // set_texture_id(*((GLuint*)image->_driver_userdata));
    } else {
        // set_texture_id(white_pixel);
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
    // set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void Direct3D11_Graphics_Driver::render_command_draw_image(const render_command& rc) {
    auto destination_rect = rc.destination;
    auto source_rect      = rc.source;
    auto color            = rc.modulation_u8;
    auto image_buffer     = rc.image;
    // set_blend_mode(rc.blend_mode);
    push_render_quad_vertices(destination_rect, source_rect, color32u8_to_color32f32(color), image_buffer, rc.angle_degrees, rc.angle_y_degrees, rc.flags, rc.rotation_center);
}

void Direct3D11_Graphics_Driver::render_command_draw_line(const render_command& rc) {
    auto start = rc.start;
    auto end   = rc.end;
    auto color = color32u8_to_color32f32(rc.modulation_u8);
    // set_blend_mode(rc.blend_mode);
    // set_texture_id(white_pixel);

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
    // set_blend_mode(rc.blend_mode);

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
        context->VSSetShader(vertex_shader, 0, 0);
        context->PSSetShader(pixel_shader, 0, 0);
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

void Direct3D11_Graphics_Driver::consume_render_commands(struct render_commands* commands) {
    if (commands->should_clear_buffer) {
        clear_color_buffer(commands->clear_buffer_color);
    }
    // unimplemented("Not done");

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
    _debugprintf("NOP right now. TODO");
}

void Direct3D11_Graphics_Driver::screenshot(char* where) {
    _debugprintf("Direct3D11 driver screenshot is nop");
    // unimplemented("Not done");
}


const char* Direct3D11_Graphics_Driver::get_name(void) {
    return "(Direct3D11)";
}
