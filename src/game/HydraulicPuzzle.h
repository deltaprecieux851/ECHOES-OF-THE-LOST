#pragma once

#include "math/Vec3.h"

#include <array>
#include <string>

namespace echoes::platform {
class Renderer;
}

namespace echoes::game {

struct HydraulicGate {
    std::string id;
    math::Vec3 leverPosition;
    bool open{false};
};

class HydraulicPuzzle {
public:
    void Initialize();
    void Update(float deltaTime);
    void Render(platform::Renderer& renderer);

    bool TryToggleGate(const math::Vec3& playerPos, float interactRadius);
    bool IsSolved() const;
    float GetTargetWaterLevel() const;

    const std::array<HydraulicGate, 3>& GetGates() const { return gates_; }

private:
    std::array<HydraulicGate, 3> gates_{};
    float animatedWaterLevel_{2.5f};
    float targetWaterLevel_{2.5f};
    bool solved_{false};
};

}  // namespace echoes::game
