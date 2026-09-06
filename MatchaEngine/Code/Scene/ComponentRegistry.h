#pragma once

#include "Entity.h"

#include <nlohmann/json.hpp>

#include <functional>
#include <string>
#include <vector>

namespace Matcha
{
class ResourceManager;

// One serializable component type: the JSON key it lives under, plus the paired write/read for it.
// Keeping both halves in a single entry is the whole point - they were previously two independent
// if-chains in SceneSerializer (one in SerializeEntity, one in DeserializeEntity) that could drift
// apart silently, with nothing to catch a field written but never read back.
struct ComponentSerializer
{
    std::string key;

    // Writes the component into entityNode[key] - a no-op if `entity` doesn't have it, or if it
    // isn't representable (e.g. a mesh with no regeneratable primitiveKind, a procedural texture
    // with no source path - see SceneSerializer.h for that fidelity ceiling).
    std::function<void(nlohmann::json& entityNode, Entity entity, ResourceManager& resourceManager)> write;

    // Adds the component to `entity` and applies componentNode to it. Only called when the key is
    // actually present.
    std::function<void(const nlohmann::json& componentNode, Entity entity, ResourceManager& resourceManager)> read;
};

// Every component SceneSerializer round-trips, in one place - so adding a component type means
// adding one entry here rather than editing two separate chains.
//
// Two components are deliberately absent, because their handling can't be expressed as "one key,
// read and written independently per entity":
//  - TagComponent: its id/name/isActive live at the entity node's root, not under a key, and the
//    id has to exist before anything else (it's what CreateEntity is called with).
//  - HierarchyComponent: its parent link is a reference to another entity, resolvable only in a
//    second pass once every entity in the batch exists.
// Both stay hand-written in SceneSerializer.
[[nodiscard]] const std::vector<ComponentSerializer>& GetComponentSerializers();
}  // namespace Matcha
