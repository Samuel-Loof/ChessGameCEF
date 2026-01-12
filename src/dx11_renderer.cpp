// dx11_renderer.cpp - DirectX 11 Renderer Implementation

#include "dx11_renderer.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Vertex shader code (HLSL)
const char* VERTEX_SHADER_CODE = R"(
struct VS_INPUT {
    float3 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.tex = input.tex;
    return output;
}
)";

// Pixel shader code (HLSL)
const char* PIXEL_SHADER_CODE = R"(
Texture2D tex : register(t0);
SamplerState samplerState : register(s0);

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET {
    return tex.Sample(samplerState, input.tex);
}
)";

// Constructor - initialize mouse state
DX11Renderer::DX11Renderer() 
    : hwnd_(nullptr), device_(nullptr), context_(nullptr),
      swap_chain_(nullptr), render_target_view_(nullptr),
      vertex_buffer_(nullptr), texture_(nullptr),
      texture_view_(nullptr), sampler_state_(nullptr),
      vertex_shader_(nullptr), pixel_shader_(nullptr),
      input_layout_(nullptr),
      width_(1280), height_(720),
      mouse_x_(0), mouse_y_(0),
	  mouse_wheel_delta_(0)	  {
    // Initialize mouse buttons
    mouse_buttons_[0] = false;
    mouse_buttons_[1] = false;
    mouse_buttons_[2] = false;
}

// Initialize everything
bool DX11Renderer::Initialize(const std::string& title, int width, int height) {
    width_ = width;
    height_ = height;
    
    if (!CreateAppWindow(title, width, height)) return false;
    if (!InitializeDirectX()) return false;
    if (!CreateVertexBuffer()) return false;
    // if (!CreateTexture(width, height)) return false;  // COMMENT THIS OUT - we create texture dynamically now
    if (!CreateShaders()) return false;
    
    return true;
}

// Create the application window
bool DX11Renderer::CreateAppWindow(const std::string& title, int width, int height) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszClassName = "ChessGameCEFWindowClass";
    
    if (!RegisterClassEx(&wc)) {
        return false;
    }
    
    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    hwnd_ = CreateWindowEx(
        0,
        "ChessGameCEFWindowClass",
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this  // Pass 'this' pointer
    );
    
    if (!hwnd_) {
        return false;
    }
    
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    
    return true;
}

// Initialize DirectX 11
bool DX11Renderer::InitializeDirectX() {
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width_;
    scd.BufferDesc.Height = height_;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd_;
    scd.SampleDesc.Count = 1;
    scd.SampleDesc.Quality = 0;
    scd.Windowed = TRUE;
    
    D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        &feature_level,
        1,
        D3D11_SDK_VERSION,
        &scd,
        &swap_chain_,
        &device_,
        nullptr,
        &context_
    );
    
    if (FAILED(hr)) {
        return false;
    }
    
    ID3D11Texture2D* back_buffer = nullptr;
    swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer);
    device_->CreateRenderTargetView(back_buffer, nullptr, &render_target_view_);
    back_buffer->Release();
    
    context_->OMSetRenderTargets(1, &render_target_view_, nullptr);
    
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)width_;
    viewport.Height = (float)height_;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);
    
    return true;
}

// Create vertex buffer for fullscreen quad
bool DX11Renderer::CreateVertexBuffer() {
    Vertex vertices[] = {
        // Triangle 1
        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
        
        // Triangle 2
        {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f },
    };
    
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA init_data = {};
    init_data.pSysMem = vertices;
    
    HRESULT hr = device_->CreateBuffer(&bd, &init_data, &vertex_buffer_);
    return SUCCEEDED(hr);
}

// Create texture to hold CEF pixels
bool DX11Renderer::CreateTexture(int width, int height) {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &texture_);
    if (FAILED(hr)) {
        return false;
    }
    
    hr = device_->CreateShaderResourceView(texture_, nullptr, &texture_view_);
    if (FAILED(hr)) {
        return false;
    }
    
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MaxAnisotropy = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = device_->CreateSamplerState(&sampler_desc, &sampler_state_);
    return SUCCEEDED(hr);
}

