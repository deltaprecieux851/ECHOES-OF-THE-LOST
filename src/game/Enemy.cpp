#include "game/Enemy.h"

#include "platform/Renderer.h"

#include <algorithm>
#include <cmath>

namespace echoes::game {

namespace {

float DistanceFlat(const math::Vec3& a, const math::Vec3& b) {
    const float dx = a.x - b.x;
    const float dz = a.z - b.z;
    return std::sqrt(dx * dx + dz * dz);
}

}  // namespace

Enemy::Enemy(std::string id, const std::vector<PatrolPoint>& patrolRoute, float health)
    : id_(std::move(id)), patrolRoute_(patrolRoute), health_(health) {
    if (!patrolRoute_.empty()) {
        position_ = patrolRoute_.front().position;
    }
}

void Enemy::Update(float deltaTime, const math::Vec3& playerPos, bool playerInCover) {
    if (!IsAlive()) {
        return;
    }

    attackCooldown_ = std::max(0.0f, attackCooldown_ - deltaTime);

    const float dist = DistanceFlat(position_, playerPos);

    if (dist < 18.0f && !playerInCover) {
        state_ = EnemyState::Chase;
        const math::Vec3 dir = (playerPos - position_).Normalized();
        position_ = position_ + dir * (3.5f * deltaTime);

        if (dist < 10.0f) {
            state_ = EnemyState::Attack;
        }
    } else {
        state_ = EnemyState::Patrol;
        AdvancePatrol(deltaTime);
    }
}

void Enemy::AdvancePatrol(float deltaTime) {
    if (patrolRoute_.empty()) {
        return;
    }

    const math::Vec3 target = patrolRoute_[patrolIndex_].position;
    const math::Vec3 dir = (target - position_).Normalized();
    position_ = position_ + dir * (2.0f * deltaTime);

    if (DistanceFlat(position_, target) < 0.5f) {
        patrolIndex_ = (patrolIndex_ + 1) % patrolRoute_.size();
    }
}

void Enemy::TakeDamage(float amount) {
    if (!IsAlive()) {
        return;
    }

    health_ -= amount;
    if (health_ <= 0.0f) {
        health_ = 0.0f;
        state_ = EnemyState::Dead;
    }
}

bool Enemy::CanShootPlayer(float maxRange) const {
    return IsAlive() && state_ == EnemyState::Attack && attackCooldown_ <= 0.0f && maxRange > 0.0f;
}

void Enemy::Shoot() {
    attackCooldown_ = 1.2f;
}

void Enemy::Render(platform::Renderer& renderer) {
    if (!IsAlive()) {
        return;
    }

    platform::RenderColor color{0.75f, 0.15f, 0.15f, 1.0f};
    if (state_ == EnemyState::Chase || state_ == EnemyState::Attack) {
        color = {0.95f, 0.25f, 0.1f, 1.0f};
    }

    renderer.DrawBox(position_, {0.9f, 1.8f, 0.9f}, color);
}

}  // namespace echoes::game
