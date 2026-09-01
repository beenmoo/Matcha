#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <optional>
#include <string>
#include <vector>

namespace MatchaEditor
{
// Covers dragging one or more selected rows onto another row in the Scene Hierarchy panel. All
// dragged entities share one new parent (the drop target), but each keeps its own individual old
// parent to restore on undo - a multi-select drag can pull entities from different original
// parents.
class ReparentEntitiesCommand : public Command
{
public:
    struct Reparent
    {
        UUID id;
        std::optional<UUID> oldParentId;  // nullopt = was at the scene root
    };

    // newParentId nullopt = the drop target was the scene root (detach).
    ReparentEntitiesCommand(EngineContext& context, std::string description, std::vector<Reparent> reparents,
                            std::optional<UUID> newParentId);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::vector<Reparent> m_Reparents;
    std::optional<UUID> m_NewParentId;
};
}  // namespace MatchaEditor
