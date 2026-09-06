#pragma once

#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"

#include <gtest/gtest.h>

namespace MatchaTests
{
constexpr float kEpsilon = 1e-4f;

inline void ExpectVectorNear(const Matcha::Vector3& actual, const Matcha::Vector3& expected, float epsilon = kEpsilon)
{
    EXPECT_NEAR(actual.x, expected.x, epsilon);
    EXPECT_NEAR(actual.y, expected.y, epsilon);
    EXPECT_NEAR(actual.z, expected.z, epsilon);
}

inline void ExpectQuaternionNear(const Matcha::Quaternion& actual, const Matcha::Quaternion& expected, float epsilon = kEpsilon)
{
    EXPECT_NEAR(actual.x, expected.x, epsilon);
    EXPECT_NEAR(actual.y, expected.y, epsilon);
    EXPECT_NEAR(actual.z, expected.z, epsilon);
    EXPECT_NEAR(actual.w, expected.w, epsilon);
}

inline void ExpectMatrixNear(const Matcha::Matrix4& actual, const Matcha::Matrix4& expected, float epsilon = kEpsilon)
{
    const float* a = actual.GetData();
    const float* b = expected.GetData();

    for (int i = 0; i < 16; ++i)
        EXPECT_NEAR(a[i], b[i], epsilon) << "at index " << i;
}
}  // namespace MatchaTests
