#pragma once

#include "math/Mat4.h"
#include "math/Vec3.h"

#include <vector>

namespace echoes::platform {

class Window;

struct RenderColor {
    float r{1.0f};
    float g{1.0f};
    float b{1.0f};
    float a{1.0f};
};

struct DrawInstance {
    math::Vec3 position;
    math::Vec3 scale{1.0f, 1.0f, 1.0f};
    RenderColor color;
};

class Renderer {
public:
    explicit Renderer(Window& window);
    ~Renderer();

    bool Initialize();
    void Shutdown();

    void BeginFrame(const math::Mat4& viewProjection);
    void EndFrame();

    void DrawBox(const math::Vec3& pos, const math::Vec3& scale, const RenderColor& color);
    void DrawWaterPlane(float waterLevel, float extent);
    void DrawTempleScene();
    void FlushDrawQueue();

    bool IsInitialized() const { return initialized_; }
    float GetAspectRatio() const;

private:
    bool CreateDevice();
    bool CreateSwapChain();
    bool CreateRenderTargets();
    bool CreatePipeline();
    bool CreateCubeGeometry();
    void WaitForGpu();
    void MoveToNextFrame();

    Window& window_;
    bool initialized_{false};

    struct Dx12State;
    Dx12State* dx_{nullptr};

    math::Mat4 viewProjection_{};
    std::vector<DrawInstance> drawQueue_;
};

}  // namespace echoes::platform
