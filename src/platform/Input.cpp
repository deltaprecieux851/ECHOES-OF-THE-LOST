#include "platform/Input.h"

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace echoes::platform {

Input& Input::Instance() {
    static Input instance;
    return instance;
}

void Input::Update() {
    std::memcpy(previous_, current_, sizeof(current_));

#ifdef _WIN32
    for (int vk = 0; vk < 256; ++vk) {
        current_[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
    }
#endif
}

bool Input::IsKeyDown(int virtualKey) const {
    if (virtualKey < 0 || virtualKey >= 256) {
        return false;
    }
    return current_[virtualKey];
}

bool Input::IsKeyPressed(int virtualKey) const {
    if (virtualKey < 0 || virtualKey >= 256) {
        return false;
    }
    return current_[virtualKey] && !previous_[virtualKey];
}

bool Input::IsKeyReleased(int virtualKey) const {
    if (virtualKey < 0 || virtualKey >= 256) {
        return false;
    }
    return !current_[virtualKey] && previous_[virtualKey];
}

}  // namespace echoes::platform
