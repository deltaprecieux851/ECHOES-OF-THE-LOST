#include "platform/Window.h"

#include "core/Logger.h"

namespace echoes::platform {

#ifdef _WIN32

namespace {
    Window* g_windowInstance = nullptr;
}

Window::Window(std::string title, int width, int height)
    : title_(std::move(title)), width_(width), height_(height) {}

Window::~Window() {
    Destroy();
}

bool Window::Create() {
    g_windowInstance = this;

    const wchar_t* className = L"EchoesOfTheLostWindowClass";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc)) {
        core::Logger::Log(core::LogLevel::Error, "Failed to register window class.");
        return false;
    }

    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    const int posX = (screenWidth - width_) / 2;
    const int posY = (screenHeight - height_) / 2;

    std::wstring wideTitle(title_.begin(), title_.end());

    hwnd_ = CreateWindowExW(
        0,
        className,
        wideTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        posX, posY,
        width_, height_,
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    if (!hwnd_) {
        core::Logger::Log(core::LogLevel::Error, "Failed to create application window.");
        return false;
    }

    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);
    return true;
}

void Window::Destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    g_windowInstance = nullptr;
}

bool Window::ProcessEvents() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            shouldClose_ = true;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return !shouldClose_;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        if (g_windowInstance) {
            g_windowInstance->shouldClose_ = true;
        }
        return 0;
    }

    if (msg == WM_CLOSE) {
        if (g_windowInstance) {
            g_windowInstance->shouldClose_ = true;
        }
        DestroyWindow(hwnd);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

#else

Window::Window(std::string title, int width, int height)
    : title_(std::move(title)), width_(width), height_(height) {}

Window::~Window() = default;
bool Window::Create() { return false; }
void Window::Destroy() {}
bool Window::ProcessEvents() { return false; }

#endif

}  // namespace echoes::platform
