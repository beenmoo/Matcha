#include "Transform.h"

#include <glm/gtx/matrix_decompose.hpp>

namespace Matcha
{
    Transform::Transform() :
        mPosition(0.0f, 0.0f, 0.0f),
        mRotation(Vector3(0.0f, 0.0f, 0.0f)),
        mScale(1.0f, 1.0f, 1.0f)
    {}

    void Transform::Translate(const Vector3& translation)
    {
        mPosition += translation;
    }

    void Transform::Translate(float x, float y, float z)
    {
        mPosition.x += x;
        mPosition.y += y;
        mPosition.z += z;
    }

    void Transform::Rotate(const Vector3& eulers, Space space)
    {
        const Quaternion rot = Quaternion(radians(eulers));

        switch (space)
        {
        case Space::Self:
            mRotation *= rot;
            break;
        case Space::World:
            mRotation = rot * mRotation;
            break;
        default:
            break;
        }
    }

    void Transform::Rotate(float x, float y, float z, Space space)
    {
        Rotate(Vector3(x, y, z), space);
    }

    void Transform::Rotate(const Vector3& axis, float angle, Space space)
    {
        const Quaternion rot = angleAxis(glm::radians(angle), axis);

        switch (space)
        {
        case Space::Self:
            mRotation *= rot;
            break;
        case Space::World:
            mRotation = rot * mRotation;
            break;
        default:
            break;
        }
    }

    void Transform::SetPosition(const Vector3& position)
    {
        mPosition = position;
    }

    void Transform::SetPosition(float x, float y, float z)
    {
        SetPosition({ x, y, z });
    }

    Vector3& Transform::GetPosition()
    {
        return mPosition;
    }

    const Vector3& Transform::GetPosition() const
    {
        return mPosition;
    }

    void Transform::SetRotation(const Quaternion& rotation)
    {
        mRotation = rotation;
    }

    void Transform::SetRotationEuler(const Vector3& eulers)
    {
        mRotation = Quaternion(radians(eulers));
    }

    void Transform::SetRotationEuler(float x, float y, float z)
    {
        SetRotationEuler({ x, y, z });
    }

    const Quaternion& Transform::GetRotation() const
    {
        return mRotation;
    }

    Vector3 Transform::GetRotationEuler() const
    {
        return Vector3(eulerAngles(mRotation));
    }

    void Transform::SetScale(const Vector3& scale)
    {
        mScale = scale;
    }

    void Transform::SetScale(float x, float y, float z)
    {
        SetScale({ x, y, z });
    }

    Vector3 Transform::GetScale() const
    {
        return mScale;
    }

    Vector3 Transform::GetForward() const
    {
        return inverse(mRotation) * Vector3(0.0f, 0.0f, -1.0f);
    }

    Vector3 Transform::GetRight() const
    {
        return inverse(mRotation) * Vector3(1.0f, 0.0f, 0.0f);
    }

    Vector3 Transform::GetUp() const
    {
        return inverse(mRotation) * Vector3(0.0f, 1.0f, 0.0f);
    }

    Matrix4 Transform::GetModelMatrix() const
    {
        Matrix4 model(1.0f);

        const Matrix4 translate = glm::translate(model, mPosition);
        const Matrix4 rotate = toMat4(mRotation);
        const Matrix4 scale = glm::scale(model, mScale);

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
        glm::vec4 perspective;

        decompose(model, scale, rotation, translation, skew, perspective);

        Translate(translation);
        Rotate(eulerAngles(rotation));
        SetScale(scale);

        return *this;
    }
}