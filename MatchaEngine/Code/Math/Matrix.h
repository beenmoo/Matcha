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
[[nodiscard]] bool Decompose(const Matrix4& m, Vector3& scale, Quaternion& rotation, Vector3& translation, Vector3& skew, Vector4& perspective);
}  // namespace Matcha
