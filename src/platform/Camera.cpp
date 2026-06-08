#include "platform/Camera.h"

#include <algorithm>
#include <cmath>

namespace echoes::platform {

void Camera::SetTarget(const math::Vec3& target) {
    target_ = target;
}

void Camera::AddPitch(float delta) {
    pitch_ = std::clamp(pitch_ + delta, -1.2f, 0.3f);
}

math::Vec3 Camera::GetPosition() const {
    const float cp = std::cos(pitch_);
    const float sp = std::sin(pitch_);
    const float cy = std::cos(yaw_);
    const float sy = std::sin(yaw_);

    return {
        target_.x + distance_ * cp * sy,
        target_.y + distance_ * sp + 1.5f,
        target_.z + distance_ * cp * cy
    };
}

math::Mat4 Camera::GetViewMatrix() const {
    return math::Mat4::LookAt(GetPosition(), target_, {0.0f, 1.0f, 0.0f});
}

math::Mat4 Camera::GetProjectionMatrix(float aspect) const {
    return math::Mat4::Perspective(1.1f, aspect, 0.1f, 250.0f);
}

math::Mat4 Camera::GetViewProjection(float aspect) const {
    return GetProjectionMatrix(aspect) * GetViewMatrix();
}

}  // namespace echoes::platform
