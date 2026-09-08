#pragma once

#include "Core/Command.h"

#include <Matcha.h>
#include <Scene/Component/PythonScriptComponent.h>

#include <algorithm>
#include <string>
#include <utility>

namespace MatchaEditor
{
// Undo command for the Python Script box's per-binding "Remove Script" button - removes one
// Binding from an existing PythonScriptComponent's vector, snapshotting its (moduleName,
// className) at construction time so Undo() can restore it. The live pybind11::object instance is
// deliberately not preserved across the round trip - same reasoning RemoveComponentCommand's
// restore already follows for a whole component: a restored binding always re-instantiates fresh
// the next time PythonScriptSystem ticks it, rather than keeping a detached script instance alive
// with nothing driving it while it isn't attached to anything.
//
// Targets a specific index into the vector, captured at construction - correct under
// CommandManager's strict LIFO undo/redo ordering for the same reason AddScriptBindingCommand's
// own comment explains: nothing else can have changed this entity's bindings between this
// command's Execute()/Undo() calls without that change already being undone first.
class RemoveScriptBindingCommand : public Command
{
public:
    RemoveScriptBindingCommand(SceneManager& sceneManager, std::string description, UUID entityId, size_t index)
        : m_SceneManager(sceneManager),
          m_Description(std::move(description)),
          m_EntityId(entityId),
          m_Index(index)
    {
        Entity entity = m_SceneManager.GetScene().FindEntityByUUID(m_EntityId);
        if (!entity.IsValid() || !entity.HasComponent<PythonScriptComponent>())
            return;

        const std::vector<PythonScriptComponent::Binding>& bindings = entity.GetComponent<PythonScriptComponent>().bindings;
        if (m_Index < bindings.size())
        {
            m_ModuleName = bindings[m_Index].moduleName;
            m_ClassName = bindings[m_Index].className;
        }
    }

    void Execute() override
    {
        Entity entity = m_SceneManager.GetScene().FindEntityByUUID(m_EntityId);
        if (entity.IsValid() && entity.HasComponent<PythonScriptComponent>())
        {
            std::vector<PythonScriptComponent::Binding>& bindings = entity.GetComponent<PythonScriptComponent>().bindings;
            if (m_Index < bindings.size())
                bindings.erase(bindings.begin() + m_Index);
        }

        m_SceneManager.GetScene().NotifyChanged();
    }

    void Undo() override
    {
        Entity entity = m_SceneManager.GetScene().FindEntityByUUID(m_EntityId);
        if (entity.IsValid() && entity.HasComponent<PythonScriptComponent>())
        {
            std::vector<PythonScriptComponent::Binding>& bindings = entity.GetComponent<PythonScriptComponent>().bindings;
            size_t insertAt = std::min(m_Index, bindings.size());
            bindings.insert(bindings.begin() + insertAt, {m_ModuleName, m_ClassName, pybind11::object()});
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
    size_t m_Index;
    std::string m_ModuleName;
    std::string m_ClassName;
};
}  // namespace MatchaEditor
