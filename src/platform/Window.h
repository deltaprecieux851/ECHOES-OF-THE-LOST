#pragma once

#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace echoes::platform {

class Window {
public:
    Window(std::string title, int width, int height);
    ~Window();

    bool Create();
    void Destroy();
    bool ProcessEvents();

#ifdef _WIN32
    HWND GetHandle() const { return hwnd_; }
#endif
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }

private:
#ifdef _WIN32
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    HWND hwnd_{nullptr};
#endif

    std::string title_;
    int width_;
    int height_;
    bool shouldClose_{false};
};

}  // namespace echoes::platform
