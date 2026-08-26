#include "Quaternion.h"
#include "Matrix.h"

#include <glm/gtx/quaternion.hpp>

namespace Matcha
{
namespace
{
glm::quat ToGLM(const Quaternion& q)
{
    return glm::quat(q.w, q.x, q.y, q.z);
}

Quaternion FromGLM(const glm::quat& q)
{
    return Quaternion(q.x, q.y, q.z, q.w);
}

glm::vec3 ToGLM(const Vector3& v)
{
    return glm::vec3(v.x, v.y, v.z);
}

Vector3 FromGLM(const glm::vec3& v)
{
    return Vector3(v.x, v.y, v.z);
}
}  // namespace

Quaternion::Quaternion(const Vector3& eulerRadians)
{
    glm::quat q(ToGLM(eulerRadians));
    x = q.x;
    y = q.y;
    z = q.z;
    w = q.w;
}

Quaternion& Quaternion::operator*=(const Quaternion& rhs)
{
    *this = *this * rhs;
    return *this;
}

Quaternion Quaternion::operator*(const Quaternion& rhs) const
{
    return FromGLM(ToGLM(*this) * ToGLM(rhs));
}

Vector3 Quaternion::operator*(const Vector3& v) const
{
    return FromGLM(ToGLM(*this) * ToGLM(v));
}

Quaternion AngleAxis(float angleRadians, const Vector3& axis)
{
    return FromGLM(glm::angleAxis(angleRadians, ToGLM(axis)));
}

Quaternion Inverse(const Quaternion& q)
{
    return FromGLM(glm::inverse(ToGLM(q)));
}

Vector3 EulerAngles(const Quaternion& q)
{
    return FromGLM(glm::eulerAngles(ToGLM(q)));
}

Matrix4 ToMat4(const Quaternion& q)
{
    glm::mat4 m = glm::toMat4(ToGLM(q));
    return Matrix4(&m[0][0]);
}
}  // namespace Matcha
