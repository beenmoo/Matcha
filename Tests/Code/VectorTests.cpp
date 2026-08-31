#include "pch.h"
#include "Math/Vector.h"

#include <gtest/gtest.h>

using namespace Matcha;

TEST(Vector2Tests, DefaultConstructorIsZero)
{
    Vector2 v;

    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vector2Tests, ScalarConstructorBroadcasts)
{
    Vector2 v(3.0f);

    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, 3.0f);
}

TEST(Vector2Tests, AdditionAndSubtraction)
{
    Vector2 a(1.0f, 2.0f);
    Vector2 b(3.0f, 4.0f);

    Vector2 sum = a + b;
    EXPECT_FLOAT_EQ(sum.x, 4.0f);
    EXPECT_FLOAT_EQ(sum.y, 6.0f);

    Vector2 diff = b - a;
    EXPECT_FLOAT_EQ(diff.x, 2.0f);
    EXPECT_FLOAT_EQ(diff.y, 2.0f);

    a += b;
    EXPECT_EQ(a, sum);
}

TEST(Vector2Tests, ScalarMultiplication)
{
    Vector2 v(2.0f, 3.0f);
    Vector2 scaled = v * 2.0f;

    EXPECT_FLOAT_EQ(scaled.x, 4.0f);
    EXPECT_FLOAT_EQ(scaled.y, 6.0f);
}

TEST(Vector2Tests, Equality)
{
    EXPECT_EQ(Vector2(1.0f, 2.0f), Vector2(1.0f, 2.0f));
    EXPECT_NE(Vector2(1.0f, 2.0f), Vector2(1.0f, 3.0f));
}

TEST(Vector2IntTests, ComponentConstructor)
{
    Vector2Int v(4, -2);

    EXPECT_EQ(v.x, 4);
    EXPECT_EQ(v.y, -2);
}

TEST(Vector3Tests, DefaultConstructorIsZero)
{
    Vector3 v;

    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
    EXPECT_FLOAT_EQ(v.z, 0.0f);
}

TEST(Vector3Tests, ScalarConstructorBroadcasts)
{
    Vector3 v(2.0f);

    EXPECT_FLOAT_EQ(v.x, 2.0f);
    EXPECT_FLOAT_EQ(v.y, 2.0f);
    EXPECT_FLOAT_EQ(v.z, 2.0f);
}

TEST(Vector3Tests, AdditionSubtractionScalarMultiplication)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(4.0f, 5.0f, 6.0f);

    Vector3 sum = a + b;
    EXPECT_FLOAT_EQ(sum.x, 5.0f);
    EXPECT_FLOAT_EQ(sum.y, 7.0f);
    EXPECT_FLOAT_EQ(sum.z, 9.0f);

    Vector3 diff = b - a;
    EXPECT_FLOAT_EQ(diff.x, 3.0f);
    EXPECT_FLOAT_EQ(diff.y, 3.0f);
    EXPECT_FLOAT_EQ(diff.z, 3.0f);

    Vector3 scaled = a * 3.0f;
    EXPECT_FLOAT_EQ(scaled.x, 3.0f);
    EXPECT_FLOAT_EQ(scaled.y, 6.0f);
    EXPECT_FLOAT_EQ(scaled.z, 9.0f);
}

TEST(Vector3Tests, UnaryNegation)
{
    Vector3 v(1.0f, -2.0f, 3.0f);
    Vector3 negated = -v;

    EXPECT_FLOAT_EQ(negated.x, -1.0f);
    EXPECT_FLOAT_EQ(negated.y, 2.0f);
    EXPECT_FLOAT_EQ(negated.z, -3.0f);
}

TEST(Vector3Tests, CompoundAssignment)
{
    Vector3 v(1.0f, 1.0f, 1.0f);

    v += Vector3(1.0f, 2.0f, 3.0f);
    EXPECT_EQ(v, Vector3(2.0f, 3.0f, 4.0f));

    v -= Vector3(1.0f, 1.0f, 1.0f);
    EXPECT_EQ(v, Vector3(1.0f, 2.0f, 3.0f));

    v *= 2.0f;
    EXPECT_EQ(v, Vector3(2.0f, 4.0f, 6.0f));
}

TEST(Vector3Tests, LengthAndNormalize)
{
    Vector3 v(3.0f, 0.0f, 4.0f);

    EXPECT_FLOAT_EQ(Length(v), 5.0f);

    Vector3 normalized = Normalize(v);
    EXPECT_NEAR(Length(normalized), 1.0f, 1e-6f);
    EXPECT_NEAR(normalized.x, 0.6f, 1e-6f);
    EXPECT_NEAR(normalized.z, 0.8f, 1e-6f);
}

TEST(Vector3Tests, NormalizeOfZeroVectorReturnsZero)
{
    EXPECT_EQ(Normalize(Vector3(0.0f)), Vector3(0.0f));
}

TEST(Vector4Tests, ComponentConstructorAndAddition)
{
    Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4 b(1.0f, 1.0f, 1.0f, 1.0f);

    Vector4 sum = a + b;

    EXPECT_FLOAT_EQ(sum.x, 2.0f);
    EXPECT_FLOAT_EQ(sum.y, 3.0f);
    EXPECT_FLOAT_EQ(sum.z, 4.0f);
    EXPECT_FLOAT_EQ(sum.w, 5.0f);
}

TEST(RadiansTests, ScalarConversion)
{
    EXPECT_NEAR(Radians(180.0f), 3.14159265f, 1e-4f);
    EXPECT_NEAR(Radians(90.0f), 1.57079633f, 1e-4f);
    EXPECT_FLOAT_EQ(Radians(0.0f), 0.0f);
}

TEST(RadiansTests, VectorConversionIsComponentWise)
{
    Vector3 result = Radians(Vector3(180.0f, 90.0f, 0.0f));

    EXPECT_NEAR(result.x, 3.14159265f, 1e-4f);
    EXPECT_NEAR(result.y, 1.57079633f, 1e-4f);
    EXPECT_NEAR(result.z, 0.0f, 1e-4f);
}
