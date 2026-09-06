#pragma once

#include "Core/Command.h"
#include "Core/Commands/ComponentSnapshot.h"

#include <Matcha.h>

#include <string>
#include <utility>
#include <vector>

namespace MatchaEditor
{
// Undo command for the Inspector's "+ Add Component" - templated on the component type (like
// PropertyEditCommand) so this is one command shared by every addable component, not a
// hand-written one per type.
//
// Undo snapshots each component before removing it, and Execute restores from that snapshot,
// because the component the user added is rarely still the blank default it started as by the
// time they undo: a PythonScriptComponent gets a script bound to it (Browse...), a component's
// fields get edited. Re-adding a blank default on redo would silently discard all of that - which
// is exactly what it did before, most visibly for a Python script, whose whole content is the
// binding added after the component itself.
template <typename Component>
class AddComponentCommand : public Command
{
public:
    AddComponentCommand(EngineContext& context, std::string description, std::string componentKey,
                        std::vector<UUID> entityIds)
        : m_Context(context),
          m_Description(std::move(description)),
          m_ComponentKey(std::move(componentKey)),
          m_EntityIds(std::move(entityIds)),
          // Index-aligned with m_EntityIds and null until the first Undo() fills them in - a null
          // json has no componentKey entry, so RestoreComponent falls back to a blank default,
          // which is exactly right for the very first Execute().
          m_Snapshots(m_EntityIds.size())
    {
    }

    void Execute() override
    {
        Scene& scene = m_Context.GetScene();
        ResourceManager& resourceManager = m_Context.GetResourceManager();

        for (size_t i = 0; i < m_EntityIds.size(); ++i)
        {
            Entity entity = scene.FindEntityByUUID(m_EntityIds[i]);
            if (entity.IsValid() && !entity.HasComponent<Component>())
                RestoreComponent<Component>(entity, m_Snapshots[i], m_ComponentKey, resourceManager);
        }

        scene.NotifyChanged();
    }

    void Undo() override
    {
        Scene& scene = m_Context.GetScene();
        ResourceManager& resourceManager = m_Context.GetResourceManager();

        for (size_t i = 0; i < m_EntityIds.size(); ++i)
        {
            Entity entity = scene.FindEntityByUUID(m_EntityIds[i]);
            if (!entity.IsValid() || !entity.HasComponent<Component>())
                continue;

            m_Snapshots[i] = CaptureComponent(entity, m_ComponentKey, resourceManager);
            entity.RemoveComponent<Component>();
        }

        scene.NotifyChanged();
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return m_Description;
    }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::string m_ComponentKey;

    // Only entities that didn't already have Component at the time this command was built - see
    // InspectorPanel::RegisterComponentInspector, which filters the selection the same way the
    // un-doable "+ Add Component" path always has, before ever constructing this command.
    std::vector<UUID> m_EntityIds;

    // Whatever each entity's component held the last time Undo() removed it, so the next Execute()
    // (a redo) puts back what was actually there rather than a fresh default. Index-aligned with
    // m_EntityIds.
    std::vector<nlohmann::json> m_Snapshots;
};
}  // namespace MatchaEditor
