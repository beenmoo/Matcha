#include "Transform.h"

#include <utility>

namespace Matcha
{
Transform::Transform() : m_Position(0.0f),
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
    return Inverse(m_Rotation) * Vector3(0.0f, 0.0f, -1.0f);
}

Vector3 Transform::GetRight() const
{
    return Inverse(m_Rotation) * Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Transform::GetUp() const
{
    return Inverse(m_Rotation) * Vector3(0.0f, 1.0f, 0.0f);
}

Matrix4 Transform::GetModelMatrix() const
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
    const Matrix4 model = GetModelMatrix() * rhs.GetModelMatrix();

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
