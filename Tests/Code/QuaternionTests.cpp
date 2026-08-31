#include "pch.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "MathTestUtils.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

TEST(QuaternionTests, DefaultConstructorIsIdentity)
{
    Quaternion q;

    EXPECT_FLOAT_EQ(q.x, 0.0f);
    EXPECT_FLOAT_EQ(q.y, 0.0f);
    EXPECT_FLOAT_EQ(q.z, 0.0f);
    EXPECT_FLOAT_EQ(q.w, 1.0f);
}

TEST(QuaternionTests, ZeroEulerAnglesIsIdentity)
{
    Quaternion q(Vector3(0.0f));

    ExpectQuaternionNear(q, Quaternion());
}

TEST(QuaternionTests, IdentityDoesNotRotateVector)
{
    Quaternion identity;
    Vector3 v(1.0f, 2.0f, 3.0f);

    ExpectVectorNear(identity * v, v);
}

TEST(QuaternionTests, AngleAxisPreservesVectorLength)
{
    // AngleAxis takes radians directly, unlike Transform::Rotate which takes degrees.
    Quaternion rot = AngleAxis(Radians(37.0f), Vector3(0.0f, 1.0f, 0.0f));
    Vector3 v(1.0f, 0.0f, 0.0f);

    Vector3 rotated = rot * v;

    EXPECT_NEAR(Length(rotated), Length(v), kEpsilon);
}

TEST(QuaternionTests, FullRevolutionReturnsToOriginal)
{
    Quaternion rot = AngleAxis(Radians(360.0f), Vector3(0.0f, 1.0f, 0.0f));
    Vector3 v(1.0f, 2.0f, 3.0f);

    ExpectVectorNear(rot * v, v, 1e-3f);
}

TEST(QuaternionTests, InverseUndoesRotation)
{
    Quaternion rot = AngleAxis(Radians(53.0f), Vector3(0.0f, 0.0f, 1.0f));
    Vector3 v(0.4f, -1.2f, 2.5f);

    Vector3 rotated = rot * v;
    Vector3 restored = Inverse(rot) * rotated;

    ExpectVectorNear(restored, v);
}

TEST(QuaternionTests, ComposingTwoRotationsMatchesCombinedRotation)
{
    Quaternion first = AngleAxis(Radians(30.0f), Vector3(0.0f, 1.0f, 0.0f));
    Quaternion second = AngleAxis(Radians(60.0f), Vector3(0.0f, 1.0f, 0.0f));
    Quaternion combined = AngleAxis(Radians(90.0f), Vector3(0.0f, 1.0f, 0.0f));

    Vector3 v(1.0f, 0.0f, 0.0f);

    Vector3 stepwise = second * (first * v);
    Vector3 direct = combined * v;

    ExpectVectorNear(stepwise, direct);
}

TEST(QuaternionTests, EulerRoundTrip)
{
    Vector3 eulers = Radians(Vector3(20.0f, 35.0f, -15.0f));
    Quaternion q(eulers);

    Vector3 recovered = EulerAngles(q);

    ExpectVectorNear(recovered, eulers, 1e-3f);
}

TEST(QuaternionTests, ToMat4OfIdentityIsIdentityMatrix)
{
    Matrix4 m = ToMat4(Quaternion());
    Matrix4 identity;

    ExpectMatrixNear(m, identity);
}
