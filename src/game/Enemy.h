#pragma once

#include "math/Vec3.h"

#include <string>
#include <vector>

namespace echoes::platform {
class Renderer;
}

namespace echoes::game {

enum class EnemyState {
    Patrol,
    Chase,
    Attack,
    Dead
};

struct PatrolPoint {
    math::Vec3 position;
};

class Enemy {
public:
    Enemy(std::string id, const std::vector<PatrolPoint>& patrolRoute, float health);

    void Update(float deltaTime, const math::Vec3& playerPos, bool playerInCover);
    void Render(platform::Renderer& renderer);
    void TakeDamage(float amount);

    bool IsAlive() const { return state_ != EnemyState::Dead; }
    bool CanShootPlayer(float maxRange) const;
    void Shoot();
    float GetAttackCooldown() const { return attackCooldown_; }

    const math::Vec3& GetPosition() const { return position_; }
    const std::string& GetId() const { return id_; }
    EnemyState GetState() const { return state_; }

private:
    void AdvancePatrol(float deltaTime);

    std::string id_;
    std::vector<PatrolPoint> patrolRoute_;
    std::size_t patrolIndex_{0};
    math::Vec3 position_;
    float health_{100.0f};
    float attackCooldown_{0.0f};
    EnemyState state_{EnemyState::Patrol};
};

}  // namespace echoes::game
