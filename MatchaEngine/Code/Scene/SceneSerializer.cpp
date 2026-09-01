#include "SceneSerializer.h"
#include "Scene.h"
#include "Component/CameraComponent.h"
#include "Component/HierarchyComponent.h"
#include "Component/LightComponent.h"
#include "Component/MaterialComponent.h"
#include "Component/MeshComponent.h"
#include "Component/TagComponent.h"
#include "Component/TransformComponent.h"
#include "Core/Logger.h"
#include "Graphics/Primitives.h"
#include "Graphics/ResourceManager.h"
#include "Graphics/ShaderDataType.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/Vector.h"
#include "Utility/UUID.h"

#include <entt/entt.hpp>

#include <fstream>
#include <unordered_map>
#include <vector>

namespace Matcha
{
namespace
{
nlohmann::json ToJson(const Vector3& v)
{
    return {v.x, v.y, v.z};
}

Vector3 Vector3FromJson(const nlohmann::json& j)
{
    return Vector3(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>());
}

nlohmann::json ToJson(const Quaternion& q)
{
    return {q.x, q.y, q.z, q.w};
}

Quaternion QuaternionFromJson(const nlohmann::json& j)
{
    return Quaternion(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>());
}

nlohmann::json ToJson(const Vector4& v)
{
    return {v.x, v.y, v.z, v.w};
}

Vector4 Vector4FromJson(const nlohmann::json& j)
{
    return Vector4(j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>(), j.at(3).get<float>());
}

// The only kind currently registered anywhere (SceneHierarchyWidget::CreateCubeEntity) - add a
// branch here alongside whatever new call passes a new primitiveKind to
// ResourceManager::CreateMesh, so Deserialize can regenerate it too.
MeshHandle CreatePrimitiveMesh(ResourceManager& resourceManager, const std::string& kind)
{
    if (kind == "Cube")
    {
        CubePrimitive cube;
        return resourceManager.CreateMesh(cube.vertices, {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2},
                                          cube.indices, "Cube");
    }

    MT_CORE_WARN("SceneSerializer: unknown mesh primitive kind \"{}\" - mesh not restored.", kind);
    return MeshHandle();
}
}  // namespace

void SceneSerializer::SerializeEntity(nlohmann::json& out, Entity entity, ResourceManager& resourceManager)
{
    const TagComponent& tag = entity.GetComponent<TagComponent>();
    out["id"] = static_cast<uint64_t>(tag.id);
    out["name"] = tag.name;
    out["isActive"] = tag.isActive;

    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity parentHandle = entity.GetComponent<HierarchyComponent>().parent;
        if (parentHandle != entt::null)
        {
            // Guaranteed to have a TagComponent (and thus an id): every entity with a
            // HierarchyComponent link was created via Scene::CreateEntity, which always adds one.
            const TagComponent& parentTag = entity.WithHandle(parentHandle).GetComponent<TagComponent>();
            out["parent"] = static_cast<uint64_t>(parentTag.id);
        }
    }

    if (entity.HasComponent<TransformComponent>())
    {
        const Transform& transform = entity.GetComponent<TransformComponent>().transform;
        out["transform"] = {
            {"position", ToJson(transform.GetPosition())},
            {"rotation", ToJson(transform.GetRotation())},
            {"scale", ToJson(transform.GetScale())},
        };
    }

    if (entity.HasComponent<LightComponent>())
    {
        const LightComponent& light = entity.GetComponent<LightComponent>();
        out["light"] = {
            {"type", static_cast<int>(light.type)},
            {"color", ToJson(light.color)},
            {"intensity", light.intensity},
            {"range", light.range},
            {"innerConeAngle", light.innerConeAngle},
            {"outerConeAngle", light.outerConeAngle},
            {"ambientStrength", light.ambientStrength},
            {"ambientColor", ToJson(light.ambientColor)},
            {"castShadows", light.castShadows},
        };
    }

