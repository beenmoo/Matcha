#pragma once

#include "Math/Vector.h"

namespace Matcha
{
enum class LightType
{
    Directional,
    Point,
    Spot
};

struct LightComponent
{
    LightType type = LightType::Directional;

    Vector3 color = Vector3(1.0f);
    float intensity = 1.0f;

    float range = 10.0f;           // For point and spot lights
    float innerConeAngle = 15.0f;  // For spot lights
    float outerConeAngle = 30.0f;  // For spot lights

    // Not physically part of this light - it's a flat scene-wide fill term, attached here only
    // because there's nowhere else to author it from yet (no scene-wide lighting settings exist).
    float ambientStrength = 0.1f;
    Vector3 ambientColor = Vector3(1.0f);

    bool castShadows = false;
};
}  // namespace Matcha