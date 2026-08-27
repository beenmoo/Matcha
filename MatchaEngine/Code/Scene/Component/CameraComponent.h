#pragma once

#include "Math/Matrix.h"

namespace Matcha
{
enum class CameraProjectionType
{
    Perspective,
    Orthographic
};

struct CameraComponent
{
    CameraProjectionType projectionType = CameraProjectionType::Perspective;

    // Perspective projection parameters
    float perspectiveFOV = 45.0f;
    float perspectiveNear = 0.1f;
    float perspectiveFar = 1000.0f;

    // Orthographic projection parameters
    float orthographicSize = 10.0f;
    float orthographicNear = -1.0f;
    float orthographicFar = 1.0f;

    float aspectRatio = 1.0f;

    Matrix4 projection;

    bool primary = true;  // TODO: Allow multiple cameras per scene and select which one to use for rendering
    bool fixedAspectRatio = false;  // If true, the viewport size will not affect the aspect ratio of the camera
};
}  // namespace Matcha
