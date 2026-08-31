#pragma once

#include "Math/Matrix.h"
#include "Math/Transform.h"

namespace Matcha
{
struct TransformComponent
{
    // Authoritative, set by game/editor code. Local to the entity's parent (or to the scene, if
    // it has none).
    Transform transform;

    // Cached by TransformSystem::Update, composed with ancestors' via HierarchyComponent. Not
    // meant to be written to directly - it's overwritten every update.
    Matrix4 worldMatrix;
};
}  // namespace Matcha
