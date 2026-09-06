#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <vector>

namespace MatchaEditor
{
// Pastes a previously copied set of entities (a SceneSerializer snapshot held by the Scene
// Hierarchy panel's clipboard) as new entities under an optional parent. The inverse shape of
// DeleteEntitiesCommand: Execute() deserializes, Undo() destroys what it created.
class PasteEntitiesCommand : public Command
{
public:
    // The clipboard snapshot is remapped to fresh UUIDs once, here at construction, rather than on
    // every Execute() - so a redo re-creates the same entities under the same identities as the
    // first paste, instead of a third set of ids that Undo()'s recorded roots wouldn't match.
    // Pasting the same clipboard twice means two commands, each with its own remap, which is
    // exactly the intent (two independent copies).
    PasteEntitiesCommand(EngineContext& context, std::string description, const nlohmann::json& clipboard,
                         std::optional<UUID> parentId);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

    // False when the clipboard held nothing usable - the caller skips executing it rather than
    // pushing a command that would do nothing.
    [[nodiscard]] bool IsEmpty() const { return m_Nodes.empty(); }

private:
    EngineContext& m_Context;
    std::string m_Description;
    nlohmann::json m_Nodes;
    std::vector<UUID> m_RootIds;
};
}  // namespace MatchaEditor
