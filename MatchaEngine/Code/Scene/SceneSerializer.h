#pragma once

#include "Entity.h"

#include <string>
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
// NativeScriptComponent is skipped for a similar reason: its bindings are type-erased function
// pointers with no name->Bind<T>() registry yet to serialize a binding *as* data
// (InspectorPanel::RegisterScripts() is still an empty stub).
class SceneSerializer
{
public:
    SceneSerializer() = delete;

    static void Serialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager);

    // Populates the given (already-constructed) Scene rather than returning a new one - the
    // engine's Scene is owned and swapped in one place (SceneManager), not handed out as a
    // fresh object from here.
    static void Deserialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager);

private:
    static void SerializeEntity(nlohmann::json& out, Entity entity, ResourceManager& resourceManager);

    // Creates the entity and applies every serializable component except HierarchyComponent's
    // parent link - resolving that needs every entity to already exist first, so Deserialize()
    // does it in a second pass rather than here. Returns the created entity so that pass can look
    // it up again.
    static Entity DeserializeEntity(const nlohmann::json& entityNode, Scene* scene, ResourceManager& resourceManager);
};
}  // namespace Matcha