// Compile and create shaders
bool DX11Renderer::CreateShaders() {
    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* error_blob = nullptr;
    
    // Compile vertex shader
    HRESULT hr = D3DCompile(
        VERTEX_SHADER_CODE,
        strlen(VERTEX_SHADER_CODE),
        nullptr, nullptr, nullptr,
        "main", "vs_5_0",
        0, 0,
        &vs_blob, &error_blob
    );
    
    if (FAILED(hr)) {
        if (error_blob) error_blob->Release();
        return false;
    }
    
    hr = device_->CreateVertexShader(
        vs_blob->GetBufferPointer(),
        vs_blob->GetBufferSize(),
        nullptr,
        &vertex_shader_
    );
    
    if (FAILED(hr)) {
        vs_blob->Release();
        return false;
    }
    
    // Create input layout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = device_->CreateInputLayout(
        layout,
        2,
        vs_blob->GetBufferPointer(),
        vs_blob->GetBufferSize(),
        &input_layout_
    );
    
    vs_blob->Release();
    
    if (FAILED(hr)) {
        return false;
    }
    
    // Compile pixel shader
    hr = D3DCompile(
        PIXEL_SHADER_CODE,
        strlen(PIXEL_SHADER_CODE),
        nullptr, nullptr, nullptr,
        "main", "ps_5_0",
        0, 0,
        &ps_blob, &error_blob
    );
    
    if (FAILED(hr)) {
        if (error_blob) error_blob->Release();
        return false;
    }
    
    hr = device_->CreatePixelShader(
        ps_blob->GetBufferPointer(),
        ps_blob->GetBufferSize(),
        nullptr,
        &pixel_shader_
    );
    
    ps_blob->Release();
    
    return SUCCEEDED(hr);
}

// Update texture with CEF pixels
void DX11Renderer::UpdateTexture(const void* buffer, int width, int height) {
    if (!device_ || !context_) return;
    
    // Recreate texture if size changed OR if texture doesn't exist yet
    if (!cef_texture_ || 
        texture_width_ != width || 
        texture_height_ != height) {
        
        // Release old texture if it exists
        if (cef_texture_) {
            cef_texture_->Release();
            cef_texture_ = nullptr;
        }
        if (cef_srv_) {
            cef_srv_->Release();
            cef_srv_ = nullptr;
        }
        
        // Create new texture with new size
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        
        HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &cef_texture_);
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create CEF texture!\n");
            return;
        }
        
        // Create shader resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = desc.Format;
        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;
        
        hr = device_->CreateShaderResourceView(cef_texture_, &srv_desc, &cef_srv_);
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create CEF SRV!\n");
            return;
        }
		
		if (!sampler_state_) {
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MaxAnisotropy = 1;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    
    device_->CreateSamplerState(&sampler_desc, &sampler_state_);
}
        
        // Store the new texture dimensions
        texture_width_ = width;
        texture_height_ = height;
        
        char msg[128];
        sprintf_s(msg, "Created new CEF texture: %dx%d\n", width, height);
        OutputDebugStringA(msg);
    }
    
    // Update texture data
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = context_->Map(cef_texture_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to map CEF texture!\n");
        return;
    }
    
    const uint8_t* src = static_cast<const uint8_t*>(buffer);
    uint8_t* dst = static_cast<uint8_t*>(mapped.pData);
    
    // Copy each row of pixels
    for (int y = 0; y < height; y++) {
        memcpy(dst + y * mapped.RowPitch, 
               src + y * width * 4, 
               width * 4);
    }
    
    context_->Unmap(cef_texture_, 0);
}

// Render current frame
void DX11Renderer::Render() {
    UpdateViewport();
    
    // Just clear to black - CEF window is on top
    float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->ClearRenderTargetView(render_target_view_, clear_color);
    
    // Optional: Draw DirectX background content here if needed
    
    swap_chain_->Present(1, 0);
}


