#pragma once

#include "math/Vec3.h"

#include <cmath>
#include <cstring>

namespace echoes::math {

struct Mat4 {
    float m[16]{};

    static Mat4 Identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    static Mat4 Perspective(float fovY, float aspect, float nearZ, float farZ) {
        Mat4 r;
        const float f = 1.0f / std::tan(fovY * 0.5f);
        r.m[0] = f / aspect;
        r.m[5] = f;
        r.m[10] = farZ / (nearZ - farZ);
        r.m[11] = -1.0f;
        r.m[14] = (nearZ * farZ) / (nearZ - farZ);
        return r;
    }

    static Mat4 LookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        const Vec3 z = (eye - target).Normalized();
        const Vec3 x = Vec3::Cross(up, z).Normalized();
        const Vec3 y = Vec3::Cross(z, x);

        Mat4 r = Identity();
        r.m[0] = x.x; r.m[1] = y.x; r.m[2] = z.x;
        r.m[4] = x.y; r.m[5] = y.y; r.m[6] = z.y;
        r.m[8] = x.z; r.m[9] = y.z; r.m[10] = z.z;
        r.m[12] = -Vec3::Dot(x, eye);
        r.m[13] = -Vec3::Dot(y, eye);
        r.m[14] = -Vec3::Dot(z, eye);
        return r;
    }

    static Mat4 Translation(float x, float y, float z) {
        Mat4 r = Identity();
        r.m[12] = x;
        r.m[13] = y;
        r.m[14] = z;
        return r;
    }

    static Mat4 Scale(float x, float y, float z) {
        Mat4 r = Identity();
        r.m[0] = x;
        r.m[5] = y;
        r.m[10] = z;
        return r;
    }

    static Mat4 Multiply(const Mat4& a, const Mat4& b) {
        Mat4 r;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                }
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    return Mat4::Multiply(a, b);
}

}  // namespace echoes::math
