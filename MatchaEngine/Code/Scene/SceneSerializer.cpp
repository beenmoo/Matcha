#include "SceneSerializer.h"
#include "ComponentRegistry.h"
#include "Scene.h"
#include "Component/HierarchyComponent.h"
#include "Component/TagComponent.h"
#include "Core/Logger.h"
#include "Graphics/ResourceManager.h"
#include "Utility/UUID.h"

#include <entt/entt.hpp>

#include <fstream>
#include <unordered_map>
#include <vector>

namespace Matcha
{
void SceneSerializer::SerializeEntity(nlohmann::json& out, Entity entity, ResourceManager& resourceManager)
{
    const TagComponent& tag = entity.GetComponent<TagComponent>();
    out["id"] = static_cast<uint64_t>(tag.id);
    out["name"] = tag.name;
    out["isActive"] = tag.isActive;

    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity parentHandle = entity.GetComponent<HierarchyComponent>().GetParent();
        if (parentHandle != entt::null)
        {
            // Guaranteed to have a TagComponent (and thus an id): every entity with a
            // HierarchyComponent link was created via Scene::CreateEntity, which always adds one.
            const TagComponent& parentTag = entity.WithHandle(parentHandle).GetComponent<TagComponent>();
            out["parent"] = static_cast<uint64_t>(parentTag.id);
        }
    }

    for (const ComponentSerializer& component : GetComponentSerializers())
        component.write(out, entity, resourceManager);
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

    for (const ComponentSerializer& component : GetComponentSerializers())
    {
        if (entityNode.contains(component.key))
            component.read(entityNode.at(component.key), entity, resourceManager);
    }

    return entity;
}

nlohmann::json SceneSerializer::SerializeEntities(const std::vector<Entity>& entities, ResourceManager& resourceManager)
{
    nlohmann::json entityNodes = nlohmann::json::array();

    for (Entity entity : entities)
    {
        nlohmann::json entityNode;
        SerializeEntity(entityNode, entity, resourceManager);
        entityNodes.push_back(std::move(entityNode));
    }

    return entityNodes;
}

std::vector<Entity> SceneSerializer::DeserializeEntities(const nlohmann::json& entityNodes, Scene* scene, ResourceManager& resourceManager)
{
    // First pass creates every entity (preserving its serialized UUID) and applies every
    // component except hierarchy - a parent link can only be resolved once the entity it points
    // at is guaranteed to already exist, which isn't true until this whole pass finishes.
    std::unordered_map<uint64_t, Entity> entitiesById;
    std::vector<Entity> created;
    created.reserve(entityNodes.size());

    for (const nlohmann::json& entityNode : entityNodes)
    {
        Entity entity = DeserializeEntity(entityNode, scene, resourceManager);
        entitiesById[static_cast<uint64_t>(entity.GetComponent<TagComponent>().id)] = entity;
        created.push_back(entity);
    }

    // Second pass: resolve each parent link (stored as the parent's UUID) now that every entity
    // in this batch exists, and rebuild the sibling-linked-list structure via the same
    // SetParent() every other reparenting operation in the engine already goes through, rather
    // than reconstructing HierarchyComponent's firstChild/prevSibling/nextSibling fields by hand
    // here. A parent id not found within this batch is looked up in the live scene instead - a
    // partial snapshot (e.g. one deleted subtree out of a whole scene) can have its root's parent
    // still alive outside the batch, and that link needs to be restored too, not just links
    // internal to the batch.
    for (const nlohmann::json& entityNode : entityNodes)
    {
        if (!entityNode.contains("parent"))
            continue;

        auto childIt = entitiesById.find(entityNode.at("id").get<uint64_t>());
        if (childIt == entitiesById.end())
            continue;

        uint64_t parentId = entityNode.at("parent").get<uint64_t>();
        auto parentIt = entitiesById.find(parentId);
        Entity parent = parentIt != entitiesById.end() ? parentIt->second : scene->FindEntityByUUID(UUID(parentId));

        if (parent.IsValid())
            SetParent(childIt->second, parent);
    }

    return created;
}

nlohmann::json SceneSerializer::RemapEntityIds(const nlohmann::json& entityNodes)
{
    std::unordered_map<uint64_t, uint64_t> idRemap;
    for (const nlohmann::json& entityNode : entityNodes)
        idRemap.emplace(entityNode.at("id").get<uint64_t>(), static_cast<uint64_t>(UUID()));

    nlohmann::json remapped = entityNodes;
    for (nlohmann::json& entityNode : remapped)
    {
        entityNode["id"] = idRemap.at(entityNode.at("id").get<uint64_t>());

        if (!entityNode.contains("parent"))
            continue;

        // Only rewrite a parent that's part of this same set - one pointing at an entity outside
        // it (e.g. the still-live original parent of a copied subtree) has to keep its original
        // id so DeserializeEntities' scene-wide fallback lookup can still find it.
        auto parentIt = idRemap.find(entityNode.at("parent").get<uint64_t>());
        if (parentIt != idRemap.end())
            entityNode["parent"] = parentIt->second;
    }

    return remapped;
}

void SceneSerializer::Serialize(const std::string& filepath, Scene* scene, ResourceManager& resourceManager)
{
    // Every entity has a TagComponent - Scene::CreateEntity always adds one - so this reaches
    // every live entity in the scene, not just tagged ones.
    std::vector<Entity> entities;
    for (entt::entity handle : scene->View<TagComponent>())
        entities.emplace_back(handle, scene);

    nlohmann::json root;
    root["entities"] = SerializeEntities(entities, resourceManager);

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
    DeserializeEntities(root.value("entities", emptyArray), scene, resourceManager);
}
}  // namespace Matcha
