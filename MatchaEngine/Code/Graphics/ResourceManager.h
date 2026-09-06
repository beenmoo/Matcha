#pragma once

#include "Graphics/ShaderDataType.h"
#include "Mesh.h"
#include "RenderHandles.h"
#include "Shader.h"
#include "ShaderHotReloader.h"
#include "Texture.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Matcha
{
class RendererAPI;

class ResourceManager
{
public:
    // Takes the backend it creates resources through, rather than reaching for a global "currently
    // active" one: the dependency is visible in the signature, two managers can run against
    // different backends (a real one and a headless test double - see Tests/Code/NullRendererAPI.h),
    // and there's no initialization order to get wrong.
    explicit ResourceManager(RendererAPI& rendererAPI);

    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths);

    // For a runtime-sized path list (e.g. SceneSerializer deserializing Shader::GetPaths() back
    // into a real handle) - the initializer_list overload above can't accept that, only a
    // compile-time {"a", "b"} literal. A braced literal binds to std::initializer_list but never to
    // std::span, which is why both overloads exist rather than one span-based one; they share a
    // single implementation (CreateShaderFromPaths) so only the parameter type differs.
    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::vector<std::string>& paths);

    void DestroyShader(ShaderHandle handle);

    // Reloads any shaders whose source files changed on disk since the last call.
    // Intended to be called once per frame.
    void ReloadModifiedShaders();

    [[nodiscard]] TextureHandle CreateTexture(std::string_view path);
    [[nodiscard]] TextureHandle CreateTexture(uint32_t width, uint32_t height);
    void DestroyTexture(TextureHandle handle);

    // primitiveKind is empty for the common case (an imported model's mesh, with no way to
    // regenerate it from scratch yet - see SceneSerializer.h) - pass a name (e.g. "Cube") only
    // when the caller can rebuild the exact same geometry procedurally given just that name, so
    // SceneSerializer can round-trip a MeshComponent by regenerating it instead of needing to
    // save geometry data or resolve a source file.
    [[nodiscard]] MeshHandle CreateMesh(std::span<const float> vertices,
                                        std::initializer_list<ShaderDataType> layout,
                                        std::span<const uint32_t> indices,
                                        std::string_view primitiveKind = {});
    void DestroyMesh(MeshHandle handle);

    // The one place procedural primitive geometry ("Cube", and whatever joins it) is built, shared
    // by everything that needs one: the editor's Create Cube action and SceneSerializer restoring a
    // MeshComponent by primitiveKind. Both previously built their own cube from CubePrimitive with
    // their own copy of the vertex layout.
    //
    // Cached per kind, so N cube entities share one GPU mesh instead of uploading N identical ones
    // (loading a scene of 100 cubes used to create 100 separate meshes). Returns an invalid handle
    // for an unrecognized kind.
    [[nodiscard]] MeshHandle GetOrCreatePrimitiveMesh(std::string_view kind);

    // Non-owning, and only valid until the next Destroy*/reload call for that handle - the
    // unique_ptr in the map below owns the object, the caller just borrows it. Holding one for the
    // span of a function is the intended use (Renderer::Flush keeps a Shader* across a run of draws
    // sharing that handle, so Bind() only fires on a change), but anything outliving the call
    // should store the handle and re-resolve through it, the way MaterialComponent/MeshComponent
    // already do: IDs are never recycled, so re-resolving a destroyed handle returns nullptr,
    // whereas a pointer held across its Destroy* silently dangles.
    [[nodiscard]] Shader* GetShader(ShaderHandle handle);
    [[nodiscard]] Texture* GetTexture(TextureHandle handle);
    [[nodiscard]] Mesh* GetMesh(MeshHandle handle);

    // Empty if this handle wasn't created with a primitiveKind (imported meshes, or an invalid
    // handle) - see CreateMesh's own comment.
    [[nodiscard]] std::string GetMeshPrimitiveKind(MeshHandle handle) const;

private:
    // Shared by both CreateShader overloads - see their comment for why the parameter type is the
    // only thing that differs between them.
    [[nodiscard]] ShaderHandle CreateShaderFromPaths(std::string_view name, std::span<const std::string> paths);

private:
    RendererAPI& m_RendererAPI;

    uint32_t m_NextShaderID = 1;
    uint32_t m_NextTextureID = 1;
    uint32_t m_NextMeshID = 1;

    std::unordered_map<uint32_t, std::unique_ptr<Shader>> m_Shaders;
    std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<uint32_t, std::unique_ptr<Mesh>> m_Meshes;

    // CreateTexture(path) dedupes by normalized path: a texture used by many meshes (materials
    // sharing the same source image, as glTF imports commonly do) is decoded and uploaded once,
    // not once per caller. Only covers the path-based overload - CreateTexture(width, height)
    // (procedural textures) has no natural key to dedupe on.
    std::unordered_map<std::string, TextureHandle> m_TexturePathToHandle;

    // Only ever holds entries for handles created with a non-empty primitiveKind - see
    // CreateMesh's comment.
    std::unordered_map<uint32_t, std::string> m_MeshPrimitiveKinds;

    // The reverse of m_MeshPrimitiveKinds: the one shared mesh per primitive kind, so repeated
    // requests for "Cube" all resolve to the same handle. See GetOrCreatePrimitiveMesh.
    std::unordered_map<std::string, MeshHandle> m_PrimitiveMeshes;

    ShaderHotReloader m_ShaderHotReloader;
};
}  // namespace Matcha
