#include "game/HydraulicPuzzle.h"

#include "core/Logger.h"
#include "platform/Renderer.h"

#include <algorithm>
#include <cmath>

namespace echoes::game {

namespace {

float Distance(const math::Vec3& a, const math::Vec3& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

}  // namespace

void HydraulicPuzzle::Initialize() {
    gates_ = {{
        {"north_gate", {-8.0f, 1.0f, -4.0f}, false},
        {"east_gate",  { 8.0f, 1.0f,  2.0f}, false},
        {"south_gate", { 0.0f, 1.0f, 10.0f}, false},
    }};
    animatedWaterLevel_ = 2.5f;
    targetWaterLevel_ = 2.5f;
    solved_ = false;
}

void HydraulicPuzzle::Update(float deltaTime) {
    if (IsSolved()) {
        targetWaterLevel_ = 0.4f;
        if (!solved_) {
            solved_ = true;
            core::Logger::Log(core::LogLevel::Info, "Hydraulic puzzle solved — temple channels restored.");
        }
    }

    const float delta = targetWaterLevel_ - animatedWaterLevel_;
    animatedWaterLevel_ += delta * std::min(1.0f, deltaTime * 1.5f);
}

bool HydraulicPuzzle::TryToggleGate(const math::Vec3& playerPos, float interactRadius) {
    for (auto& gate : gates_) {
        if (Distance(playerPos, gate.leverPosition) <= interactRadius) {
            gate.open = !gate.open;
            core::Logger::Log(core::LogLevel::Info, "Gate toggled: " + gate.id);
            return true;
        }
    }
    return false;
}

bool HydraulicPuzzle::IsSolved() const {
    for (const auto& gate : gates_) {
        if (!gate.open) {
            return false;
        }
    }
    return true;
}

float HydraulicPuzzle::GetTargetWaterLevel() const {
    return animatedWaterLevel_;
}

void HydraulicPuzzle::Render(platform::Renderer& renderer) {
    for (const auto& gate : gates_) {
        const platform::RenderColor color = gate.open
            ? platform::RenderColor{0.2f, 0.8f, 0.3f, 1.0f}
            : platform::RenderColor{0.8f, 0.3f, 0.2f, 1.0f};
        renderer.DrawBox(gate.leverPosition, {0.8f, 1.6f, 0.8f}, color);
    }
}

}  // namespace echoes::game
