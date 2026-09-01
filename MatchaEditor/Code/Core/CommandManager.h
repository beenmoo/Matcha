#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace MatchaEditor
{
class Command;

class CommandManager
{
public:
    // Both declared here (not defined inline as `= default`) and defined in CommandManager.cpp,
    // where Command.h is visible: Command is only forward-declared in this header, and both the
    // constructor and destructor of std::vector<std::unique_ptr<Command>> (m_UndoStack/
    // m_RedoStack) need Command's complete type to determine their exception specifications /
    // generate their bodies - if defined inline here, every translation unit that embeds a
    // CommandManager by value (e.g. EditorMainWindow) but doesn't itself happen to include
    // Command.h would fail to compile.
    CommandManager();
    ~CommandManager();

    void Undo();
    void Redo();

    void ExecuteCommand(std::unique_ptr<Command> command);

    // Drops every command on both stacks - required whenever the Scene a stack's commands target
    // is about to be destroyed (SceneManager::AddOnSceneReplaced), since a Command resolves its
    // target(s) fresh out of the live Scene at every Execute()/Undo() and has nothing left to
    // resolve against once that Scene is gone.
    void Clear();

    [[nodiscard]] bool CanUndo() const { return !m_UndoStack.empty(); }
    [[nodiscard]] bool CanRedo() const { return !m_RedoStack.empty(); }

    // Empty string if the respective stack is empty - callers (the Edit menu) treat that as "no
    // description to append", not as an error.
    [[nodiscard]] std::string GetUndoDescription() const;
    [[nodiscard]] std::string GetRedoDescription() const;

    // Single-subscriber (only MenuChrome ever listens, to keep the Edit menu's enabled state and
    // label text in sync) - matches the Set*-prefixed single-slot pattern used elsewhere in this
    // codebase for exactly this shape of thing (e.g. Window::SetTickCallback), not Scene's
    // multi-subscriber Add* pattern (which only exists there because more than one panel
    // independently subscribes).
    void SetOnStackChanged(std::function<void()> callback);

private:
    void NotifyStackChanged();

private:
    std::vector<std::unique_ptr<Command>> m_UndoStack;
    std::vector<std::unique_ptr<Command>> m_RedoStack;
    size_t m_MaxStackSize = 100;  // Limit the number of commands stored in the undo/redo stacks
    std::function<void()> m_OnStackChanged;
};
}  // namespace MatchaEditor