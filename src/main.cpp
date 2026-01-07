// main.cpp - CEF + DirectX 11 with FULL Input (Mouse, Scroll, Keyboard) - FIXED

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "dx11_renderer.h"
#include <windows.h>

void DebugPrint(const char* msg) {
    char buffer[256];
    sprintf_s(buffer, "[CEF DEBUG] %s\n", msg);
    OutputDebugStringA(buffer);
}

class RenderHandler;

// NEW: Off-screen render handler
class RenderHandler : public CefRenderHandler {
public:
    RenderHandler(DX11Renderer* renderer) : renderer_(renderer) {}
    
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override {
        rect.x = 0;
        rect.y = 0;
        rect.width = 1280;
        rect.height = 720;
    }
    
    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
                        PaintElementType type,
                        const RectList& dirtyRects,
                        const void* buffer,
                        int width,
                        int height) override {
        if (renderer_) {
            renderer_->UpdateTexture(buffer, width, height);
        }
    }
    
    IMPLEMENT_REFCOUNTING(RenderHandler);
    
private:
    DX11Renderer* renderer_;
};

// NEW: Browser handler
class BrowserHandler : public CefClient, public CefLifeSpanHandler {
public:
    BrowserHandler(DX11Renderer* renderer) 
        : render_handler_(new RenderHandler(renderer)), browser_(nullptr) {}
    
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
        return render_handler_;
    }
    
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }
    
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override {
        browser_ = browser;
    }
    
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        browser_ = nullptr;
        CefQuitMessageLoop();
    }
    
    CefRefPtr<CefBrowser> GetBrowser() { return browser_; }
    
    IMPLEMENT_REFCOUNTING(BrowserHandler);
    
private:
    CefRefPtr<RenderHandler> render_handler_;
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
    if (!renderer.Initialize("Chess Game CEF - DirectX11", 1280, 720)) {
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
    
    CefBrowserHost::CreateBrowser(
        window_info,
        handler.get(),
        // "http://127.0.0.1:3000",
		"http://127.0.0.1:3000",
        browser_settings,
        nullptr,
        nullptr
    );
    
    // NEW: Set focus to the exe window
    SetFocus(renderer.GetWindowHandle());
	
	Sleep(1000);
    
    // NEW: Track mouse input state
    bool prev_left = false;
    bool prev_right = false;
    int prev_x = 0;
    int prev_y = 0;
    
    // NEW: Main loop with FULL input (FIXED)
    while (renderer.ProcessMessages()) {
        CefDoMessageLoopWork();
        
        CefRefPtr<CefBrowser> browser = handler->GetBrowser();
        
        if (browser) {
            auto host = browser->GetHost();
            
            // NEW: Ensure CEF has focus for input
            host->SetFocus(true);
            
            // NEW: Get mouse position
            int x, y;
            renderer.GetMousePosition(x, y);
            bool left = renderer.IsMouseButtonDown(0);
            bool right = renderer.IsMouseButtonDown(1);
            
            // NEW: Mouse move
            if (x != prev_x || y != prev_y) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = 0;
                host->SendMouseMoveEvent(evt, false);
                prev_x = x;
                prev_y = y;
            }
            
            // NEW: Mouse clicks (with debug)
if (left && !prev_left) {
    DebugPrint("LEFT CLICK DOWN");  // NEW
    CefMouseEvent evt;
    evt.x = x;
    evt.y = y;
    evt.modifiers = 0;
    host->SendMouseClickEvent(evt, MBT_LEFT, false, 1);
}
if (!left && prev_left) {
    DebugPrint("LEFT CLICK UP");  // NEW
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
            
            // NEW: Mouse wheel (scroll)
            int wheel_delta = renderer.GetMouseWheelDelta();
            if (wheel_delta != 0) {
                CefMouseEvent evt;
                evt.x = x;
                evt.y = y;
                evt.modifiers = 0;
                host->SendMouseWheelEvent(evt, 0, wheel_delta);
            }
            
            // NEW: Keyboard input
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