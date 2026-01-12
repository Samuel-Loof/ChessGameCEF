// main.cpp - CEF + DirectX 11 with FULL Input (Mouse, Scroll, Keyboard) - FIXED

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "dx11_renderer.h"
#include <windows.h>
#include <mutex>

void DebugPrint(const char* msg) {
    char buffer[256];
    sprintf_s(buffer, "[CEF DEBUG] %s\n", msg);
    OutputDebugStringA(buffer);
}

class RenderHandler;

// Off-screen render handler with proper viewport handling
class RenderHandler : public CefRenderHandler {
public:
    RenderHandler(DX11Renderer* renderer) 
        : renderer_(renderer), 
          browser_width_(1600), 
          browser_height_(900),
          texture_width_(1600),
          texture_height_(900) {}
    
    void SetBrowserSize(int width, int height) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        browser_width_ = width;
        browser_height_ = height;
        texture_width_ = width;
        texture_height_ = height;
        
        char msg[256];
        sprintf_s(msg, "[RenderHandler] SetBrowserSize: %dx%d\n", width, height);
        OutputDebugStringA(msg);
    }
    
    // Dynamic viewport based on browser size (CRITICAL!)
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        rect.x = 0;
        rect.y = 0;
        rect.width = browser_width_;
        rect.height = browser_height_;
        
        char msg[256];
        sprintf_s(msg, "[GetViewRect] Returning: %dx%d\n", browser_width_, browser_height_);
        OutputDebugStringA(msg);
    }
    
    // Provide screen info for proper DPI scaling - NO SCALING (1.0)
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        int current_width = browser_width_;
        int current_height = browser_height_;
        
        screen_info.device_scale_factor = 1.0f;  // NO scaling - 1:1 pixels
        screen_info.depth = 24;
        screen_info.depth_per_component = 8;
        screen_info.is_monochrome = false;
        screen_info.rect = CefRect(0, 0, current_width, current_height);
        screen_info.available_rect = CefRect(0, 0, current_width, current_height);
        
        char msg[256];
        sprintf_s(msg, "[GetScreenInfo] %dx%d (scale=1.0)\n", current_width, current_height);
        OutputDebugStringA(msg);
        
        return true;
    }
    
    // Provide screen point mapping
    virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, 
                               int viewX, int viewY, 
                               int& screenX, int& screenY) override {
        screenX = viewX;
        screenY = viewY;
        return true;
    }
    
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                        PaintElementType type,
                        const RectList& dirtyRects,
                        const void* buffer,
                        int width,
                        int height) override {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // DIAGNOSTIC: Log if sizes don't match
        static int paint_count = 0;
        if (paint_count++ % 60 == 0) {  // Log every 60 frames
            char msg[512];
            sprintf_s(msg, "[OnPaint #%d] CEF painted: %dx%d | Browser thinks: %dx%d | %s\n",
                paint_count, width, height, browser_width_, browser_height_,
                (width == browser_width_ && height == browser_height_) ? "MATCH ✓" : "MISMATCH ✗");
            OutputDebugStringA(msg);
        }
        
        if (renderer_) {
            renderer_->UpdateTexture(buffer, width, height);
        }
    }
    
    IMPLEMENT_REFCOUNTING(RenderHandler);
    
private:
    DX11Renderer* renderer_;
    int browser_width_;
    int browser_height_;
    int texture_width_;
    int texture_height_;
    std::mutex mutex_;
};

