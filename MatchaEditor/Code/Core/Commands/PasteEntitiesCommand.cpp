#include "PasteEntitiesCommand.h"
#include "Scene/Component/HierarchyComponent.h"

#include <Matcha.h>

#include <unordered_set>

namespace MatchaEditor
{
PasteEntitiesCommand::PasteEntitiesCommand(SceneManager& sceneManager, ResourceManager& resourceManager,
                                           std::string description, const nlohmann::json& clipboard,
                                           std::optional<UUID> parentId)
    : m_SceneManager(sceneManager),
      m_ResourceManager(resourceManager),
      m_Description(std::move(description))
{
    if (!clipboard.is_array() || clipboard.empty())
        return;

    m_Nodes = SceneSerializer::RemapEntityIds(clipboard);

    std::unordered_set<uint64_t> pastedIds;
    for (const nlohmann::json& entityNode : m_Nodes)
        pastedIds.insert(entityNode.at("id").get<uint64_t>());

    for (nlohmann::json& entityNode : m_Nodes)
    {
        // A node whose parent is outside this set is a root *of the paste* - it's the one that
        // gets re-homed under the paste target. Nodes parented within the set keep the internal
        // structure RemapEntityIds already rewired, so a copied subtree stays intact.
        bool isRoot = !entityNode.contains("parent") || !pastedIds.contains(entityNode.at("parent").get<uint64_t>());
        if (!isRoot)
            continue;

        m_RootIds.emplace_back(entityNode.at("id").get<uint64_t>());

        // Rewriting "parent" here rather than calling SetParent after deserializing: DeserializeEntities
        // already resolves an out-of-set parent id against the live scene, so the paste target just
        // slots into the same mechanism.
        if (parentId)
            entityNode["parent"] = static_cast<uint64_t>(*parentId);
        else
            entityNode.erase("parent");
    }
}

void PasteEntitiesCommand::Execute()
{
    if (m_Nodes.empty())
        return;

    Scene::ChangeBatch batch(m_SceneManager.GetScene());

    SceneSerializer::DeserializeEntities(m_Nodes, &m_SceneManager.GetScene(), m_ResourceManager);
}

void PasteEntitiesCommand::Undo()
{
    Scene& scene = m_SceneManager.GetScene();
    Scene::ChangeBatch batch(scene);

    for (UUID id : m_RootIds)
    {
        Entity entity = scene.FindEntityByUUID(id);
        if (entity.IsValid())
            DestroyEntityRecursive(scene, entity);
    }
}
}  // namespace MatchaEditor
