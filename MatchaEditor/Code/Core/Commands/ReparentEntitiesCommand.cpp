#include "ReparentEntitiesCommand.h"
#include "Scene/Component/HierarchyComponent.h"

#include <Matcha.h>

namespace MatchaEditor
{
ReparentEntitiesCommand::ReparentEntitiesCommand(EngineContext& context, std::string description,
                                                 std::vector<Reparent> reparents, std::optional<UUID> newParentId)
    : m_Context(context),
      m_Description(std::move(description)),
      m_Reparents(std::move(reparents)),
      m_NewParentId(newParentId)
{
}

void ReparentEntitiesCommand::Execute()
{
    Scene& scene = m_Context.GetScene();

    Entity newParent = m_NewParentId ? scene.FindEntityByUUID(*m_NewParentId) : Entity();

    for (const Reparent& reparent : m_Reparents)
    {
        Entity entity = scene.FindEntityByUUID(reparent.id);
        if (entity.IsValid())
            SetParent(entity, newParent);
    }
}

void ReparentEntitiesCommand::Undo()
{
    Scene& scene = m_Context.GetScene();

    for (const Reparent& reparent : m_Reparents)
    {
        Entity entity = scene.FindEntityByUUID(reparent.id);
        if (!entity.IsValid())
            continue;

        Entity oldParent = reparent.oldParentId ? scene.FindEntityByUUID(*reparent.oldParentId) : Entity();
        SetParent(entity, oldParent);
    }
}
}  // namespace MatchaEditor