// Load handler to inject responsive JavaScript - AGGRESSIVE VIEWPORT
class LoadHandler : public CefLoadHandler {
public:
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
                          CefRefPtr<CefFrame> frame,
                          int httpStatusCode) override {
        if (frame->IsMain() && httpStatusCode == 200) {
            // AGGRESSIVE viewport injection
            std::string viewport_js = R"(
                (function() {
                    // Remove ALL existing viewport metas
                    document.querySelectorAll('meta[name="viewport"]').forEach(m => m.remove());
                    
                    // Create fresh viewport meta
                    const viewportMeta = document.createElement('meta');
                    viewportMeta.name = 'viewport';
                    // CRITICAL: Use device-width for proper reflow
                    viewportMeta.content = 'width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no';
                    document.head.insertBefore(viewportMeta, document.head.firstChild);
                    
                    // Force immediate CSS update
                    document.documentElement.style.width = '100%';
                    document.documentElement.style.height = '100%';
                    document.documentElement.style.overflow = 'hidden';
                    document.body.style.width = '100%';
                    document.body.style.height = '100%';
                    document.body.style.margin = '0';
                    document.body.style.padding = '0';
                    document.body.style.overflow = 'hidden';
                    
                    // Global resize handler
                    window.__CEF_RESIZE_HANDLER = function() {
                        const width = window.innerWidth;
                        const height = window.innerHeight;
                        
                        // Update CSS custom properties
                        document.documentElement.style.setProperty('--viewport-width', width + 'px');
                        document.documentElement.style.setProperty('--viewport-height', height + 'px');
                        
                        // Force CSS recalculation
                        const el = document.documentElement;
                        el.style.display = 'none';
                        el.offsetHeight; // Trigger reflow
                        el.style.display = '';
                        
                        console.log('[CEF RESIZE]', width + 'x' + height);
                    };
                    
                    // Set up event listener
                    window.addEventListener('resize', window.__CEF_RESIZE_HANDLER);
                    
                    // Initial call
                    setTimeout(() => window.__CEF_RESIZE_HANDLER(), 100);
                    
                    console.log('[CEF] Viewport meta injected and initialized');
                })();
            )";
            
            frame->ExecuteJavaScript(viewport_js, frame->GetURL(), 0);
            OutputDebugStringA("[LOAD] Injected aggressive viewport JavaScript\n");
            
            // Also inject CSS to ensure proper sizing
            std::string css_js = R"(
                (function() {
                    const style = document.createElement('style');
                    style.textContent = `
                        html, body, #__next, #root {
                            width: 100% !important;
                            height: 100% !important;
                            margin: 0 !important;
                            padding: 0 !important;
                            overflow: hidden !important;
                            box-sizing: border-box !important;
                        }
                        * {
                            box-sizing: inherit !important;
                        }
                    `;
                    document.head.appendChild(style);
                })();
            )";
            
            frame->ExecuteJavaScript(css_js, frame->GetURL(), 0);
        }
    }
    
    IMPLEMENT_REFCOUNTING(LoadHandler);
};

class BrowserHandler : public CefClient, public CefLifeSpanHandler {
public:
    BrowserHandler(DX11Renderer* renderer) 
        : render_handler_(new RenderHandler(renderer)),
          load_handler_(new LoadHandler()),
          browser_(nullptr) {}
    
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }
    
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }
    
    virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override {
        return load_handler_;
    }
    
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        browser_ = browser;
    }
    
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        browser_ = nullptr;
        CefQuitMessageLoop();
    }
    
    CefRefPtr<CefBrowser> GetBrowser() { return browser_; }
    CefRefPtr<RenderHandler> GetRenderHandlerRef() { return render_handler_; }
    
    IMPLEMENT_REFCOUNTING(BrowserHandler);
    
private:
    CefRefPtr<RenderHandler> render_handler_;
    CefRefPtr<LoadHandler> load_handler_;
    CefRefPtr<CefBrowser> browser_;
};

class SimpleApp : public CefApp {
public:
    SimpleApp() {}
    IMPLEMENT_REFCOUNTING(SimpleApp);
};

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    
    CefMainArgs main_args(hInstance);
    CefRefPtr<SimpleApp> app(new SimpleApp());
    
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }
    
    DX11Renderer renderer;
    if (!renderer.Initialize("Chess Game CEF - DirectX11", 1600, 900)) {
        MessageBox(nullptr, "Failed to initialize DirectX!", "Error", MB_OK);
        return -1;
    }
    
    CefSettings settings;
    settings.no_sandbox = true;
    settings.multi_threaded_message_loop = false;
    settings.windowless_rendering_enabled = true;
    
    CefInitialize(main_args, settings, app.get(), nullptr);
    
    CefWindowInfo window_info;
    window_info.SetAsWindowless(renderer.GetWindowHandle());
    
    CefBrowserSettings browser_settings;
    browser_settings.windowless_frame_rate = 60;
    
    CefRefPtr<BrowserHandler> handler(new BrowserHandler(&renderer));
    
  // SIMPLIFIED resize callback - compatible with all CEF versions
