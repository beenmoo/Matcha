#include "DeleteEntitiesCommand.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"

#include <Matcha.h>

#include <entt/entt.hpp>

namespace MatchaEditor
{
DeleteEntitiesCommand::DeleteEntitiesCommand(EngineContext& context, std::string description,
                                             const std::vector<Entity>& subtreeRoots)
    : m_Context(context),
      m_Description(std::move(description))
{
    std::vector<Entity> allEntities;
    for (Entity root : subtreeRoots)
    {
        m_RootIds.push_back(root.GetComponent<TagComponent>().id);
        CollectSubtree(root, allEntities);
    }

    m_Snapshot = SceneSerializer::SerializeEntities(allEntities, context.GetResourceManager());
}

void DeleteEntitiesCommand::Execute()
{
    Scene& scene = m_Context.GetScene();
    Scene::ChangeBatch batch(scene);

    for (UUID id : m_RootIds)
    {
        Entity entity = scene.FindEntityByUUID(id);
        if (entity.IsValid())
            DestroyEntityRecursive(scene, entity);
    }
}

void DeleteEntitiesCommand::Undo()
{
    Scene::ChangeBatch batch(m_Context.GetScene());

    SceneSerializer::DeserializeEntities(m_Snapshot, &m_Context.GetScene(), m_Context.GetResourceManager());
}
}  // namespace MatchaEditor
