#pragma once

#include <memory>
#include <string>

namespace echoes::core {
class JobSystem;
}

namespace echoes::game {
class CombatSystem;
class EchoSystem;
class LevelManager;
class PlayerController;
}

namespace echoes::platform {
class Camera;
class Renderer;
class Window;
}

namespace echoes::core {

class Application {
public:
    Application();
    ~Application();

    int Run(int argc, char* argv[]);

private:
    bool Initialize();
    void Shutdown();
    void Update(float deltaTime);
    void Render();
    void ParseArguments(int argc, char* argv[]);
    void AllocateDebugConsole();

    std::unique_ptr<platform::Window> window_;
    std::unique_ptr<platform::Renderer> renderer_;
    std::unique_ptr<platform::Camera> camera_;
    std::unique_ptr<JobSystem> jobSystem_;
    std::unique_ptr<game::EchoSystem> echoSystem_;
    std::unique_ptr<game::PlayerController> player_;
    std::unique_ptr<game::LevelManager> levelManager_;

    std::string startupLevel_{"levels/mohenjo_daro/les_larmes_de_la_terre"};
    bool running_{false};
    float objectiveCheckTimer_{0.0f};
};

}  // namespace echoes::core
