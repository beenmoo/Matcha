#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <functional>
#include <optional>
#include <string>

namespace MatchaEditor
{
// Covers every "Create X Entity" action in the Scene Hierarchy panel (Empty/Cube/Camera/Light) -
// the per-kind differences (which components get added) live entirely in the populate callback,
// not in a separate Command subclass per kind.
class CreateEntityCommand : public Command
{
public:
    // id is fixed at construction (not generated fresh in Execute()) so that Redo() - which just
    // calls Execute() again - recreates the entity under the exact same identity as the first
    // Execute(), rather than a new random one every time.
    CreateEntityCommand(EngineContext& context, std::string description, std::string name,
                        std::optional<UUID> parentId, std::function<void(Entity)> populate = {});

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::string m_Name;
    UUID m_Id;
    std::optional<UUID> m_ParentId;
    std::function<void(Entity)> m_Populate;
};
}  // namespace MatchaEditor
