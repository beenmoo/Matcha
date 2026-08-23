#pragma once

#include "Vector.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace Matcha
{
class Transform
{
public:
    enum class Space
    {
        World,
        Self
    };

public:
    Transform();

    void Translate(const Vector3& translation);
    void Translate(float x, float y, float z);

    void Rotate(const Vector3& eulers, Space space = Space::Self);
    void Rotate(float x, float y, float z, Space space = Space::Self);
    void Rotate(const Vector3& axis, float angle, Space space = Space::Self);

    void SetPosition(const Vector3& position);
    void SetPosition(float x, float y, float z);

    template <typename Self>
    [[nodiscard]] auto& GetPosition(this Self& self)
    {
        return self.mPosition;
    }

    void SetRotation(const Quaternion& rotation);
    void SetRotationEuler(const Vector3& eulers);
    void SetRotationEuler(float x, float y, float z);
    [[nodiscard]] const Quaternion& GetRotation() const;
    [[nodiscard]] Vector3 GetRotationEuler() const;

    void SetScale(const Vector3& scale);
    void SetScale(float x, float y, float z);
    [[nodiscard]] const Vector3& GetScale() const;

    [[nodiscard]] Vector3 GetForward() const;
    [[nodiscard]] Vector3 GetRight() const;
    [[nodiscard]] Vector3 GetUp() const;

    [[nodiscard]] Matrix4 GetModelMatrix() const;

    Transform& operator*=(const Transform& rhs);

private:
    Vector3 mPosition;
    Quaternion mRotation;
    Vector3 mScale;
};
}  // namespace Matcha