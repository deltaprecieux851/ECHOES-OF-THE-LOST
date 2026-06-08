#pragma once

namespace echoes::platform {

class Input {
public:
    static Input& Instance();

    void Update();
    bool IsKeyDown(int virtualKey) const;
    bool IsKeyPressed(int virtualKey) const;
    bool IsKeyReleased(int virtualKey) const;

private:
    Input() = default;
    bool current_[256]{};
    bool previous_[256]{};
};

}  // namespace echoes::platform
