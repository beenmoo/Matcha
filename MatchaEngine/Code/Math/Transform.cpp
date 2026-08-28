#include "Transform.h"

#include <utility>

namespace Matcha
{
Transform::Transform()
    : m_Position(0.0f),
      m_Rotation(Vector3(0.0f)),
      m_Scale(1.0f)
{
}

void Transform::Translate(const Vector3& translation)
{
    m_Position += translation;
}

void Transform::Translate(float x, float y, float z)
{
    m_Position.x += x;
    m_Position.y += y;
    m_Position.z += z;
}

void Transform::Rotate(const Vector3& eulers, Space space)
{
    const Quaternion rot = Quaternion(Radians(eulers));

    switch (space)
    {
    case Space::Self:
        m_Rotation *= rot;
        break;
    case Space::World:
        m_Rotation = rot * m_Rotation;
        break;
    default:
        std::unreachable();
    }
}

void Transform::Rotate(float x, float y, float z, Space space)
{
    Rotate(Vector3(x, y, z), space);
}

void Transform::Rotate(const Vector3& axis, float angle, Space space)
{
    const Quaternion rot = AngleAxis(Radians(angle), axis);

    switch (space)
    {
    case Space::Self:
        m_Rotation *= rot;
        break;
    case Space::World:
        m_Rotation = rot * m_Rotation;
        break;
    default:
        std::unreachable();
    }
}

void Transform::SetPosition(const Vector3& position)
{
    m_Position = position;
}

void Transform::SetPosition(float x, float y, float z)
{
    SetPosition({x, y, z});
}

void Transform::SetRotation(const Quaternion& rotation)
{
    m_Rotation = rotation;
}

void Transform::SetRotationEuler(const Vector3& eulers)
{
    m_Rotation = Quaternion(Radians(eulers));
}

void Transform::SetRotationEuler(float x, float y, float z)
{
    SetRotationEuler({x, y, z});
}

const Quaternion& Transform::GetRotation() const
{
    return m_Rotation;
}

Vector3 Transform::GetRotationEuler() const
{
    return EulerAngles(m_Rotation);
}

void Transform::SetScale(const Vector3& scale)
{
    m_Scale = scale;
}

void Transform::SetScale(float x, float y, float z)
{
    SetScale({x, y, z});
}

const Vector3& Transform::GetScale() const
{
    return m_Scale;
}

Vector3 Transform::GetForward() const
{
    // m_Rotation directly, not its inverse - matches GetLocalMatrix()/ToMat4(), which use
    // m_Rotation as the object's local-to-world orientation. Using Inverse(m_Rotation) here
    // rotated these vectors in the opposite rotational sense from the object's actual visual
    // orientation the moment any real rotation was applied (identity-only tests never caught it,
    // since Inverse(q)*v is still orthonormal and unit-length - just pointing the wrong way).
    return m_Rotation * Vector3(0.0f, 0.0f, -1.0f);
}

Vector3 Transform::GetRight() const
{
    return m_Rotation * Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Transform::GetUp() const
{
    return m_Rotation * Vector3(0.0f, 1.0f, 0.0f);
}

Matrix4 Transform::GetLocalMatrix() const
{
    Matrix4 model(1.0f);

    const Matrix4 translate = Matcha::Translate(model, m_Position);
    const Matrix4 rotate = ToMat4(m_Rotation);
    const Matrix4 scale = Matcha::Scale(model, m_Scale);

    model = translate * rotate * scale;

    return model;
}

Transform& Transform::operator*=(const Transform& rhs)
{
    const Matrix4 model = GetLocalMatrix() * rhs.GetLocalMatrix();

    Vector3 translation;
    Quaternion rotation;
    Vector3 scale;
    Vector3 skew;
    Vector4 perspective;

    (void)Decompose(model, scale, rotation, translation, skew, perspective);

    Translate(translation);
    Rotate(EulerAngles(rotation));
    SetScale(scale);

    return *this;
}
}  // namespace Matcha
