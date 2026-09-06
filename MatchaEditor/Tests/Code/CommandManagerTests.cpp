#include "pch.h"
#include "Core/Command.h"
#include "Core/CommandManager.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace MatchaEditor;

namespace
{
// Records what the manager did to it, in order, into a log the test owns - CommandManager's
// contract is entirely about *which* command it runs and *when*, so a command that does nothing
// but say so is enough to pin all of it down without dragging in a Scene.
class RecordingCommand : public Command
{
public:
    RecordingCommand(std::string name, std::vector<std::string>& log)
        : m_Name(std::move(name)),
          m_Log(log)
    {
    }

    void Execute() override
    {
        m_Log.push_back(m_Name + ":execute");
    }

    void Undo() override
    {
        m_Log.push_back(m_Name + ":undo");
    }

    [[nodiscard]] std::string GetDescription() const override
    {
        return m_Name;
    }

private:
    std::string m_Name;
    std::vector<std::string>& m_Log;
};

std::unique_ptr<Command> MakeCommand(std::string name, std::vector<std::string>& log)
{
    return std::make_unique<RecordingCommand>(std::move(name), log);
}
}  // namespace

TEST(CommandManagerTests, ExecuteCommandRunsItImmediatelyAndMakesItUndoable)
{
    std::vector<std::string> log;
    CommandManager manager;

    EXPECT_FALSE(manager.CanUndo());
    EXPECT_FALSE(manager.CanRedo());

    manager.ExecuteCommand(MakeCommand("a", log));

    EXPECT_EQ(log, (std::vector<std::string>{"a:execute"}));
    EXPECT_TRUE(manager.CanUndo());
    EXPECT_FALSE(manager.CanRedo());
    EXPECT_EQ(manager.GetUndoDescription(), "a");
    EXPECT_EQ(manager.GetRedoDescription(), "");
}

TEST(CommandManagerTests, UndoAndRedoWalkTheStackInLastInFirstOutOrder)
{
    std::vector<std::string> log;
    CommandManager manager;

    manager.ExecuteCommand(MakeCommand("a", log));
    manager.ExecuteCommand(MakeCommand("b", log));
    log.clear();

    manager.Undo();
    manager.Undo();
    EXPECT_EQ(log, (std::vector<std::string>{"b:undo", "a:undo"}));
    EXPECT_FALSE(manager.CanUndo());
    EXPECT_TRUE(manager.CanRedo());
    EXPECT_EQ(manager.GetRedoDescription(), "a");

    log.clear();
    manager.Redo();
    manager.Redo();
    EXPECT_EQ(log, (std::vector<std::string>{"a:execute", "b:execute"}));
    EXPECT_TRUE(manager.CanUndo());
    EXPECT_FALSE(manager.CanRedo());
}

TEST(CommandManagerTests, UndoAndRedoOnEmptyStacksDoNothing)
{
    std::vector<std::string> log;
    CommandManager manager;

    manager.Undo();
    manager.Redo();

    EXPECT_TRUE(log.empty());
    EXPECT_FALSE(manager.CanUndo());
    EXPECT_FALSE(manager.CanRedo());
}

TEST(CommandManagerTests, ExecutingANewCommandDropsTheRedoBranch)
{
    std::vector<std::string> log;
    CommandManager manager;

    manager.ExecuteCommand(MakeCommand("a", log));
    manager.Undo();
    ASSERT_TRUE(manager.CanRedo());

    // The scene has moved on down a different branch - "a" can no longer be replayed onto it.
    manager.ExecuteCommand(MakeCommand("b", log));

    EXPECT_FALSE(manager.CanRedo());
    EXPECT_EQ(manager.GetUndoDescription(), "b");
}

TEST(CommandManagerTests, ClearDropsBothStacksWithoutUndoingAnything)
{
    std::vector<std::string> log;
    CommandManager manager;

    manager.ExecuteCommand(MakeCommand("a", log));
    manager.ExecuteCommand(MakeCommand("b", log));
    manager.Undo();
    log.clear();

    manager.Clear();

    // Nothing is replayed on the way out: Clear() exists for the case where the Scene these
    // commands target is being destroyed (SceneManager::AddOnSceneReplaced), so undoing them
    // against it would be exactly the wrong thing.
    EXPECT_TRUE(log.empty());
    EXPECT_FALSE(manager.CanUndo());
    EXPECT_FALSE(manager.CanRedo());
    EXPECT_EQ(manager.GetUndoDescription(), "");
    EXPECT_EQ(manager.GetRedoDescription(), "");
}

TEST(CommandManagerTests, StackChangedFiresForEveryMutationAndOnlyForRealOnes)
{
    std::vector<std::string> log;
    CommandManager manager;

    int notifications = 0;
    manager.SetOnStackChanged([&notifications] { ++notifications; });

    manager.ExecuteCommand(MakeCommand("a", log));
    EXPECT_EQ(notifications, 1);

    manager.Undo();
    EXPECT_EQ(notifications, 2);

    manager.Redo();
    EXPECT_EQ(notifications, 3);

    manager.Clear();
    EXPECT_EQ(notifications, 4);

    // Nothing left to change: a no-op Undo/Redo/Clear shouldn't make the Edit menu rebuild
    // itself, which is the one thing subscribed to this.
    manager.Undo();
    manager.Redo();
    manager.Clear();
    EXPECT_EQ(notifications, 4);
}

TEST(CommandManagerTests, TheOldestCommandIsDroppedOnceTheUndoStackIsFull)
{
    std::vector<std::string> log;
    CommandManager manager;

    // One past the 100-entry cap, so exactly one command (the first) should have been evicted.
    for (int i = 0; i < 101; ++i)
        manager.ExecuteCommand(MakeCommand(std::to_string(i), log));

    log.clear();

    int undone = 0;
    while (manager.CanUndo())
    {
        manager.Undo();
        ++undone;
    }

    EXPECT_EQ(undone, 100);
    EXPECT_EQ(log.front(), "100:undo");
    EXPECT_EQ(log.back(), "1:undo");  // "0" was evicted, not undone
}