renderer.SetResizeCallback([&handler](int width, int height) {
    CefRefPtr<CefBrowser> browser = handler->GetBrowser();
    if (browser) {
        OutputDebugStringA("========== RESIZE START ==========\n");
        
        // 1. Update render handler FIRST
        handler->GetRenderHandlerRef()->SetBrowserSize(width, height);
        
        auto host = browser->GetHost();
        
        // 2. Reset any zoom (IMPORTANT)
        host->SetZoomLevel(0.0);
        
        // 3. SINGLE NotifyScreenInfoChanged - this triggers reflow
        host->NotifyScreenInfoChanged();
        
        // 4. SINGLE WasResized
        host->WasResized();
        
        // 5. JavaScript to update viewport - simple inline version
        std::string js = 
            "if (window.__CEF_RESIZE_HANDLER) { window.__CEF_RESIZE_HANDLER(); }"
            "document.documentElement.style.setProperty('--viewport-width', '" + std::to_string(width) + "px');"
            "document.documentElement.style.setProperty('--viewport-height', '" + std::to_string(height) + "px');"
            "window.dispatchEvent(new Event('resize'));"
            "console.log('[CEF] Window resized to:', " + std::to_string(width) + ", " + std::to_string(height) + ");";
        
        browser->GetMainFrame()->ExecuteJavaScript(js, browser->GetMainFrame()->GetURL(), 0);
        
        char msg[256];
        sprintf_s(msg, "[RESIZE] Notified CEF: %dx%d\n", width, height);
        OutputDebugStringA(msg);
        OutputDebugStringA("========== RESIZE END ==========\n");
    }
});
    
    CefBrowserHost::CreateBrowser(
        window_info,
        handler.get(),
        "http://127.0.0.1:3000",
        browser_settings,
        nullptr,
        nullptr
    );
    
    SetFocus(renderer.GetWindowHandle());
    Sleep(2000);
    
    bool prev_left = false;
    bool prev_right = false;
    int prev_x = 0;
    int prev_y = 0;
    bool is_dragging = false;
    bool initial_resize_done = false;
    
    while (renderer.ProcessMessages()) {
        CefDoMessageLoopWork();
        
        CefRefPtr<CefBrowser> browser = handler->GetBrowser();
        
        if (browser) {
            auto host = browser->GetHost();
            
            if (!initial_resize_done) {
                // Set initial browser size
                int w, h;
                renderer.GetWindowSize(w, h);
                handler->GetRenderHandlerRef()->SetBrowserSize(w, h);
                host->WasResized();
                initial_resize_done = true;
                OutputDebugStringA("Initial resize triggered\n");
            }
            
            static int last_width = 0;
            static int last_height = 0;
            int current_width, current_height;
            renderer.GetWindowSize(current_width, current_height);
            
            if (current_width != last_width || current_height != last_height) {
                host->WasResized();
                last_width = current_width;
                last_height = current_height;
                
                char msg[256];
                sprintf_s(msg, "Window resized to: %dx%d\n", current_width, current_height);
                OutputDebugStringA(msg);
            }
            
            host->SetFocus(true);
            
            int x, y;
            renderer.GetMousePosition(x, y);
            bool left = renderer.IsMouseButtonDown(0);
            bool right = renderer.IsMouseButtonDown(1);
            
            if (x != prev_x || y != prev_y || is_dragging) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = 0;
                
                if (left) {
                    evt.modifiers |= EVENTFLAG_LEFT_MOUSE_BUTTON;
                }
                
                host->SendMouseMoveEvent(evt, false);
                prev_x = x;
                prev_y = y;
            }
            
            if (left && !prev_left) {
                DebugPrint("LEFT CLICK DOWN");
                is_dragging = true;
                
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = EVENTFLAG_LEFT_MOUSE_BUTTON;
                host->SendMouseClickEvent(evt, MBT_LEFT, false, 1);
            }
            if (!left && prev_left) {
                DebugPrint("LEFT CLICK UP");
                is_dragging = false;
                
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = 0;
                host->SendMouseClickEvent(evt, MBT_LEFT, true, 1);
            }
            
            if (right && !prev_right) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                host->SendMouseClickEvent(evt, MBT_RIGHT, false, 1);
            }
            if (!right && prev_right) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                host->SendMouseClickEvent(evt, MBT_RIGHT, true, 1);
            }
            
            prev_left = left;
            prev_right = right;
            
            int wheel_delta = renderer.GetMouseWheelDelta();
            if (wheel_delta != 0) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = 0;
                host->SendMouseWheelEvent(evt, 0, wheel_delta);
            }
            
            KeyboardEvent key_evt;
            while (renderer.GetNextKeyEvent(key_evt)) {
                CefKeyEvent cef_key;
                cef_key.windows_key_code = (int)key_evt.wparam;
                cef_key.native_key_code = (int)key_evt.lparam;
                cef_key.modifiers = 0;
                
                if (GetKeyState(VK_SHIFT) & 0x8000)
                    cef_key.modifiers |= EVENTFLAG_SHIFT_DOWN;
                if (GetKeyState(VK_CONTROL) & 0x8000)
                    cef_key.modifiers |= EVENTFLAG_CONTROL_DOWN;
                if (GetKeyState(VK_MENU) & 0x8000)
                    cef_key.modifiers |= EVENTFLAG_ALT_DOWN;
                
                switch (key_evt.msg) {
                    case WM_KEYDOWN:
                    case WM_SYSKEYDOWN:
                        cef_key.type = KEYEVENT_RAWKEYDOWN;
                        host->SendKeyEvent(cef_key);
                        break;
                    
                    case WM_KEYUP:
                    case WM_SYSKEYUP:
                        cef_key.type = KEYEVENT_KEYUP;
                        host->SendKeyEvent(cef_key);
                        break;
                    
                    case WM_CHAR:
                    case WM_SYSCHAR:
                        cef_key.type = KEYEVENT_CHAR;
                        cef_key.windows_key_code = (int)key_evt.wparam;
                        cef_key.native_key_code = (int)key_evt.wparam;
                        host->SendKeyEvent(cef_key);
                        break;
                }
            }
        } 
        
        renderer.Render();
        Sleep(1);
    } 
    
    CefShutdown();
    return 0;
}