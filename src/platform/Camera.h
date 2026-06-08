#pragma once

#include "math/Mat4.h"
#include "math/Vec3.h"

namespace echoes::platform {

class Camera {
public:
    void SetTarget(const math::Vec3& target);
    void SetDistance(float distance) { distance_ = distance; }
    void AddYaw(float delta) { yaw_ += delta; }
    void AddPitch(float delta);

    math::Vec3 GetPosition() const;
    math::Mat4 GetViewMatrix() const;
    math::Mat4 GetProjectionMatrix(float aspect) const;
    math::Mat4 GetViewProjection(float aspect) const;

private:
    math::Vec3 target_{0.0f, 1.5f, 0.0f};
    float distance_{8.0f};
    float yaw_{0.0f};
    float pitch_{-0.35f};
};

}  // namespace echoes::platform
