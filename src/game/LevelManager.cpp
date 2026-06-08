#include "game/LevelManager.h"

#include "core/Logger.h"
#include "game/CombatSystem.h"
#include "game/PlayerController.h"
#include "platform/Renderer.h"

#ifdef EOL_HAS_JSON
#include <nlohmann/json.hpp>
#endif

#include <filesystem>
#include <fstream>

namespace echoes::game {

LevelManager::LevelManager()
    : combatSystem_(std::make_unique<CombatSystem>()) {}

LevelManager::~LevelManager() {
    UnloadLevel();
}

bool LevelManager::LoadLevel(const std::string& levelId) {
    UnloadLevel();
    currentLevelId_ = levelId;

    std::filesystem::path jsonPath = std::filesystem::path("assets") / "levels" / "mohenjo_daro" / "les_larmes_de_la_terre.json";

    if (levelId.find("mohenjo") != std::string::npos ||
        levelId.find("larmes") != std::string::npos ||
        LoadLevelJson(jsonPath.string())) {
        SetupMohenjoDaroPrototype();
        return true;
    }

    core::Logger::Log(core::LogLevel::Warning, "Unknown level — sandbox mode.");
    return false;
}

bool LevelManager::LoadLevelJson(const std::string& path) {
#ifdef EOL_HAS_JSON
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json data;
        file >> data;

        levelTitle_ = data.value("title", "Unknown Level");
        objectives_.clear();

        if (data.contains("objectives")) {
            for (const auto& obj : data["objectives"]) {
                objectives_.push_back({
                    obj.value("id", ""),
                    obj.value("description", ""),
                    false
                });
            }
        }

        core::Logger::Log(core::LogLevel::Info, "Loaded level JSON: " + levelTitle_);
        return true;
    } catch (...) {
        core::Logger::Log(core::LogLevel::Warning, "Failed to parse level JSON.");
        return false;
    }
#else
    (void)path;
    return false;
#endif
}

void LevelManager::SetupMohenjoDaroPrototype() {
    if (objectives_.empty()) {
        objectives_ = {
            {"hydraulic_puzzle", "Restore the flooded temple channels", false},
            {"cartel_patrol",    "Survive the first Cartel patrol", false},
            {"mirror_throne",    "Align the throne room mirrors", false},
            {"crystal_retrieve", "Recover the First Echo Crystal", false},
        };
    }

    coverSpots_ = {
        {{ -3.0f, 1.0f, 6.0f},  {0.0f, 0.0f, 1.0f}},
        {{  5.0f, 1.0f, -4.0f},  {0.0f, 0.0f, -1.0f}},
        {{  0.0f, 1.0f, 14.0f},  {0.0f, 0.0f, 1.0f}},
    };

    grapplePoints_ = {
        {0.0f, 6.0f, 0.0f},
        {-6.0f, 5.0f, 8.0f},
        {8.0f, 4.5f, -6.0f},
    };

    hydraulicPuzzle_.Initialize();
    combatSystem_->InitializeCartelPatrol();
    waterLevel_ = 2.5f;
    crystalRetrieved_ = false;

    core::Logger::Log(core::LogLevel::Info, "Prototype ready: Les Larmes de la Terre");
}

void LevelManager::UnloadLevel() {
    objectives_.clear();
    coverSpots_.clear();
    grapplePoints_.clear();
    currentLevelId_.clear();
    levelTitle_.clear();
    waterLevel_ = 0.0f;
    crystalRetrieved_ = false;
}

void LevelManager::Update(float deltaTime) {
    if (currentLevelId_.empty()) {
        return;
    }

    hydraulicPuzzle_.Update(deltaTime);
    waterLevel_ = hydraulicPuzzle_.GetTargetWaterLevel();
}

void LevelManager::Render(platform::Renderer& renderer) {
    if (currentLevelId_.empty()) {
        return;
    }

    renderer.DrawTempleScene();
    renderer.DrawWaterPlane(waterLevel_, 36.0f);
    hydraulicPuzzle_.Render(renderer);

    for (const auto& cover : coverSpots_) {
        renderer.DrawBox(cover.position, {1.4f, 1.2f, 0.6f}, {0.28f, 0.32f, 0.38f, 1.0f});
    }

    for (const auto& point : grapplePoints_) {
        renderer.DrawBox(point, {0.4f, 0.4f, 0.4f}, {0.9f, 0.85f, 0.2f, 1.0f});
    }

    renderer.DrawBox({0.0f, 2.0f, -12.0f}, {1.0f, 2.0f, 1.0f}, {0.6f, 0.3f, 0.9f, 1.0f});

    combatSystem_->Render(renderer);
}

void LevelManager::CheckObjectives(const PlayerController& player) {
    if (hydraulicPuzzle_.IsSolved()) {
        CompleteObjective("hydraulic_puzzle");
    }

    if (combatSystem_->IsPatrolCleared()) {
        CompleteObjective("cartel_patrol");
    }

    const math::Vec3 throneMirror{0.0f, 2.0f, -12.0f};
    const float dist = (player.GetPosition() - throneMirror).Length();
    if (dist < 3.0f && hydraulicPuzzle_.IsSolved()) {
        CompleteObjective("mirror_throne");
    }

    if (dist < 2.0f && combatSystem_->IsPatrolCleared() && hydraulicPuzzle_.IsSolved()) {
        if (!crystalRetrieved_) {
            crystalRetrieved_ = true;
            CompleteObjective("crystal_retrieve");
            core::Logger::Log(core::LogLevel::Info, "First Echo Crystal recovered. Father's journal unlocked.");
        }
    }

    if (IsLevelComplete()) {
        core::Logger::Log(core::LogLevel::Info, "=== LEVEL COMPLETE: Les Larmes de la Terre ===");
    }
}

void LevelManager::CompleteObjective(const std::string& id) {
    for (auto& obj : objectives_) {
        if (obj.id == id && !obj.completed) {
            obj.completed = true;
            core::Logger::Log(core::LogLevel::Info, "Objective complete: " + obj.description);
        }
    }
}

bool LevelManager::IsLevelComplete() const {
    if (objectives_.empty()) {
        return false;
    }
    for (const auto& obj : objectives_) {
        if (!obj.completed) {
            return false;
        }
    }
    return true;
}

}  // namespace echoes::game
