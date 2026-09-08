#include "Ray.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace Matcha
{
namespace
{
// Mirrors Matrix.cpp's own private ToGLM/FromGLM helpers - kept local here rather than shared
// since Matrix4 exposes no glm-typed interface, only the raw float[16] via GetData().
glm::mat4 ToGLM(const Matrix4& m)
{
    glm::mat4 result;
    std::memcpy(&result[0][0], m.GetData(), sizeof(float) * 16);
    return result;
}

Vector3 FromGLM(const glm::vec3& v)
{
    return Vector3(v.x, v.y, v.z);
}

// One axis of the slab method: narrows [tMin, tMax] to the ray's overlap with this axis's
// [boundsMin, boundsMax] slab. Returns false the moment the interval becomes empty.
bool NarrowSlab(float origin, float direction, float boundsMin, float boundsMax, float& tMin, float& tMax)
{
    constexpr float epsilon = 1e-8f;

    if (std::abs(direction) < epsilon)
        return origin >= boundsMin && origin <= boundsMax;

    float invDirection = 1.0f / direction;
    float t1 = (boundsMin - origin) * invDirection;
    float t2 = (boundsMax - origin) * invDirection;
    if (t1 > t2)
        std::swap(t1, t2);

    tMin = std::max(tMin, t1);
    tMax = std::min(tMax, t2);
    return tMin <= tMax;
}
}  // namespace

Ray ScreenPointToRay(const Vector2& screenPoint, const Vector2& viewportSize, const Matrix4& viewProjection)
{
    glm::mat4 vp = ToGLM(viewProjection);
    glm::vec4 viewport(0.0f, 0.0f, viewportSize.x, viewportSize.y);

    // Qt's y grows downward from the top; glm::unProject's viewport convention (like OpenGL's
    // own) has y=0 at the bottom, so flip before mapping into NDC. model is left as identity -
    // there's no separate view matrix here, only the already-combined viewProjection - which
    // unProject accepts by passing it as `proj` (it only ever uses inverse(proj * model)).
    float flippedY = viewportSize.y - screenPoint.y;

    glm::vec3 nearPoint = glm::unProject(glm::vec3(screenPoint.x, flippedY, 0.0f), glm::mat4(1.0f), vp, viewport);
    glm::vec3 farPoint = glm::unProject(glm::vec3(screenPoint.x, flippedY, 1.0f), glm::mat4(1.0f), vp, viewport);

    Vector3 origin = FromGLM(nearPoint);
    Vector3 direction = Normalize(FromGLM(farPoint - nearPoint));

    return Ray{origin, direction};
}

bool IntersectRayAABB(const Ray& ray, const Vector3& boundsMin, const Vector3& boundsMax, float& outDistance)
{
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::max();

    if (!NarrowSlab(ray.origin.x, ray.direction.x, boundsMin.x, boundsMax.x, tMin, tMax))
        return false;
    if (!NarrowSlab(ray.origin.y, ray.direction.y, boundsMin.y, boundsMax.y, tMin, tMax))
        return false;
    if (!NarrowSlab(ray.origin.z, ray.direction.z, boundsMin.z, boundsMax.z, tMin, tMax))
        return false;

    outDistance = tMin;
    return true;
}

bool IntersectRayPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& outDistance)
{
    constexpr float epsilon = 1e-8f;

    float denominator = Dot(planeNormal, ray.direction);
    if (std::abs(denominator) < epsilon)
        return false;

    outDistance = Dot(planePoint - ray.origin, planeNormal) / denominator;
    return outDistance >= 0.0f;
}
}  // namespace Matcha
