#pragma once

#include "Core/Command.h"

#include <Matcha.h>
#include <Scene/Component/PythonScriptComponent.h>

#include <string>
#include <utility>

namespace MatchaEditor
{
// Undo command for the Python Script box's "Browse..." button - adds one Binding to an existing
// PythonScriptComponent, rather than the whole component (that's AddComponentCommand's job, used
// when the box itself doesn't exist yet). Scoped to a single entity, not the whole selection - see
// InspectorPanel's own PythonScriptComponent inspector registration comment for why this box
// already only ever edits the front-most selected entity.
//
// Always appends to the end of the vector on Execute(), and always removes from the end on Undo()
// - correct only because CommandManager enforces strict LIFO undo/redo ordering: nothing else on
// the stack can have inserted or removed a binding on this same entity between this command's
// Execute() and its own Undo(), since whatever ran after it is guaranteed to already be undone
// first. See RemoveScriptBindingCommand for the mirror image of this reasoning.
class AddScriptBindingCommand : public Command
{
public:
    AddScriptBindingCommand(SceneManager& sceneManager, std::string description, UUID entityId,
                            std::string moduleName, std::string className)
        : m_SceneManager(sceneManager),
          m_Description(std::move(description)),
          m_EntityId(entityId),
          m_ModuleName(std::move(moduleName)),
          m_ClassName(std::move(className))
    {
    }

    void Execute() override
    {
        Entity entity = m_SceneManager.GetScene().FindEntityByUUID(m_EntityId);
        if (entity.IsValid() && entity.HasComponent<PythonScriptComponent>())
            entity.GetComponent<PythonScriptComponent>().Bind(m_ModuleName, m_ClassName);

        m_SceneManager.GetScene().NotifyChanged();
    }

    void Undo() override
    {
        Entity entity = m_SceneManager.GetScene().FindEntityByUUID(m_EntityId);
        if (entity.IsValid() && entity.HasComponent<PythonScriptComponent>())
        {
            std::vector<PythonScriptComponent::Binding>& bindings = entity.GetComponent<PythonScriptComponent>().bindings;
            if (!bindings.empty())
                bindings.pop_back();
        }

        m_SceneManager.GetScene().NotifyChanged();
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return m_Description;
    }

private:
    SceneManager& m_SceneManager;
    std::string m_Description;
    UUID m_EntityId;
    std::string m_ModuleName;
    std::string m_ClassName;
};
}  // namespace MatchaEditor
