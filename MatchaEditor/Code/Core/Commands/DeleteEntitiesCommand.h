#pragma once

#include "Core/Command.h"

#include <Matcha.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace MatchaEditor
{
// Covers deleting one or more top-level selections from the Scene Hierarchy panel (each of which
// may itself have children) as a single undo entry. Snapshots every root's whole subtree via
// SceneSerializer at construction time - nothing survives in the ECS after
// DestroyEntityRecursive(), so there's no other source to reconstruct from on Undo().
class DeleteEntitiesCommand : public Command
{
public:
    // subtreeRoots must be live, valid entities at construction time - the snapshot is taken here,
    // before Execute() ever runs, since the caller's handles are only guaranteed valid right now.
    DeleteEntitiesCommand(EngineContext& context, std::string description, const std::vector<Entity>& subtreeRoots);

    void Execute() override;
    void Undo() override;
    [[nodiscard]] std::string GetDescription() const override { return m_Description; }

private:
    EngineContext& m_Context;
    std::string m_Description;
    std::vector<UUID> m_RootIds;
    nlohmann::json m_Snapshot;
};
}  // namespace MatchaEditor
