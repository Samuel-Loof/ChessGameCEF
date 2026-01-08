// dx11_renderer.h - DirectX 11 Renderer
// This class handles creating a window and rendering textures with DirectX

#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <string>
#include <queue>

// Simple vertex structure for our quad
// A quad is 2 triangles that make a rectangle
// We'll render the CEF texture onto this quad
struct Vertex {
    float x, y, z;    // Position in 3D space
    float u, v;       // Texture coordinates (0-1 range)
};

// Keyboard event structure
struct KeyboardEvent {
    UINT msg;      // WM_KEYDOWN, WM_KEYUP, WM_CHAR
    WPARAM wparam; // Virtual key code or character
    LPARAM lparam; // Key flags
};

// DirectX11 Renderer class
// Handles window creation, DirectX device, and rendering
class DX11Renderer {
public:

void GetWindowSize(int& width, int& height) {
    RECT rect;
    GetClientRect(hwnd_, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
}

    // Just declare the constructor, definition will be in cpp file
    DX11Renderer();
    
    ~DX11Renderer() { Cleanup(); }
    
    // Initialize window and DirectX
    // Returns true if successful
    bool Initialize(const std::string& title, int width, int height);
    
    // Update the texture with new pixel data from CEF
    // buffer: raw BGRA pixel data from CEF
    // width, height: dimensions of the buffer
    void UpdateTexture(const void* buffer, int width, int height);
    
    // Render the current frame
    // Clears screen and draws the CEF texture
    void Render();
    
    // Handle Windows messages (resize, close, etc)
    // Returns true if message was handled
    bool ProcessMessages();
    
    // Get window handle (needed for CEF)
    HWND GetWindowHandle() const { return hwnd_; }
    
    // Get mouse input for CEF
    // Returns true if mouse button is down
    bool IsMouseButtonDown(int button) const;

    // Get current mouse position
    void GetMousePosition(int& x, int& y) const;

    // Check if a key is pressed
    bool IsKeyDown(int vk_code) const;
	
	int GetMouseWheelDelta();  // Get scroll wheel delta
	
	bool GetNextKeyEvent(KeyboardEvent& evt);  // Get next keyboard event

private:
    // Create the window
    bool CreateAppWindow(const std::string& title, int width, int height);
    
    // Initialize DirectX device, swap chain, etc
    bool InitializeDirectX();
    
    // Create vertex buffer for the quad we'll render to
    bool CreateVertexBuffer();
    
    // Create texture that will hold CEF's pixels
    bool CreateTexture(int width, int height);
    
    // Compile and create shaders
    bool CreateShaders();
    
    // Clean up all DirectX resources
    void Cleanup();
    
    // Windows message callback (static because Windows API requires it)
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    
    // Window and DirectX members
    HWND hwnd_;                                    // Window handle
    ID3D11Device* device_;                         // DirectX device
    ID3D11DeviceContext* context_;                 // DirectX context (for drawing)
    IDXGISwapChain* swap_chain_;                   // Swap chain (double buffering)
    ID3D11RenderTargetView* render_target_view_;   // Where we render to
    ID3D11Buffer* vertex_buffer_;                  // Vertex data for quad
    ID3D11Texture2D* texture_;                     // CEF pixels stored here
    ID3D11ShaderResourceView* texture_view_;       // View of texture for shader
    ID3D11SamplerState* sampler_state_;            // Texture sampling settings
    ID3D11VertexShader* vertex_shader_;            // Vertex shader
    ID3D11PixelShader* pixel_shader_;              // Pixel shader
    ID3D11InputLayout* input_layout_;              // Vertex format description
    
    int width_;   // Window width
    int height_;  // Window height
    
    // Track mouse state
    int mouse_x_;              // Current mouse X position
    int mouse_y_;              // Current mouse Y position
    bool mouse_buttons_[3];    // Left, Right, Middle
	int mouse_wheel_delta_; // Scroll wheel delta
	std::queue<KeyboardEvent> key_queue_;  // Queue of keyboard events
	
	int texture_width_ = 0;   // Track current texture size
    int texture_height_ = 0;
	
	 ID3D11Texture2D* cef_texture_ = nullptr;           // CEF rendering texture
    ID3D11ShaderResourceView* cef_srv_ = nullptr;      // Shader resource view for CEF texture
};
