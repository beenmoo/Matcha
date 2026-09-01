#include "CreateEntityCommand.h"

#include <Matcha.h>

namespace MatchaEditor
{
CreateEntityCommand::CreateEntityCommand(EngineContext& context, std::string description, std::string name,
                                         std::optional<UUID> parentId, std::function<void(Entity)> populate)
    : m_Context(context),
      m_Description(std::move(description)),
      m_Name(std::move(name)),
      m_ParentId(parentId),
      m_Populate(std::move(populate))
{
}

void CreateEntityCommand::Execute()
{
    Scene& scene = m_Context.GetScene();

    Entity entity = scene.CreateEntity(m_Id, m_Name);

    if (m_Populate)
        m_Populate(entity);

    if (m_ParentId)
    {
        Entity parent = scene.FindEntityByUUID(*m_ParentId);
        if (parent.IsValid())
            SetParent(entity, parent);
    }
}

void CreateEntityCommand::Undo()
{
    Scene& scene = m_Context.GetScene();

    Entity entity = scene.FindEntityByUUID(m_Id);
    if (entity.IsValid())
        DestroyEntityRecursive(scene, entity);
}
}  // namespace MatchaEditor