    if (entity.HasComponent<CameraComponent>())
    {
        const CameraComponent& camera = entity.GetComponent<CameraComponent>();
        out["camera"] = {
            {"projectionType", static_cast<int>(camera.projectionType)},
            {"perspectiveFOV", camera.perspectiveFOV},
            {"perspectiveNear", camera.perspectiveNear},
            {"perspectiveFar", camera.perspectiveFar},
            {"orthographicSize", camera.orthographicSize},
            {"orthographicNear", camera.orthographicNear},
            {"orthographicFar", camera.orthographicFar},
            {"aspectRatio", camera.aspectRatio},
            {"primary", camera.primary},
            {"fixedAspectRatio", camera.fixedAspectRatio},
        };
    }

    if (entity.HasComponent<MeshComponent>())
    {
        MeshHandle mesh = entity.GetComponent<MeshComponent>().mesh;
        if (mesh.IsValid())
        {
            std::string primitiveKind = resourceManager.GetMeshPrimitiveKind(mesh);
            if (!primitiveKind.empty())
                out["mesh"] = {{"primitive", primitiveKind}};
            // else: an imported mesh, not a regeneratable primitive - see SceneSerializer.h.
        }
    }

    if (entity.HasComponent<MaterialComponent>())
    {
        const MaterialComponent& material = entity.GetComponent<MaterialComponent>();

        nlohmann::json materialNode = {
            {"albedoColor", ToJson(material.albedoColor)},
            {"specularStrength", material.specularStrength},
            {"shininess", material.shininess},
        };

        // shader/texture are handles, not data - only serializable via what the underlying
        // Shader/Texture object can report about its own origin (Shader::GetPaths()/Texture::
        // GetPath(), see the DLL-copy-adjacent Texture/Shader work this stems from). A procedural
        // texture (CreateTexture(width, height)) has an empty path and is skipped the same way an
        // imported mesh is above - nothing to resolve it back from on load.
        if (material.shader.IsValid())
        {
            if (Shader* shader = resourceManager.GetShader(material.shader); shader && !shader->GetPaths().empty())
                materialNode["shaderPaths"] = shader->GetPaths();
        }

        if (material.texture.IsValid())
        {
            if (Texture* texture = resourceManager.GetTexture(material.texture); texture && !texture->GetPath().empty())
                materialNode["texturePath"] = texture->GetPath();
        }

        out["material"] = std::move(materialNode);
    }
}

Entity SceneSerializer::DeserializeEntity(const nlohmann::json& entityNode, Scene* scene, ResourceManager& resourceManager)
{
    UUID id(entityNode.at("id").get<uint64_t>());
    std::string name = entityNode.value("name", std::string());

    // CreateEntity(UUID, name) already adds TagComponent (with this id/name) and
    // TransformComponent - GetComponent below, not AddComponent, for both.
    Entity entity = scene->CreateEntity(id, name);

    if (entityNode.contains("isActive"))
        entity.GetComponent<TagComponent>().isActive = entityNode.at("isActive").get<bool>();

    if (entityNode.contains("transform"))
    {
        const nlohmann::json& t = entityNode.at("transform");
        Transform& transform = entity.GetComponent<TransformComponent>().transform;
        transform.SetPosition(Vector3FromJson(t.at("position")));
        transform.SetRotation(QuaternionFromJson(t.at("rotation")));
        transform.SetScale(Vector3FromJson(t.at("scale")));
    }

    if (entityNode.contains("light"))
    {
        const nlohmann::json& l = entityNode.at("light");
        LightComponent& light = entity.AddComponent<LightComponent>();
        light.type = static_cast<LightType>(l.at("type").get<int>());
        light.color = Vector3FromJson(l.at("color"));
        light.intensity = l.at("intensity").get<float>();
        light.range = l.at("range").get<float>();
        light.innerConeAngle = l.at("innerConeAngle").get<float>();
        light.outerConeAngle = l.at("outerConeAngle").get<float>();
        light.ambientStrength = l.at("ambientStrength").get<float>();
        light.ambientColor = Vector3FromJson(l.at("ambientColor"));
        light.castShadows = l.at("castShadows").get<bool>();
    }

    if (entityNode.contains("camera"))
    {
        const nlohmann::json& c = entityNode.at("camera");
        CameraComponent& camera = entity.AddComponent<CameraComponent>();
        camera.projectionType = static_cast<CameraProjectionType>(c.at("projectionType").get<int>());
        camera.perspectiveFOV = c.at("perspectiveFOV").get<float>();
        camera.perspectiveNear = c.at("perspectiveNear").get<float>();
        camera.perspectiveFar = c.at("perspectiveFar").get<float>();
        camera.orthographicSize = c.at("orthographicSize").get<float>();
        camera.orthographicNear = c.at("orthographicNear").get<float>();
        camera.orthographicFar = c.at("orthographicFar").get<float>();
        camera.aspectRatio = c.at("aspectRatio").get<float>();
        camera.primary = c.at("primary").get<bool>();
        camera.fixedAspectRatio = c.at("fixedAspectRatio").get<bool>();
    }

    if (entityNode.contains("mesh"))
    {
        std::string primitiveKind = entityNode.at("mesh").at("primitive").get<std::string>();
        MeshHandle mesh = CreatePrimitiveMesh(resourceManager, primitiveKind);
        if (mesh.IsValid())
            entity.AddComponent<MeshComponent>().mesh = mesh;
    }

    if (entityNode.contains("material"))
    {
        const nlohmann::json& m = entityNode.at("material");
        MaterialComponent& material = entity.AddComponent<MaterialComponent>();
        material.albedoColor = Vector4FromJson(m.at("albedoColor"));
        material.specularStrength = m.at("specularStrength").get<float>();
        material.shininess = m.at("shininess").get<float>();

        if (m.contains("shaderPaths"))
        {
            std::vector<std::string> shaderPaths = m.at("shaderPaths").get<std::vector<std::string>>();
            if (!shaderPaths.empty())
                material.shader = resourceManager.CreateShader("Material", shaderPaths);
        }

        if (m.contains("texturePath"))
            material.texture = resourceManager.CreateTexture(m.at("texturePath").get<std::string>());
    }

    return entity;
}

