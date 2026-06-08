#pragma once

#include "game/Enemy.h"

#include <memory>
#include <vector>

namespace echoes::platform {
class Renderer;
}

namespace echoes::game {

class PlayerController;

class CombatSystem {
public:
    void InitializeCartelPatrol();
    void Update(float deltaTime, PlayerController& player);
    void Render(platform::Renderer& renderer);

    std::vector<std::unique_ptr<Enemy>>& GetEnemies() { return enemies_; }
    bool IsPatrolCleared() const;
    int GetAliveEnemyCount() const;

private:
    std::vector<std::unique_ptr<Enemy>> enemies_;
};

}  // namespace echoes::game
