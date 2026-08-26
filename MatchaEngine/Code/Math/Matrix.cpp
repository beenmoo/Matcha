#include "Matrix.h"
#include "Quaternion.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <cstring>

namespace Matcha
{
namespace
{
glm::mat4 ToGLM(const Matrix4& m)
{
    glm::mat4 result;
    std::memcpy(&result[0][0], m.GetData(), sizeof(float) * 16);
    return result;
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

Matrix4::Matrix4(float diagonal)
{
    glm::mat4 m(diagonal);
    std::memcpy(mData, &m[0][0], sizeof(mData));
}

Matrix4::Matrix4(const float data[16])
{
    std::memcpy(mData, data, sizeof(mData));
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    glm::mat4 result = ToGLM(*this) * ToGLM(rhs);
    return Matrix4(&result[0][0]);
}

Matrix4 Translate(const Matrix4& m, const Vector3& translation)
{
    glm::mat4 result = glm::translate(ToGLM(m), ToGLM(translation));
    return Matrix4(&result[0][0]);
}

Matrix4 Scale(const Matrix4& m, const Vector3& scale)
{
    glm::mat4 result = glm::scale(ToGLM(m), ToGLM(scale));
    return Matrix4(&result[0][0]);
}

bool Decompose(const Matrix4& m, Vector3& scale, Quaternion& rotation, Vector3& translation, Vector3& skew, Vector4& perspective)
{
    glm::vec3 glmScale;
    glm::quat glmRotation;
    glm::vec3 glmTranslation;
    glm::vec3 glmSkew;
    glm::vec4 glmPerspective;

    bool success = glm::decompose(ToGLM(m), glmScale, glmRotation, glmTranslation, glmSkew, glmPerspective);

    scale = FromGLM(glmScale);
    rotation = Quaternion(glmRotation.x, glmRotation.y, glmRotation.z, glmRotation.w);
    translation = FromGLM(glmTranslation);
    skew = FromGLM(glmSkew);
    perspective = Vector4(glmPerspective.x, glmPerspective.y, glmPerspective.z, glmPerspective.w);

    return success;
}
}  // namespace Matcha
