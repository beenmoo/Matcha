#include "pch.h"
#include "Math/Matrix.h"
#include "Math/Ray.h"
#include "MathTestUtils.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

TEST(RayTests, ScreenPointToRayAtCenterPointsStraightAhead)
{
    // Camera at the world origin looking down -Z (identity view), orthographic so the math is
    // exact rather than perspective-distorted - -Z is "ahead" by OpenGL's own convention, which
    // this engine's projection matrices (Perspective/Orthographic in Matrix.cpp) already follow.
    Matrix4 projection = Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    Vector2 viewportSize(800.0f, 600.0f);

    Ray ray = ScreenPointToRay(Vector2(400.0f, 300.0f), viewportSize, projection);

    ExpectVectorNear(ray.origin, Vector3(0.0f, 0.0f, -0.1f));
    ExpectVectorNear(ray.direction, Vector3(0.0f, 0.0f, -1.0f));
}

TEST(RayTests, ScreenPointToRayAtTopLeftMapsToNegativeXPositiveY)
{
    // Qt reports (0,0) as the viewport's top-left corner - should land at NDC (-1, +1), which
    // this ortho projection maps to world (-1, +1, ...).
    Matrix4 projection = Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    Vector2 viewportSize(800.0f, 600.0f);

    Ray ray = ScreenPointToRay(Vector2(0.0f, 0.0f), viewportSize, projection);

    ExpectVectorNear(ray.origin, Vector3(-1.0f, 1.0f, -0.1f));
}

TEST(RayTests, IntersectRayAABBHitsUnitCubeAtOrigin)
{
    Ray ray{Vector3(0.0f, 0.0f, 5.0f), Vector3(0.0f, 0.0f, -1.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayAABB(ray, Vector3(-0.5f), Vector3(0.5f), distance);

    EXPECT_TRUE(hit);
    EXPECT_NEAR(distance, 4.5f, 1e-5f);
}

TEST(RayTests, IntersectRayAABBMissesWhenOffToTheSide)
{
    Ray ray{Vector3(5.0f, 5.0f, 5.0f), Vector3(0.0f, 0.0f, -1.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayAABB(ray, Vector3(-0.5f), Vector3(0.5f), distance);

    EXPECT_FALSE(hit);
}

TEST(RayTests, IntersectRayAABBMissesWhenBoxIsBehindOrigin)
{
    Ray ray{Vector3(0.0f, 0.0f, -5.0f), Vector3(0.0f, 0.0f, -1.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayAABB(ray, Vector3(-0.5f), Vector3(0.5f), distance);

    EXPECT_FALSE(hit);
}

TEST(RayTests, IntersectRayPlaneHitsAlignedPlane)
{
    Ray ray{Vector3(0.0f, 5.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayPlane(ray, Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), distance);

    EXPECT_TRUE(hit);
    EXPECT_NEAR(distance, 3.0f, 1e-5f);
}

TEST(RayTests, IntersectRayPlaneMissesWhenParallel)
{
    Ray ray{Vector3(0.0f, 5.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayPlane(ray, Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), distance);

    EXPECT_FALSE(hit);
}

TEST(RayTests, IntersectRayPlaneMissesWhenBehindOrigin)
{
    Ray ray{Vector3(0.0f, 5.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f)};

    float distance = 0.0f;
    bool hit = IntersectRayPlane(ray, Vector3(0.0f, 2.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), distance);

    EXPECT_FALSE(hit);
}
