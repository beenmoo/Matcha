#pragma once

#include "Vector.h"

namespace Matcha
{
class Quaternion;

class Matrix4
{
public:
    Matrix4()
        : Matrix4(1.0f)
    {
    }
    explicit Matrix4(float diagonal);
    explicit Matrix4(const float data[16]);

    [[nodiscard]] Matrix4 operator*(const Matrix4& rhs) const;

    [[nodiscard]] const float* GetData() const
    {
        return m_Data;
    }

private:
    float m_Data[16] = {};
};

[[nodiscard]] Matrix4 Translate(const Matrix4& m, const Vector3& translation);
[[nodiscard]] Matrix4 Scale(const Matrix4& m, const Vector3& scale);
[[nodiscard]] Vector3 GetTranslation(const Matrix4& m);
[[nodiscard]] bool Decompose(const Matrix4& m, Vector3& scale, Quaternion& rotation, Vector3& translation, Vector3& skew, Vector4& perspective);

// fovDegrees is the full vertical field of view, in degrees.
[[nodiscard]] Matrix4 Perspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane);
[[nodiscard]] Matrix4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

[[nodiscard]] Matrix4 Inverse(const Matrix4& m);
}  // namespace Matcha
