#pragma once

#include "Entity.h"

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Matcha
{
class Scene;
class ResourceManager;

// Round-trips TagComponent, TransformComponent (authored fields only, not the cached
// worldMatrix), HierarchyComponent (only the parent link - child/sibling links are rebuilt from
// repeated SetParent() calls on load, not stored directly), LightComponent, and CameraComponent
// (again, authored fields only, not the cached projection matrix).
//
// MeshComponent round-trips only for a mesh created with a primitiveKind (ResourceManager::
// CreateMesh's last argument, e.g. "Cube") - that's a name Deserialize can regenerate the exact
// same geometry from procedurally. An imported model's mesh has no primitiveKind (nothing to
// regenerate it from - no source path is tracked either), so it's still skipped entirely, same as
// before: writing a numeric handle ID would be meaningless after a reload, and there's no
// geometry data or path saved to reconstruct it from instead.
//
// MaterialComponent's plain fields (albedoColor/specularStrength/shininess) always round-trip.
// Its shader/texture handles round-trip only as far as the underlying Shader/Texture object can
// report about its own origin (Shader::GetPaths()/Texture::GetPath()) - a procedural texture
// (ResourceManager::CreateTexture(width, height)) has no path and is skipped, same reasoning as
// an imported mesh above.
//
// PythonScriptComponent round-trips as a list of (moduleName, className) pairs per binding -
// Python's own import system resolves that pair to a class at load time, so unlike the old
// NativeScriptComponent (compiled function pointers with no registry to turn a binding into
// data) there's nothing else here that needs serializing. A binding with no moduleName yet
// (added via the Inspector's "Browse..." but not actually resolved) is skipped, same reasoning
// as an imported mesh's unregeneratable handle above. Not restored: whatever script directory the
// binding's moduleName resolves against - that's process-wide PythonRuntime state established by
// RegisterScriptDirectory (Sandbox.cpp at startup, or the Inspector's own picker), not per-scene
// data, so a scene loaded in a fresh session needs that directory registered again the same way.
class SceneSerializer
{
public:
    SceneSerializer() = delete;

    static void Serialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager);

    // Populates the given (already-constructed) Scene rather than returning a new one - the
    // engine's Scene is owned and swapped in one place (SceneManager), not handed out as a
    // fresh object from here.
    static void Deserialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager);

    // In-memory equivalents of Serialize/Deserialize, scoped to an explicit entity list rather
    // than a whole Scene - used by the editor's undo system to snapshot a subtree before deleting
    // it (and reconstruct it on undo) without going through a file. Shares the exact same
    // per-entity fidelity (and the same limitations - imported mesh geometry and procedural
    // textures aren't captured) as Serialize/Deserialize, since both go through the same
    // SerializeEntity/DeserializeEntity underneath. PythonScriptComponent bindings round-trip
    // through this path too (so undo/redo across a delete correctly restores them), but a
    // script's process-wide registered directory (see the class comment above) obviously isn't
    // part of what's being snapshotted here.
    //
    // DeserializeEntities resolves a "parent" id against every entity in entityNodes AND every
    // entity already live in scene - so a snapshot of a subtree whose root's parent lies outside
    // the snapshot (still alive in the scene) reattaches to that external parent correctly, not
    // just to siblings within the snapshot itself.
    static nlohmann::json SerializeEntities(const std::vector<Entity>& entities, ResourceManager& resourceManager);
    static std::vector<Entity> DeserializeEntities(const nlohmann::json& entityNodes, Scene* scene, ResourceManager& resourceManager);

    // Returns a copy of entityNodes with every entity's "id" replaced by a freshly generated UUID,
    // and every "parent" reference pointing *within* the set rewritten to match. A "parent" that
    // points outside the set is left alone, so it still resolves against the live scene the way
    // DeserializeEntities already handles.
    //
    // Needed because DeserializeEntities preserves serialized ids verbatim: deserializing a
    // snapshot whose originals are still alive (duplicating/pasting, as opposed to restoring a
    // deleted subtree) would otherwise produce UUID collisions. This is the one piece that turns
    // "restore these entities" into "make copies of these entities".
    static nlohmann::json RemapEntityIds(const nlohmann::json& entityNodes);

private:
    static void SerializeEntity(nlohmann::json& out, Entity entity, ResourceManager& resourceManager);

    // Creates the entity and applies every serializable component except HierarchyComponent's
    // parent link - resolving that needs every entity to already exist first, so Deserialize()
    // does it in a second pass rather than here. Returns the created entity so that pass can look
    // it up again.
    static Entity DeserializeEntity(const nlohmann::json& entityNode, Scene* scene, ResourceManager& resourceManager);
};
}  // namespace Matcha
