#include "RenameEntitiesCommand.h"
#include "Scene/Component/TagComponent.h"

#include <Matcha.h>

namespace MatchaEditor
{
RenameEntitiesCommand::RenameEntitiesCommand(EngineContext& context, std::string description, std::vector<Rename> renames)
    : m_Context(context),
      m_Description(std::move(description)),
      m_Renames(std::move(renames))
{
}

void RenameEntitiesCommand::Execute()
{
    Scene& scene = m_Context.GetScene();

    for (const Rename& rename : m_Renames)
    {
        Entity entity = scene.FindEntityByUUID(rename.id);
        if (entity.IsValid())
            entity.GetComponent<TagComponent>().name = rename.newName;
    }

    scene.NotifyChanged();
}

void RenameEntitiesCommand::Undo()
{
    Scene& scene = m_Context.GetScene();

    for (const Rename& rename : m_Renames)
    {
        Entity entity = scene.FindEntityByUUID(rename.id);
        if (entity.IsValid())
            entity.GetComponent<TagComponent>().name = rename.oldName;
    }

    scene.NotifyChanged();
}
}  // namespace MatchaEditor
