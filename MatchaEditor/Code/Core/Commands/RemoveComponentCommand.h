#pragma once

#include "Core/Command.h"
#include "Core/Commands/ComponentSnapshot.h"

#include <Matcha.h>

#include <string>
#include <utility>
#include <vector>

namespace MatchaEditor
{
// Undo command for the Inspector's "Remove Component" (x) button - templated on the component
// type like AddComponentCommand/PropertyEditCommand, so this is one command shared by every
// removable component rather than a hand-written one per type.
//
// The mirror image of AddComponentCommand: this one snapshots up front (in the constructor,
// before Execute() destroys anything, the same way DeleteEntitiesCommand snapshots whole
// entities) and restores on Undo, rather than the other way round. Both use the same
// ComponentSnapshot capture/restore pair - see componentKey's meaning there.
template <typename Component>
class RemoveComponentCommand : public Command
{
public:
    RemoveComponentCommand(SceneManager& sceneManager, ResourceManager& resourceManager, std::string description,
                           std::string componentKey, const std::vector<Entity>& entities)
        : m_SceneManager(sceneManager),
          m_ResourceManager(resourceManager),
          m_Description(std::move(description)),
          m_ComponentKey(std::move(componentKey))
    {
        for (Entity entity : entities)
        {
            if (!entity.HasComponent<Component>())
                continue;

            Snapshot snapshot;
            snapshot.id = entity.GetComponent<TagComponent>().id;
            snapshot.data = CaptureComponent(entity, m_ComponentKey, m_ResourceManager);

            m_Snapshots.push_back(std::move(snapshot));
        }
    }

    void Execute() override
    {
        Scene& scene = m_SceneManager.GetScene();

        for (const Snapshot& snapshot : m_Snapshots)
        {
            Entity entity = scene.FindEntityByUUID(snapshot.id);
            if (entity.IsValid() && entity.HasComponent<Component>())
                entity.RemoveComponent<Component>();
        }

        scene.NotifyChanged();
    }

    void Undo() override
    {
        Scene& scene = m_SceneManager.GetScene();

        for (const Snapshot& snapshot : m_Snapshots)
        {
            Entity entity = scene.FindEntityByUUID(snapshot.id);
            if (!entity.IsValid() || entity.HasComponent<Component>())
                continue;

            RestoreComponent<Component>(entity, snapshot.data, m_ComponentKey, m_ResourceManager);
        }

        scene.NotifyChanged();
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return m_Description;
    }

private:
    struct Snapshot
    {
        UUID id;
        nlohmann::json data;
    };

private:
    SceneManager& m_SceneManager;
    ResourceManager& m_ResourceManager;
    std::string m_Description;
    std::string m_ComponentKey;
    std::vector<Snapshot> m_Snapshots;
};
}  // namespace MatchaEditor
