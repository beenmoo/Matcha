#include "ModelLoader.h"
#include "Core/Logger.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/ShaderDataType.h"
#include "Math/Quaternion.h"
#include "Math/Vector.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/MaterialComponent.h"
#include "Scene/Component/MeshComponent.h"
#include "Scene/Component/TransformComponent.h"
#include "Scene/Scene.h"
#include "Utility/Profiler.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <filesystem>
#include <vector>

namespace Matcha
{
namespace
{
void ApplyNodeTransform(Entity entity, const aiMatrix4x4& matrix)
{
    MT_PROFILE_FUNCTION();

    aiVector3D scale;
    aiQuaternion rotation;
    aiVector3D position;
    matrix.Decompose(scale, rotation, position);

    Transform& transform = entity.AddComponent<TransformComponent>().transform;
    transform.SetPosition(position.x, position.y, position.z);
    transform.SetRotation(Quaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    transform.SetScale(scale.x, scale.y, scale.z);
}

// Position (3) + normal (3) + texcoord (2) per vertex, matching StandardMesh.vert. aiMesh's
// normal/texcoord channels are optional and vary per mesh, so missing ones are zero-filled here
// to keep every imported mesh on this one fixed layout.
MeshHandle CreateMeshFromAiMesh(ResourceManager& resourceManager, const aiMesh* mesh)
{
    MT_PROFILE_FUNCTION();

    std::vector<float> vertices;
    vertices.reserve(static_cast<size_t>(mesh->mNumVertices) * 8);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
    {
        const aiVector3D& position = mesh->mVertices[i];
        vertices.push_back(position.x);
        vertices.push_back(position.y);
        vertices.push_back(position.z);

        if (mesh->HasNormals())
        {
            const aiVector3D& normal = mesh->mNormals[i];
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        if (mesh->HasTextureCoords(0))
        {
            const aiVector3D& texCoord = mesh->mTextureCoords[0][i];
            vertices.push_back(texCoord.x);
            vertices.push_back(texCoord.y);
        }
        else
        {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    std::vector<uint32_t> indices;
    indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
    {
        const aiFace& face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    MeshHandle handle;
    {
        MT_PROFILE_SCOPE("ResourceManager::CreateMesh (GL upload)");
        handle = resourceManager.CreateMesh(vertices, {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2}, indices);
    }

    return handle;
}

MaterialComponent CreateMaterialFromAiMaterial(ResourceManager& resourceManager,
                                               const aiScene* aiScene,
                                               unsigned int materialIndex,
                                               const std::filesystem::path& modelDirectory,
                                               ShaderHandle shader)
{
    MT_PROFILE_FUNCTION();

    MaterialComponent material;
    material.shader = shader;

    const aiMaterial* aiMat = aiScene->mMaterials[materialIndex];

    aiColor4D color;

    // Try the PBR base-color key first (what glTF-sourced materials use), fall back to the
    // legacy diffuse key most other formats use.
    if (aiMat->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS || aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        material.albedoColor = Vector4(color.r, color.g, color.b, color.a);

    if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString texturePath;

        if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
        {
            std::string path = texturePath.C_Str();

            // Embedded textures are referenced as "*0", "*1", etc, not a file path - not
            // supported yet, so skip rather than trying (and failing) to open one as a file.
            if (!path.empty() && path[0] != '*')
            {
                MT_PROFILE_SCOPE("ResourceManager::CreateTexture (decode+upload)");
                material.texture = resourceManager.CreateTexture((modelDirectory / path).string());
            }
        }
    }

    return material;
}

Entity ImportNode(Scene& scene,
                  ResourceManager& resourceManager,
                  const aiScene* aiScene,
                  const aiNode* node,
                  Entity parent,
                  ShaderHandle shader,
                  const std::filesystem::path& modelDirectory)
{
    MT_PROFILE_FUNCTION();

    Entity nodeEntity = scene.CreateEntity();
    ApplyNodeTransform(nodeEntity, node->mTransformation);
    SetParent(nodeEntity, parent);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = aiScene->mMeshes[node->mMeshes[i]];

        Entity meshEntity = scene.CreateEntity();
        meshEntity.AddComponent<TransformComponent>();
        SetParent(meshEntity, nodeEntity);

        meshEntity.AddComponent<MeshComponent>().mesh = CreateMeshFromAiMesh(resourceManager, mesh);
        meshEntity.AddComponent<MaterialComponent>() =
            CreateMaterialFromAiMaterial(resourceManager, aiScene, mesh->mMaterialIndex, modelDirectory, shader);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
        ImportNode(scene, resourceManager, aiScene, node->mChildren[i], nodeEntity, shader, modelDirectory);

    return nodeEntity;
}
}  // namespace

Entity ModelLoader::LoadModel(Scene& scene,
                              ResourceManager& resourceManager,
                              ShaderHandle shader,
                              const std::string& path)
{
    Profiler::Get().BeginSession("ModelLoad", "profile_results.json");

    Assimp::Importer importer;

    const aiScene* aiScene;
    {
        MT_PROFILE_SCOPE("Assimp::Importer::ReadFile");
        aiScene = importer.ReadFile(
            path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);
    }

    if (!aiScene || (aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !aiScene->mRootNode)
    {
        MT_CORE_ERROR("Failed to load model '{0}': {1}", path, importer.GetErrorString());
        Profiler::Get().EndSession();
        return Entity();
    }

    std::filesystem::path modelDirectory = std::filesystem::path(path).parent_path();

    Entity root = ImportNode(scene, resourceManager, aiScene, aiScene->mRootNode, Entity(), shader, modelDirectory);

    // Ends the session (and flushes the JSON file) before returning, so every scope timer above
    // has already been destroyed and written its entry by this point - see profile_results.json,
    // open it in chrome://tracing or https://ui.perfetto.dev.
    Profiler::Get().EndSession();

    return root;
}
}  // namespace Matcha
