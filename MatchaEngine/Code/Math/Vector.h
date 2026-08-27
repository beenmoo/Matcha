#pragma once

#include <cstdint>

namespace Matcha
{
struct Vector2
{
    float x = 0.0f;
    float y = 0.0f;

    constexpr Vector2() = default;
    constexpr explicit Vector2(float scalar)
        : x(scalar),
          y(scalar)
    {
    }
    constexpr Vector2(float x, float y)
        : x(x),
          y(y)
    {
    }

    constexpr Vector2& operator+=(const Vector2& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    constexpr Vector2& operator-=(const Vector2& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    constexpr Vector2& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr bool operator==(const Vector2&) const = default;
};

[[nodiscard]] constexpr Vector2 operator+(Vector2 lhs, const Vector2& rhs)
{
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector2 operator-(Vector2 lhs, const Vector2& rhs)
{
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector2 operator*(Vector2 lhs, float rhs)
{
    lhs *= rhs;
    return lhs;
}

struct Vector2Int
{
    int32_t x = 0;
    int32_t y = 0;

    constexpr Vector2Int() = default;
    constexpr explicit Vector2Int(int32_t scalar)
        : x(scalar),
          y(scalar)
    {
    }
    constexpr Vector2Int(int32_t x, int32_t y)
        : x(x),
          y(y)
    {
    }

    constexpr bool operator==(const Vector2Int&) const = default;
};

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vector3() = default;
    constexpr explicit Vector3(float scalar)
        : x(scalar),
          y(scalar),
          z(scalar)
    {
    }
    constexpr Vector3(float x, float y, float z)
        : x(x),
          y(y),
          z(z)
    {
    }

    constexpr Vector3& operator+=(const Vector3& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    constexpr Vector3& operator-=(const Vector3& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }

    constexpr Vector3& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    constexpr bool operator==(const Vector3&) const = default;
};

[[nodiscard]] constexpr Vector3 operator+(Vector3 lhs, const Vector3& rhs)
{
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector3 operator-(Vector3 lhs, const Vector3& rhs)
{
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector3 operator*(Vector3 lhs, float rhs)
{
    lhs *= rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector3 operator-(const Vector3& v)
{
    return Vector3(-v.x, -v.y, -v.z);
}

struct Vector3Int
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;

    constexpr Vector3Int() = default;
    constexpr explicit Vector3Int(int32_t scalar)
        : x(scalar),
          y(scalar),
          z(scalar)
    {
    }
    constexpr Vector3Int(int32_t x, int32_t y, int32_t z)
        : x(x),
          y(y),
          z(z)
    {
    }

    constexpr bool operator==(const Vector3Int&) const = default;
};

struct Vector4
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;

    constexpr Vector4() = default;
    constexpr explicit Vector4(float scalar)
        : x(scalar),
          y(scalar),
          z(scalar),
          w(scalar)
    {
    }
    constexpr Vector4(float x, float y, float z, float w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    constexpr Vector4& operator+=(const Vector4& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        w += rhs.w;
        return *this;
    }

    constexpr Vector4& operator-=(const Vector4& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        w -= rhs.w;
        return *this;
    }

    constexpr Vector4& operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }

    constexpr bool operator==(const Vector4&) const = default;
};

[[nodiscard]] constexpr Vector4 operator+(Vector4 lhs, const Vector4& rhs)
{
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector4 operator-(Vector4 lhs, const Vector4& rhs)
{
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr Vector4 operator*(Vector4 lhs, float rhs)
{
    lhs *= rhs;
    return lhs;
}

struct Vector4Int
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;
    int32_t w = 0;

    constexpr Vector4Int() = default;
    constexpr explicit Vector4Int(int32_t scalar)
        : x(scalar),
          y(scalar),
          z(scalar),
          w(scalar)
    {
    }
    constexpr Vector4Int(int32_t x, int32_t y, int32_t z, int32_t w)
        : x(x),
          y(y),
          z(z),
          w(w)
    {
    }

    constexpr bool operator==(const Vector4Int&) const = default;
};

[[nodiscard]] constexpr float Radians(float degrees)
{
    return degrees * 0.01745329251994329577f;
}

[[nodiscard]] constexpr Vector3 Radians(const Vector3& degrees)
{
    return Vector3(Radians(degrees.x), Radians(degrees.y), Radians(degrees.z));
}
}  // namespace Matcha
