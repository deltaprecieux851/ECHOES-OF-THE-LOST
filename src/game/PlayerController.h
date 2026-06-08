#pragma once

#include "math/Vec3.h"

namespace echoes::platform {
class Renderer;
}

namespace echoes::game {

class EchoSystem;
class HydraulicPuzzle;

enum class PlayerState {
    Ground,
    Climbing,
    Grappling,
    InCover,
    Swimming
};

struct CoverSpot {
    math::Vec3 position;
    math::Vec3 direction;
};

class PlayerController {
public:
    explicit PlayerController(EchoSystem& echoSystem);

    void Update(float deltaTime, class LevelManager& level);
    void Render(platform::Renderer& renderer);
    void HandleInput(class LevelManager& level);

    math::Vec3 GetPosition() const { return {positionX_, positionY_, positionZ_}; }
    float GetYaw() const { return yaw_; }

    bool IsInCover() const { return state_ == PlayerState::InCover; }
    bool WantsToFire() const { return wantsFire_; }
    float GetHealth() const { return health_; }
    float GetMaxHealth() const { return maxHealth_; }

    void TakeDamage(float amount);

private:
    bool TryStartClimb();
    bool TryGrapple(const LevelManager& level);
    bool TryTakeCover(const LevelManager& level);
    void UpdateMovement(float deltaTime, const LevelManager& level);
    void UpdateClimbing(float deltaTime);
    void UpdateGrappling(float deltaTime);
    void UpdateSwimming(float deltaTime, const LevelManager& level);
    void Jump();

    EchoSystem& echoSystem_;
    PlayerState state_{PlayerState::Ground};

    float positionX_{0.0f};
    float positionY_{1.0f};
    float positionZ_{8.0f};
    float velocityY_{0.0f};
    float yaw_{0.0f};

    float health_{100.0f};
    float maxHealth_{100.0f};
    bool grounded_{true};
    bool wantsFire_{false};

    math::Vec3 grappleTarget_{};
    float grappleProgress_{0.0f};
    math::Vec3 climbStart_{};
    math::Vec3 climbEnd_{};
    float climbProgress_{0.0f};
};

}  // namespace echoes::game
