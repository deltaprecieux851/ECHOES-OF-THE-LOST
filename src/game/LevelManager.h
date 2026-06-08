#pragma once

#include "game/HydraulicPuzzle.h"
#include "game/PlayerController.h"
#include "game/CombatSystem.h"
#include "math/Vec3.h"

#include <memory>
#include <string>
#include <vector>

namespace echoes::platform {
class Renderer;
}

namespace echoes::game {

class CombatSystem;

struct LevelObjective {
    std::string id;
    std::string description;
    bool completed{false};
};

class LevelManager {
public:
    LevelManager();
    ~LevelManager();

    bool LoadLevel(const std::string& levelId);
    void UnloadLevel();
    void Update(float deltaTime);
    void Render(platform::Renderer& renderer);

    CombatSystem& GetCombatSystem() { return *combatSystem_; }
    HydraulicPuzzle& GetHydraulicPuzzle() { return hydraulicPuzzle_; }

    const std::string& GetCurrentLevelId() const { return currentLevelId_; }
    const std::vector<LevelObjective>& GetObjectives() const { return objectives_; }
    const std::vector<CoverSpot>& GetCoverSpots() const { return coverSpots_; }
    const std::vector<math::Vec3>& GetGrapplePoints() const { return grapplePoints_; }

    float GetWaterLevel() const { return waterLevel_; }
    bool IsLevelComplete() const;
    void CheckObjectives(const PlayerController& player);

private:
    bool LoadLevelJson(const std::string& path);
    void SetupMohenjoDaroPrototype();
    void CompleteObjective(const std::string& id);

    std::string currentLevelId_;
    std::string levelTitle_;
    std::vector<LevelObjective> objectives_;
    std::vector<CoverSpot> coverSpots_;
    std::vector<math::Vec3> grapplePoints_;

    float waterLevel_{2.5f};
    bool crystalRetrieved_{false};

    HydraulicPuzzle hydraulicPuzzle_;
    std::unique_ptr<CombatSystem> combatSystem_;
};

}  // namespace echoes::game
