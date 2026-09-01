#pragma once

#include "Core/CommandManager.h"

#include <QMainWindow>

#include <memory>

class QCloseEvent;

namespace Matcha
{
class QtViewportWidget;
class EngineContext;
}  // namespace Matcha

namespace ads
{
class CDockManager;
}  // namespace ads

namespace MatchaEditor
{
class ConsoleSink;

class EditorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorMainWindow(Matcha::EngineContext& context, Matcha::QtViewportWidget* viewport, QWidget* parent = nullptr);
    ~EditorMainWindow() override;

protected:
    // Catches the window's own close (X button) the same way MenuChrome's Exit action does
    // (Exit is routed through close() specifically so this applies to both) - prompts to save
    // unsaved changes and ignores the close if the user cancels.
    void closeEvent(QCloseEvent* event) override;

private:
    // Reflects SceneManager's current file path and dirty state (an asterisk) - refreshed from
    // SceneManager::AddOnDirtyChanged/AddOnSceneReplaced, not computed once at construction.
    void UpdateWindowTitle();

private:
    Matcha::EngineContext& m_Context;

    // Constructed before every panel/MenuChrome below (all of which take a reference to it) and
    // cleared whenever the scene is replaced (New/Open) - see the constructor - since a Command's
    // targets live in the Scene object that New/Open destroys.
    CommandManager m_CommandManager;

    ads::CDockManager* m_DockManager;
    std::shared_ptr<ConsoleSink> m_ConsoleSink;
};
}  // namespace Matcha
