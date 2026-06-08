#include "game/PlayerController.h"

#include "game/EchoSystem.h"
#include "game/LevelManager.h"
#include "platform/Input.h"
#include "platform/Renderer.h"

#include <algorithm>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#endif

namespace echoes::game {

namespace {

constexpr float kMoveSpeed = 6.0f;
constexpr float kSprintMultiplier = 1.6f;
constexpr float kInteractRadius = 2.5f;

float LengthFlat(const math::Vec3& v) {
    return std::sqrt(v.x * v.x + v.z * v.z);
}

math::Vec3 Lerp(const math::Vec3& a, const math::Vec3& b, float t) {
    return a + (b - a) * t;
}

}  // namespace

PlayerController::PlayerController(EchoSystem& echoSystem)
    : echoSystem_(echoSystem) {}

void PlayerController::HandleInput(LevelManager& level) {
    auto& input = platform::Input::Instance();
    wantsFire_ = false;

#ifdef _WIN32
    if (input.IsKeyPressed('E')) {
        if (level.GetHydraulicPuzzle().TryToggleGate(GetPosition(), kInteractRadius)) {
            return;
        }
    }

    if (input.IsKeyPressed('Q')) {
        TryTakeCover(level);
    }

    if (input.IsKeyPressed('F')) {
        TryGrapple(level);
    }

    if (input.IsKeyPressed(VK_SPACE)) {
        if (state_ == PlayerState::Ground) {
            if (TryStartClimb()) {
                state_ = PlayerState::Climbing;
            } else {
                Jump();
            }
        }
    }

    if (input.IsKeyPressed('1')) {
        echoSystem_.ActivatePower(EchoPower::TemporalSlow);
    }
    if (input.IsKeyReleased('1')) {
        echoSystem_.DeactivatePower(EchoPower::TemporalSlow);
    }
    if (input.IsKeyPressed('2')) {
        echoSystem_.ActivatePower(EchoPower::SpectralVision);
    }
    if (input.IsKeyPressed('3')) {
        echoSystem_.ActivatePower(EchoPower::MemoryInvocation);
    }

    if (input.IsKeyDown(VK_LBUTTON)) {
        wantsFire_ = true;
    }

    if (input.IsKeyDown(VK_RIGHT)) {
        yaw_ += 0.03f;
    }
    if (input.IsKeyDown(VK_LEFT)) {
        yaw_ -= 0.03f;
    }
#endif
}

void PlayerController::Update(float deltaTime, LevelManager& level) {
    HandleInput(level);

    const float timeScale = echoSystem_.IsPowerActive(EchoPower::TemporalSlow) ? 0.35f : 1.0f;
    const float scaledDelta = deltaTime * timeScale;

    switch (state_) {
        case PlayerState::Climbing:
            UpdateClimbing(scaledDelta);
            break;
        case PlayerState::Grappling:
            UpdateGrappling(scaledDelta);
            break;
        case PlayerState::Swimming:
            UpdateSwimming(scaledDelta, level);
            break;
        case PlayerState::InCover:
            break;
        case PlayerState::Ground:
        default:
            UpdateMovement(scaledDelta, level);
            break;
    }

    if (state_ == PlayerState::InCover && !platform::Input::Instance().IsKeyDown('Q')) {
        state_ = PlayerState::Ground;
    }
}

void PlayerController::UpdateMovement(float deltaTime, const LevelManager& level) {
    auto& input = platform::Input::Instance();
    float speed = kMoveSpeed;

#ifdef _WIN32
    if (input.IsKeyDown(VK_SHIFT)) {
        speed *= kSprintMultiplier;
    }

    float forward = 0.0f;
    float right = 0.0f;
    if (input.IsKeyDown('Z') || input.IsKeyDown('W')) forward += 1.0f;
    if (input.IsKeyDown('S')) forward -= 1.0f;
    if (input.IsKeyDown('D')) right += 1.0f;
    if (input.IsKeyDown('A')) right -= 1.0f;

    const float sinY = std::sin(yaw_);
    const float cosY = std::cos(yaw_);
    positionX_ += (forward * sinY + right * cosY) * speed * deltaTime;
    positionZ_ += (forward * cosY - right * sinY) * speed * deltaTime;
#endif

    const float waterLevel = level.GetWaterLevel();
    if (positionY_ < waterLevel + 0.5f && waterLevel > 0.8f) {
        state_ = PlayerState::Swimming;
    }

    if (!grounded_) {
        constexpr float gravity = -18.0f;
        velocityY_ += gravity * deltaTime;
        positionY_ += velocityY_ * deltaTime;

        if (positionY_ <= 1.0f) {
            positionY_ = 1.0f;
            velocityY_ = 0.0f;
            grounded_ = true;
        }
    }
}

void PlayerController::UpdateSwimming(float deltaTime, const LevelManager& level) {
    auto& input = platform::Input::Instance();
    const float waterLevel = level.GetWaterLevel();

#ifdef _WIN32
    if (input.IsKeyDown('Z') || input.IsKeyDown('W')) positionZ_ -= 4.0f * deltaTime;
    if (input.IsKeyDown('S')) positionZ_ += 4.0f * deltaTime;
    if (input.IsKeyDown('D')) positionX_ += 4.0f * deltaTime;
    if (input.IsKeyDown('A')) positionX_ -= 4.0f * deltaTime;
    if (input.IsKeyDown(VK_SPACE)) positionY_ += 2.5f * deltaTime;
#endif

    positionY_ = std::max(waterLevel - 0.3f, positionY_ - 1.0f * deltaTime);

    if (positionY_ > waterLevel + 0.8f) {
        state_ = PlayerState::Ground;
        grounded_ = false;
    }
}

void PlayerController::UpdateClimbing(float deltaTime) {
    climbProgress_ += deltaTime * 0.8f;
    if (climbProgress_ >= 1.0f) {
        climbProgress_ = 1.0f;
        state_ = PlayerState::Ground;
        grounded_ = true;
    }

    const math::Vec3 pos = Lerp(climbStart_, climbEnd_, climbProgress_);
    positionX_ = pos.x;
    positionY_ = pos.y;
    positionZ_ = pos.z;
}

void PlayerController::UpdateGrappling(float deltaTime) {
    grappleProgress_ += deltaTime * 1.5f;
    const math::Vec3 start{positionX_, positionY_, positionZ_};
    const math::Vec3 pos = Lerp(start, grappleTarget_, std::min(1.0f, grappleProgress_));

    positionX_ = pos.x;
    positionY_ = pos.y;
    positionZ_ = pos.z;

    if (grappleProgress_ >= 1.0f) {
        state_ = PlayerState::Ground;
        grounded_ = true;
        grappleProgress_ = 0.0f;
    }
}

bool PlayerController::TryStartClimb() {
    const math::Vec3 climbWall{-6.0f, 3.0f, -2.0f};
    const math::Vec3 playerPos = GetPosition();
    const float dist = (playerPos - climbWall).Length();

    if (dist < 3.0f) {
        climbStart_ = playerPos;
        climbEnd_ = {climbWall.x + 1.0f, 5.5f, climbWall.z};
        climbProgress_ = 0.0f;
        return true;
    }
    return false;
}

bool PlayerController::TryGrapple(const LevelManager& level) {
    for (const auto& point : level.GetGrapplePoints()) {
        const float dist = (GetPosition() - point).Length();
        if (dist < 18.0f && dist > 3.0f) {
            grappleTarget_ = point;
            grappleProgress_ = 0.0f;
            state_ = PlayerState::Grappling;
            grounded_ = false;
            return true;
        }
    }
    return false;
}

bool PlayerController::TryTakeCover(const LevelManager& level) {
    for (const auto& cover : level.GetCoverSpots()) {
        const math::Vec3 offset = GetPosition() - cover.position;
        if (LengthFlat(offset) < 2.5f) {
            positionX_ = cover.position.x;
            positionY_ = cover.position.y;
            positionZ_ = cover.position.z;
            state_ = PlayerState::InCover;
            return true;
        }
    }
    return false;
}

void PlayerController::Jump() {
    if (grounded_ && state_ == PlayerState::Ground) {
        velocityY_ = 6.5f;
        grounded_ = false;
    }
}

void PlayerController::TakeDamage(float amount) {
    if (state_ == PlayerState::InCover) {
        amount *= 0.25f;
    }
    health_ = std::max(0.0f, health_ - amount);
}

void PlayerController::Render(platform::Renderer& renderer) {
    platform::RenderColor color{0.2f, 0.55f, 0.85f, 1.0f};
    if (state_ == PlayerState::InCover) {
        color = {0.15f, 0.4f, 0.65f, 1.0f};
    }
    if (echoSystem_.IsPowerActive(EchoPower::SpectralVision)) {
        color = {0.5f, 0.8f, 1.0f, 0.9f};
    }

    renderer.DrawBox(GetPosition(), {0.8f, 1.8f, 0.8f}, color);

    if (state_ == PlayerState::Grappling) {
        renderer.DrawBox(
            Lerp(GetPosition(), grappleTarget_, 0.5f),
            {0.05f, 0.05f, (grappleTarget_ - GetPosition()).Length()},
            {0.9f, 0.9f, 0.2f, 1.0f});
    }
}

}  // namespace echoes::game
