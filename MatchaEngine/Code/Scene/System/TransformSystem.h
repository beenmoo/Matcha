#pragma once

#include "Math/Matrix.h"
#include "Scene/Entity.h"

namespace Matcha
{
class Scene;

class TransformSystem
{
public:
    TransformSystem() = delete;

    // Recomputes every TransformComponent::worldMatrix in the scene from its local transform,
    // composed with its ancestors' (via HierarchyComponent) root-to-leaf. Assumes every entity
    // reachable through the hierarchy has a TransformComponent.
    static void Update(Scene& scene);

private:
    // Composes parentWorldMatrix with entity's local transform, writes the result into its
    // TransformComponent::worldMatrix, then recurses into its children.
    static void CascadeHierarchy(Scene& scene, Entity entity, const Matrix4& parentWorldMatrix);
};
}  // namespace Matcha
