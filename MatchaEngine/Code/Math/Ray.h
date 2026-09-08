#pragma once

#include "Matrix.h"
#include "Vector.h"

namespace Matcha
{
struct Ray
{
    Vector3 origin;
    Vector3 direction;  // Expected normalized.
};

// Builds a world-space ray from a viewport-local pixel coordinate (origin top-left, matching
// Qt's mouse-event convention) and the camera's combined view-projection matrix.
[[nodiscard]] Ray ScreenPointToRay(const Vector2& screenPoint, const Vector2& viewportSize, const Matrix4& viewProjection);

// Slab method. Returns whether the ray hits the axis-aligned box at all; outDistance is the
// nearest intersection's distance along the ray (meaningful only when this returns true).
[[nodiscard]] bool IntersectRayAABB(const Ray& ray, const Vector3& boundsMin, const Vector3& boundsMax, float& outDistance);

// Ray vs. an infinite plane (a point on it plus its normal). False if the ray is parallel to the
// plane or the intersection lies behind the ray's origin.
[[nodiscard]] bool IntersectRayPlane(const Ray& ray, const Vector3& planePoint, const Vector3& planeNormal, float& outDistance);
}  // namespace Matcha
