#pragma once

#include "Vector.h"

namespace Matcha
{
class Matrix4;

class Quaternion
{
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quaternion() = default;
    constexpr Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
    {
    }
    explicit Quaternion(const Vector3& eulerRadians);

    Quaternion& operator*=(const Quaternion& rhs);

    [[nodiscard]] Quaternion operator*(const Quaternion& rhs) const;
    [[nodiscard]] Vector3 operator*(const Vector3& v) const;

    [[nodiscard]] constexpr bool operator==(const Quaternion&) const = default;
};

[[nodiscard]] Quaternion AngleAxis(float angleRadians, const Vector3& axis);
[[nodiscard]] Quaternion Inverse(const Quaternion& q);
[[nodiscard]] Vector3 EulerAngles(const Quaternion& q);
[[nodiscard]] Matrix4 ToMat4(const Quaternion& q);
}  // namespace Matcha
