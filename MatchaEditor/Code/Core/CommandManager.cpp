#include "CommandManager.h"
#include "Command.h"

namespace MatchaEditor
{
CommandManager::CommandManager() = default;
CommandManager::~CommandManager() = default;

void CommandManager::Undo()
{
    if (m_UndoStack.empty())
        return;

    auto command = std::move(m_UndoStack.back());
    m_UndoStack.pop_back();
    command->Undo();
    m_RedoStack.push_back(std::move(command));

    NotifyStackChanged();
}

void CommandManager::Redo()
{
    if (m_RedoStack.empty())
        return;

    auto command = std::move(m_RedoStack.back());
    m_RedoStack.pop_back();
    command->Execute();
    m_UndoStack.push_back(std::move(command));

    NotifyStackChanged();
}

void CommandManager::ExecuteCommand(std::unique_ptr<Command> command)
{
    command->Execute();
    m_UndoStack.push_back(std::move(command));

    // A fresh action invalidates whatever branch the redo stack was tracking - without this, an
    // Undo followed by a new ExecuteCommand() leaves stale (now out-of-sync with the scene) redo
    // entries reachable via Redo().
    m_RedoStack.clear();

    // Limit the size of the undo stack
    if (m_UndoStack.size() > m_MaxStackSize)
    {
        m_UndoStack.erase(m_UndoStack.begin());
    }

    NotifyStackChanged();
}

void CommandManager::Clear()
{
    if (m_UndoStack.empty() && m_RedoStack.empty())
        return;

    m_UndoStack.clear();
    m_RedoStack.clear();

    NotifyStackChanged();
}

std::string CommandManager::GetUndoDescription() const
{
    return m_UndoStack.empty() ? std::string() : m_UndoStack.back()->GetDescription();
}

std::string CommandManager::GetRedoDescription() const
{
    return m_RedoStack.empty() ? std::string() : m_RedoStack.back()->GetDescription();
}

void CommandManager::SetOnStackChanged(std::function<void()> callback)
{
    m_OnStackChanged = std::move(callback);
}

void CommandManager::NotifyStackChanged()
{
    if (m_OnStackChanged)
        m_OnStackChanged();
}
}  // namespace MatchaEditor