// Update DirectX viewport to match current window size
void DX11Renderer::UpdateViewport() {
    RECT rect;
    GetClientRect(hwnd_, &rect);
    int new_width = rect.right - rect.left;
    int new_height = rect.bottom - rect.top;
    
    if (new_width != width_ || new_height != height_) {
        width_ = new_width;
        height_ = new_height;
        
        // Update DirectX viewport
        D3D11_VIEWPORT viewport = {};
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        viewport.Width = (float)width_;
        viewport.Height = (float)height_;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        context_->RSSetViewports(1, &viewport);
        
        char msg[128];
        sprintf_s(msg, "[DX11] Viewport updated: %dx%d\n", width_, height_);
        OutputDebugStringA(msg);
        
        // Notify CEF via callback
        if (resize_callback_) {
            resize_callback_(width_, height_);
        }
    }
}

// Process Windows messages
bool DX11Renderer::ProcessMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK DX11Renderer::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    DX11Renderer* renderer = nullptr;
    
    if (msg == WM_CREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lparam;
        renderer = (DX11Renderer*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)renderer);
    } else {
        renderer = (DX11Renderer*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (renderer) {
        switch (msg) {
            case WM_MOUSEMOVE:
                renderer->mouse_x_ = LOWORD(lparam);
                renderer->mouse_y_ = HIWORD(lparam);
                break;
            
            case WM_LBUTTONDOWN:
			OutputDebugStringA("WM_LBUTTONDOWN received\n");
                renderer->mouse_buttons_[0] = true;
                break;
            
            case WM_LBUTTONUP:
                renderer->mouse_buttons_[0] = false;
                break;
            
            case WM_RBUTTONDOWN:
                renderer->mouse_buttons_[1] = true;
                break;
            
            case WM_RBUTTONUP:
                renderer->mouse_buttons_[1] = false;
                break;
            
            case WM_MBUTTONDOWN:
                renderer->mouse_buttons_[2] = true;
                break;
            
            case WM_MBUTTONUP:
                renderer->mouse_buttons_[2] = false;
                break;
            
            case WM_MOUSEWHEEL:
                renderer->mouse_wheel_delta_ = GET_WHEEL_DELTA_WPARAM(wparam);
                break;
            
            // Capture keyboard events
            // These only fire when OUR window has focus
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_CHAR:
            case WM_SYSCHAR:
                {
                    KeyboardEvent evt;
                    evt.msg = msg;
                    evt.wparam = wparam;
                    evt.lparam = lparam;
                    renderer->key_queue_.push(evt);
                }
                break;
            
            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

// Get next keyboard event from queue
bool DX11Renderer::GetNextKeyEvent(KeyboardEvent& evt) {
    if (key_queue_.empty()) {
        return false;
    }
    evt = key_queue_.front();
    key_queue_.pop();
    return true;
}

int DX11Renderer::GetMouseWheelDelta() {
    int delta = mouse_wheel_delta_;
    mouse_wheel_delta_ = 0;  // Reset after reading
    return delta;
}

//Input query functions
bool DX11Renderer::IsMouseButtonDown(int button) const {
    return (button >= 0 && button < 3) ? mouse_buttons_[button] : false;
}

void DX11Renderer::GetMousePosition(int& x, int& y) const {
    x = mouse_x_;
    y = mouse_y_;
}

bool DX11Renderer::IsKeyDown(int vk_code) const {
    return (GetAsyncKeyState(vk_code) & 0x8000) != 0;
}

// Cleanup
void DX11Renderer::Cleanup() {
    // NEW: Clean up CEF textures
    if (cef_srv_) cef_srv_->Release();
    if (cef_texture_) cef_texture_->Release();
    
    // Existing cleanup
    if (input_layout_) input_layout_->Release();
    if (pixel_shader_) pixel_shader_->Release();
    if (vertex_shader_) vertex_shader_->Release();
    if (sampler_state_) sampler_state_->Release();
    if (texture_view_) texture_view_->Release();
    if (texture_) texture_->Release();
    if (vertex_buffer_) vertex_buffer_->Release();
    if (render_target_view_) render_target_view_->Release();
    if (swap_chain_) swap_chain_->Release();
    if (context_) context_->Release();
    if (device_) device_->Release();
}