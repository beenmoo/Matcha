#include "pch.h"
#include "Math/Transform.h"
#include "MathTestUtils.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

TEST(TransformTests, DefaultTransformIsIdentity)
{
    Transform t;

    ExpectVectorNear(t.GetPosition(), Vector3(0.0f));
    ExpectVectorNear(t.GetScale(), Vector3(1.0f));
    ExpectQuaternionNear(t.GetRotation(), Quaternion());

    Matrix4 identity;
    ExpectMatrixNear(t.GetLocalMatrix(), identity);
}

TEST(TransformTests, DefaultOrientationVectors)
{
    Transform t;

    ExpectVectorNear(t.GetForward(), Vector3(0.0f, 0.0f, -1.0f));
    ExpectVectorNear(t.GetRight(), Vector3(1.0f, 0.0f, 0.0f));
    ExpectVectorNear(t.GetUp(), Vector3(0.0f, 1.0f, 0.0f));
}

TEST(TransformTests, TranslateMovesPosition)
{
    Transform t;
    t.Translate(Vector3(1.0f, 2.0f, 3.0f));

    ExpectVectorNear(t.GetPosition(), Vector3(1.0f, 2.0f, 3.0f));

    t.Translate(1.0f, 0.0f, 0.0f);
    ExpectVectorNear(t.GetPosition(), Vector3(2.0f, 2.0f, 3.0f));
}

TEST(TransformTests, SetPositionOverwrites)
{
    Transform t;
    t.Translate(Vector3(5.0f, 5.0f, 5.0f));
    t.SetPosition(1.0f, 2.0f, 3.0f);

    ExpectVectorNear(t.GetPosition(), Vector3(1.0f, 2.0f, 3.0f));
}

TEST(TransformTests, SetScale)
{
    Transform t;
    t.SetScale(2.0f, 3.0f, 4.0f);

    ExpectVectorNear(t.GetScale(), Vector3(2.0f, 3.0f, 4.0f));
}

TEST(TransformTests, RotatePreservesOrthonormalBasis)
{
    Transform t;
    // Transform::Rotate takes degrees, not radians.
    t.Rotate(Vector3(0.0f, 1.0f, 0.0f), 90.0f);

    Vector3 forward = t.GetForward();
    Vector3 right = t.GetRight();
    Vector3 up = t.GetUp();

    auto dot = [](const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; };

    EXPECT_NEAR(dot(forward, right), 0.0f, kEpsilon);
    EXPECT_NEAR(dot(forward, up), 0.0f, kEpsilon);
    EXPECT_NEAR(dot(right, up), 0.0f, kEpsilon);

    EXPECT_NEAR(dot(forward, forward), 1.0f, kEpsilon);
    EXPECT_NEAR(dot(right, right), 1.0f, kEpsilon);
    EXPECT_NEAR(dot(up, up), 1.0f, kEpsilon);
}

TEST(TransformTests, GetForwardMatchesActualOrientationAfterRotation)
{
    Transform t;
    // Transform::Rotate takes degrees, not radians.
    t.Rotate(Vector3(0.0f, 1.0f, 0.0f), 90.0f);

    // Cross-checked against the model matrix's own recovered rotation - a path independent of
    // GetForward()'s own implementation - so this actually verifies GetForward() points the way
    // the object is visually oriented, not just that it stays orthonormal (RotatePreservesOrthonormalBasis
    // above would pass even if GetForward()/GetRight()/GetUp() all used Inverse(m_Rotation), which
    // is precisely the bug this test catches: that variant is still a valid orthonormal basis,
    // just rotated the wrong way relative to how GetLocalMatrix() actually renders the object).
    Vector3 scale;
    Quaternion recoveredRotation;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;

    bool success = Decompose(t.GetLocalMatrix(), scale, recoveredRotation, translation, skew, perspective);

    EXPECT_TRUE(success);
    ExpectVectorNear(t.GetForward(), recoveredRotation * Vector3(0.0f, 0.0f, -1.0f));
    ExpectVectorNear(t.GetRight(), recoveredRotation * Vector3(1.0f, 0.0f, 0.0f));
    ExpectVectorNear(t.GetUp(), recoveredRotation * Vector3(0.0f, 1.0f, 0.0f));
}

TEST(TransformTests, RotationEulerRoundTrip)
{
    Transform t;
    // SetRotationEuler takes degrees; GetRotationEuler returns radians.
    t.SetRotationEuler(0.0f, 45.0f, 0.0f);

    Vector3 eulers = t.GetRotationEuler();

    ExpectVectorNear(eulers, Vector3(0.0f, Radians(45.0f), 0.0f), 1e-3f);
}

TEST(TransformTests, LocalMatrixMatchesTranslateAndScaleComposition)
{
    Transform t;
    t.SetPosition(1.0f, 0.0f, 0.0f);
    t.SetScale(2.0f, 2.0f, 2.0f);

    Matrix4 model = t.GetLocalMatrix();

    Vector3 scale;
    Quaternion rotation;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;

    bool success = Decompose(model, scale, rotation, translation, skew, perspective);

    EXPECT_TRUE(success);
    ExpectVectorNear(translation, Vector3(1.0f, 0.0f, 0.0f));
    ExpectVectorNear(scale, Vector3(2.0f, 2.0f, 2.0f));
    ExpectQuaternionNear(rotation, Quaternion());
}
