#include "game/CombatSystem.h"

#include "core/Logger.h"
#include "game/PlayerController.h"
#include "platform/Renderer.h"

namespace echoes::game {

void CombatSystem::InitializeCartelPatrol() {
    enemies_.clear();

    enemies_.push_back(std::make_unique<Enemy>("cartel_01", std::vector<PatrolPoint>{
        {{12.0f, 0.0f, -6.0f}},
        {{12.0f, 0.0f, 6.0f}},
        {{6.0f, 0.0f, 10.0f}},
    }, 80.0f));

    enemies_.push_back(std::make_unique<Enemy>("cartel_02", std::vector<PatrolPoint>{
        {{-10.0f, 0.0f, 8.0f}},
        {{-4.0f, 0.0f, 12.0f}},
        {{2.0f, 0.0f, 8.0f}},
    }, 80.0f));

    enemies_.push_back(std::make_unique<Enemy>("cartel_elite", std::vector<PatrolPoint>{
        {{0.0f, 0.0f, -10.0f}},
        {{4.0f, 0.0f, -6.0f}},
        {{-4.0f, 0.0f, -6.0f}},
    }, 120.0f));

    core::Logger::Log(core::LogLevel::Info, "Cartel patrol deployed in temple courtyard.");
}

void CombatSystem::Update(float deltaTime, PlayerController& player) {
    const math::Vec3 playerPos = player.GetPosition();
    const bool inCover = player.IsInCover();

    for (auto& enemy : enemies_) {
        if (!enemy->IsAlive()) {
            continue;
        }

        enemy->Update(deltaTime, playerPos, inCover);

        const float dist = (enemy->GetPosition() - playerPos).Length();
        if (enemy->CanShootPlayer(dist) && dist < 14.0f && !inCover) {
            enemy->Shoot();
            player.TakeDamage(12.0f);
        }
    }

    if (player.WantsToFire()) {
        for (auto& enemy : enemies_) {
            if (!enemy->IsAlive()) {
                continue;
            }
            const float dist = (enemy->GetPosition() - playerPos).Length();
            if (dist < 22.0f) {
                enemy->TakeDamage(28.0f);
                break;
            }
        }
    }
}

void CombatSystem::Render(platform::Renderer& renderer) {
    for (auto& enemy : enemies_) {
        enemy->Render(renderer);
    }
}

bool CombatSystem::IsPatrolCleared() const {
    for (const auto& enemy : enemies_) {
        if (enemy->IsAlive()) {
            return false;
        }
    }
    return !enemies_.empty();
}

int CombatSystem::GetAliveEnemyCount() const {
    int count = 0;
    for (const auto& enemy : enemies_) {
        if (enemy->IsAlive()) {
            ++count;
        }
    }
    return count;
}

}  // namespace echoes::game
