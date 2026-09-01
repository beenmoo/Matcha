#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <string>
#include <vector>

namespace MatchaEditor
{
// Covers both the Scene Hierarchy panel's single-item inline rename and its multi-select
// rename-all dialog - a list of size 1 collapses to a single-entity rename, a list of size N
// becomes one undo entry covering all N.
class RenameEntitiesCommand : public Command
{
public:
    struct Rename
    {
        UUID id;
        std::string oldName;
        std::string newName;
    };

    RenameEntitiesCommand(EngineContext& context, std::string description, std::vector<Rename> renames);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::vector<Rename> m_Renames;
};
}  // namespace MatchaEditor