void SceneSerializer::Serialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager)
{
    nlohmann::json entities = nlohmann::json::array();

    // Every entity has a TagComponent - Scene::CreateEntity always adds one - so this reaches
    // every live entity in the scene, not just tagged ones.
    for (entt::entity handle : scene->View<TagComponent>())
    {
        nlohmann::json entityNode;
        SerializeEntity(entityNode, Entity(handle, scene), resourceManager);
        entities.push_back(std::move(entityNode));
    }

    nlohmann::json root;
    root["entities"] = std::move(entities);

    std::ofstream file(filepath);
    if (!file.is_open())
    {
        MT_CORE_ERROR("SceneSerializer::Serialize - failed to open \"{}\" for writing.", filepath);
        return;
    }

    file << root.dump(4);
}

void SceneSerializer::Deserialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        MT_CORE_ERROR("SceneSerializer::Deserialize - failed to open \"{}\" for reading.", filepath);
        return;
    }

    nlohmann::json root;
    try
    {
        file >> root;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        MT_CORE_ERROR("SceneSerializer::Deserialize - \"{}\" is not valid JSON: {}", filepath, e.what());
        return;
    }

    const nlohmann::json emptyArray = nlohmann::json::array();
    const nlohmann::json& entityNodes = root.value("entities", emptyArray);

    // First pass creates every entity (preserving its serialized UUID) and applies every
    // component except hierarchy - a parent link can only be resolved once the entity it points
    // at is guaranteed to already exist, which isn't true until this whole pass finishes.
    std::unordered_map<uint64_t, Entity> entitiesById;
    for (const nlohmann::json& entityNode : entityNodes)
    {
        Entity entity = DeserializeEntity(entityNode, scene, resourceManager);
        entitiesById[static_cast<uint64_t>(entity.GetComponent<TagComponent>().id)] = entity;
    }

    // Second pass: resolve each parent link (stored as the parent's UUID) now that every entity
    // exists, and rebuild the sibling-linked-list structure via the same SetParent() every other
    // reparenting operation in the engine already goes through, rather than reconstructing
    // HierarchyComponent's firstChild/prevSibling/nextSibling fields by hand here.
    for (const nlohmann::json& entityNode : entityNodes)
    {
        if (!entityNode.contains("parent"))
            continue;

        auto childIt = entitiesById.find(entityNode.at("id").get<uint64_t>());
        auto parentIt = entitiesById.find(entityNode.at("parent").get<uint64_t>());

        if (childIt != entitiesById.end() && parentIt != entitiesById.end())
            SetParent(childIt->second, parentIt->second);
    }
}
}  // namespace Matcha
