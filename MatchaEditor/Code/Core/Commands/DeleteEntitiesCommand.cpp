#include "DeleteEntitiesCommand.h"
#include "Scene/Component/HierarchyComponent.h"
#include "Scene/Component/TagComponent.h"

#include <Matcha.h>

#include <entt/entt.hpp>

namespace MatchaEditor
{
namespace
{
// Mirrors the traversal shape of HierarchyComponent.h's own detail::DestroySubtree, but collects
// into a flat list instead of destroying - root first, then each child's own subtree in turn.
void CollectSubtree(Entity entity, std::vector<Entity>& out)
{
    out.push_back(entity);

    if (entity.HasComponent<HierarchyComponent>())
    {
        entt::entity childHandle = entity.GetComponent<HierarchyComponent>().firstChild;
        while (childHandle != entt::null)
        {
            Entity child = entity.WithHandle(childHandle);
            CollectSubtree(child, out);
            childHandle = child.GetComponent<HierarchyComponent>().nextSibling;
        }
    }
}
}  // namespace

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

    for (UUID id : m_RootIds)
    {
        Entity entity = scene.FindEntityByUUID(id);
        if (entity.IsValid())
            DestroyEntityRecursive(scene, entity);
    }
}

void DeleteEntitiesCommand::Undo()
{
    SceneSerializer::DeserializeEntities(m_Snapshot, &m_Context.GetScene(), m_Context.GetResourceManager());
}
}  // namespace MatchaEditor
