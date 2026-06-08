#include "core/Application.h"

#include "core/JobSystem.h"
#include "core/Logger.h"
#include "game/CombatSystem.h"
#include "game/EchoSystem.h"
#include "game/LevelManager.h"
#include "game/PlayerController.h"
#include "platform/Camera.h"
#include "platform/Input.h"
#include "platform/Renderer.h"
#include "platform/Window.h"
#include "math/Mat4.h"

#include <chrono>
#include <cstdio>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace echoes::core {

Application::Application() = default;
Application::~Application() {
    Shutdown();
}

int Application::Run(int argc, char* argv[]) {
    ParseArguments(argc, argv);
    AllocateDebugConsole();

    if (!Initialize()) {
        Logger::Log(LogLevel::Error, "Failed to initialize Echoes of the Lost.");
        return 1;
    }

    Logger::Log(LogLevel::Info, "=== Les Larmes de la Terre ===");
    Logger::Log(LogLevel::Info, "Controls: ZQSD/WASD move | Space jump/climb | F grapple | Q cover");
    Logger::Log(LogLevel::Info, "E interact levers | LMB shoot | 1/2/3 Echo powers | Arrows rotate cam");

    auto lastFrame = std::chrono::steady_clock::now();

    while (running_ && window_->ProcessEvents()) {
        platform::Input::Instance().Update();

        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = std::chrono::duration<float>(now - lastFrame).count();
        lastFrame = now;

        Update(deltaTime);
        Render();
    }

    Shutdown();
    return 0;
}

void Application::AllocateDebugConsole() {
#ifdef _WIN32
    AllocConsole();
    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    freopen_s(&stream, "CONOUT$", "w", stderr);
#endif
}

bool Application::Initialize() {
    jobSystem_ = std::make_unique<JobSystem>();
    window_ = std::make_unique<platform::Window>("Echoes of the Lost", 1280, 720);
    renderer_ = std::make_unique<platform::Renderer>(*window_);
    camera_ = std::make_unique<platform::Camera>();
    echoSystem_ = std::make_unique<game::EchoSystem>();
    player_ = std::make_unique<game::PlayerController>(*echoSystem_);
    levelManager_ = std::make_unique<game::LevelManager>();

    if (!window_->Create()) {
        return false;
    }

    if (!renderer_->Initialize()) {
        Logger::Log(LogLevel::Error, "DirectX 12 renderer failed to initialize.");
        return false;
    }

    if (!levelManager_->LoadLevel(startupLevel_)) {
        Logger::Log(LogLevel::Warning, "Level data not found — running sandbox mode.");
    }

    camera_->SetTarget(player_->GetPosition());
    running_ = true;
    Logger::Log(LogLevel::Info, "Echoes of the Lost initialized successfully.");
    return true;
}

void Application::Shutdown() {
    if (levelManager_) {
        levelManager_->UnloadLevel();
    }
    if (renderer_) {
        renderer_->Shutdown();
    }
    if (window_) {
        window_->Destroy();
    }
    if (jobSystem_) {
        jobSystem_->Shutdown();
    }
    running_ = false;
}

void Application::Update(float deltaTime) {
    levelManager_->Update(deltaTime);
    levelManager_->GetCombatSystem().Update(deltaTime, *player_);
    player_->Update(deltaTime, *levelManager_);
    echoSystem_->Update(deltaTime);

    camera_->SetTarget(player_->GetPosition());

    objectiveCheckTimer_ += deltaTime;
    if (objectiveCheckTimer_ >= 0.5f) {
        levelManager_->CheckObjectives(*player_);
        objectiveCheckTimer_ = 0.0f;
    }

    auto& input = platform::Input::Instance();
#ifdef _WIN32
    if (input.IsKeyDown(VK_RIGHT)) camera_->AddYaw(0.03f);
    if (input.IsKeyDown(VK_LEFT)) camera_->AddYaw(-0.03f);
#endif

    if (player_->GetHealth() <= 0.0f) {
        Logger::Log(LogLevel::Error, "Kaelen fell in battle. Reload the level to retry.");
        running_ = false;
    }
}

void Application::Render() {
    if (!renderer_->IsInitialized()) {
        return;
    }

    const math::Mat4 viewProjection = camera_->GetViewProjection(renderer_->GetAspectRatio());

    renderer_->BeginFrame(viewProjection);
    levelManager_->Render(*renderer_);
    player_->Render(*renderer_);
    renderer_->EndFrame();
}

void Application::ParseArguments(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--level" && i + 1 < argc) {
            startupLevel_ = argv[++i];
        }
    }
}

}  // namespace echoes::core
