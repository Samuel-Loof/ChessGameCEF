// main.cpp - Entry point for our CEF + DirectX application
// This is the C++ code that starts everything

#include "include/cef_app.h"
#include "include/cef_client.h"
#include <windows.h>

// NEW: Handler for CEF browser events
// CefClient is the base class for handling browser callbacks
// CefLifeSpanHandler handles when the browser opens/closes
class SimpleHandler : public CefClient, public CefLifeSpanHandler {
public:
    SimpleHandler() {}
    
    // NEW: CEF calls this to get our LifeSpanHandler
    // We return 'this' because we implement it directly in this class
    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override {
        return this;
    }
    
    // NEW: Called when the browser window closes
    // We quit the message loop so the program can shut down cleanly
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override {
        CefQuitMessageLoop();
    }
    
    // NEW: CEF uses reference counting for memory management
    // This macro implements AddRef/Release automatically
    IMPLEMENT_REFCOUNTING(SimpleHandler);
};

// NEW: Application-level CEF handler
// CefApp lets us customize CEF's behavior
class SimpleApp : public CefApp {
public:
    SimpleApp() {}
    IMPLEMENT_REFCOUNTING(SimpleApp);
};

// NEW: Windows entry point
// wWinMain is used for Windows GUI apps (not console)
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    
    // NEW: CEF needs the Windows instance handle and command line args
    CefMainArgs main_args(hInstance);
    CefRefPtr<SimpleApp> app(new SimpleApp());
    
    // NEW: CEF is multi-process (just like Chrome)
    // Sub-processes (renderer, GPU, etc) run through this function
    // If it's a sub-process, it returns directly here
    int exit_code = CefExecuteProcess(main_args, app.get(), nullptr);
    if (exit_code >= 0) {
        return exit_code; // Sub-process, exit
    }
    
    // NEW: Settings for main browser process
    CefSettings settings;
    settings.no_sandbox = true; // Easier for development, but less secure
    settings.multi_threaded_message_loop = false; // We run our own message loop
    
    // NEW: Initialize CEF
    // This must be done before we create any browser
    CefInitialize(main_args, settings, app.get(), nullptr);
    
    // NEW: Window info for browser
    // SetAsPopup creates a regular window (we'll change to off-screen later)
    CefWindowInfo window_info;
    window_info.SetAsPopup(nullptr, "Chess Game CEF");
    
    // NEW: Browser-specific settings
    CefBrowserSettings browser_settings;
    
    // NEW: Create browser handler instance
    CefRefPtr<SimpleHandler> handler(new SimpleHandler());
    
    // NEW: Create the actual browser
    // We load Google first to test that everything works
    CefBrowserHost::CreateBrowser(
        window_info,           // Window config
        handler.get(),         // Event handler
        "https://www.google.com", // Initial URL
        browser_settings,      // Browser settings
        nullptr,              // Extra info
        nullptr               // Request context
    );
    
    // NEW: Message loop - keeps the program running
    // Processes Windows events and CEF callbacks
    // Blocks until CefQuitMessageLoop() is called
    CefRunMessageLoop();
    
    // NEW: Shutdown CEF when message loop exits
    CefShutdown();
    
    return 0;
}