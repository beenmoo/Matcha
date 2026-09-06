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

    // Also cached by TransformSystem::Update: true when this entity and every ancestor above it
    // has TagComponent::isActive set. Written during the same root-to-leaf cascade that computes
    // worldMatrix, which already visits the hierarchy in the order this needs - so the systems
    // that iterate flat views (Render/Light/Camera/Script) read this instead of each walking the
    // parent chain per entity per frame via IsActiveInHierarchy().
    //
    // Only meaningful after TransformSystem has run for the current frame. That holds for the
    // render-side systems (Transform runs first among m_RenderSystems), but *not* for ScriptSystem,
    // which runs earlier in Application::Update() and so deliberately keeps using the uncached
    // IsActiveInHierarchy() walk - see the comment there.
    //
    // Defaults to true so an entity is visible before the first cascade has run, rather than
    // flickering out for a frame.
    bool activeInHierarchy = true;
};
}  // namespace Matcha
