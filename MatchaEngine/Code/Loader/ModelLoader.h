#pragma once

#include "Graphics/RenderHandles.h"
#include "Scene/Entity.h"

#include <string>

namespace Matcha
{
class ResourceManager;
class Scene;

class ModelLoader
{
public:
    ModelLoader() = delete;

    // Imports a model file via assimp into scene, mirroring the file's node tree as a hierarchy
    // of entities (HierarchyComponent/TransformComponent per node, SetParent-linked) rooted at
    // the returned entity. Each node's referenced meshes become their own child entity with a
    // MeshComponent + MaterialComponent, all assigned the given shader. Only external (file-path)
    // textures are supported - meshes with embedded textures import without one. Returns a
    // default-constructed (invalid) Entity if the file fails to load.
    //
    // Emits scope timers (MT_PROFILE_SCOPE/FUNCTION) into whatever Profiler session is active,
    // but doesn't start or end one itself - a caller wanting a profile of the import should wrap
    // this call in its own Profiler::Get().BeginSession()/EndSession(). Not owned here since a
    // Profiler session is a single global, non-reentrant resource; a loader function shouldn't
    // assume it's the only thing using it (e.g. nested/concurrent imports, or a caller who already
    // started their own session).
    [[nodiscard]] static Entity LoadModel(Scene& scene, ResourceManager& resourceManager, ShaderHandle shader, const std::string& path);
};
}  // namespace Matcha
