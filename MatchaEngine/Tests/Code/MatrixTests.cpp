#include "pch.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "MathTestUtils.h"

#include <gtest/gtest.h>

using namespace Matcha;
using namespace MatchaTests;

TEST(Matrix4Tests, DefaultConstructorIsIdentity)
{
    Matrix4 m;
    const float* data = m.GetData();

    const float expected[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1};

    for (int i = 0; i < 16; ++i)
        EXPECT_FLOAT_EQ(data[i], expected[i]) << "at index " << i;
}

TEST(Matrix4Tests, DiagonalConstructorSetsDiagonalOnly)
{
    Matrix4 m(2.0f);
    const float* data = m.GetData();

    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[5], 2.0f);
    EXPECT_FLOAT_EQ(data[10], 2.0f);
    EXPECT_FLOAT_EQ(data[15], 2.0f);

    EXPECT_FLOAT_EQ(data[1], 0.0f);
    EXPECT_FLOAT_EQ(data[4], 0.0f);
    EXPECT_FLOAT_EQ(data[12], 0.0f);
}

TEST(Matrix4Tests, MultiplyByIdentityIsNoOp)
{
    Matrix4 m = Scale(Matrix4(1.0f), Vector3(2.0f, 3.0f, 4.0f));
    Matrix4 identity;

    ExpectMatrixNear(m * identity, m);
    ExpectMatrixNear(identity * m, m);
}

TEST(Matrix4Tests, TranslateSetsTranslationColumn)
{
    Matrix4 translated = Translate(Matrix4(1.0f), Vector3(1.0f, 2.0f, 3.0f));
    const float* data = translated.GetData();

    // Column-major storage: translation lives in column 3 (indices 12-14).
    EXPECT_FLOAT_EQ(data[12], 1.0f);
    EXPECT_FLOAT_EQ(data[13], 2.0f);
    EXPECT_FLOAT_EQ(data[14], 3.0f);
}

TEST(Matrix4Tests, ScaleSetsDiagonal)
{
    Matrix4 scaled = Scale(Matrix4(1.0f), Vector3(2.0f, 3.0f, 4.0f));
    const float* data = scaled.GetData();

    EXPECT_FLOAT_EQ(data[0], 2.0f);
    EXPECT_FLOAT_EQ(data[5], 3.0f);
    EXPECT_FLOAT_EQ(data[10], 4.0f);
}

TEST(Matrix4Tests, DecomposeRecoversTranslationAndScale)
{
    Matrix4 model = Translate(Matrix4(1.0f), Vector3(5.0f, -2.0f, 1.0f));
    model = Scale(model, Vector3(2.0f, 2.0f, 2.0f));

    Vector3 scale;
    Quaternion rotation;
    Vector3 translation;
    Vector3 skew;
    Vector4 perspective;

    bool success = Decompose(model, scale, rotation, translation, skew, perspective);

    EXPECT_TRUE(success);
    ExpectVectorNear(translation, Vector3(5.0f, -2.0f, 1.0f));
    ExpectVectorNear(scale, Vector3(2.0f, 2.0f, 2.0f));
    ExpectQuaternionNear(rotation, Quaternion());
}

TEST(Matrix4Tests, PerspectiveWithNinetyDegreeFovAndUnitAspect)
{
    // tan(45 deg) == 1, so both axis scale terms collapse to 1 and the math is easy to check by hand.
    Matrix4 projection = Perspective(90.0f, 1.0f, 0.1f, 100.0f);
    const float* data = projection.GetData();

    EXPECT_NEAR(data[0], 1.0f, kEpsilon);
    EXPECT_NEAR(data[5], 1.0f, kEpsilon);
    EXPECT_NEAR(data[11], -1.0f, kEpsilon);
    EXPECT_NEAR(data[10], -(100.0f + 0.1f) / (100.0f - 0.1f), kEpsilon);
    EXPECT_NEAR(data[14], -(2.0f * 100.0f * 0.1f) / (100.0f - 0.1f), kEpsilon);
}

TEST(Matrix4Tests, OrthographicWithSymmetricUnitCube)
{
    Matrix4 projection = Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    const float* data = projection.GetData();

    EXPECT_NEAR(data[0], 1.0f, kEpsilon);
    EXPECT_NEAR(data[5], 1.0f, kEpsilon);
    EXPECT_NEAR(data[10], -1.0f, kEpsilon);
    EXPECT_NEAR(data[12], 0.0f, kEpsilon);
    EXPECT_NEAR(data[13], 0.0f, kEpsilon);
    EXPECT_NEAR(data[14], 0.0f, kEpsilon);
}
