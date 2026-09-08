#pragma once

#include "Core/CommandManager.h"
#include "Panels/MenuChrome.h"

#include <QMainWindow>

#include <memory>
#include <optional>

class QCloseEvent;

namespace Matcha
{
class Application;
class PythonRuntime;
class QtViewportWidget;
class ResourceManager;
class SceneManager;
class Window;
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
    explicit EditorMainWindow(Matcha::Application& application, Matcha::SceneManager& sceneManager,
                              Matcha::ResourceManager& resourceManager, Matcha::PythonRuntime& pythonRuntime,
                              Matcha::Window& window, Matcha::QtViewportWidget* viewport, QWidget* parent = nullptr);
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
    Matcha::SceneManager& m_SceneManager;

    // Constructed before every panel/MenuChrome below (all of which take a reference to it) and
    // cleared whenever the scene is replaced (New/Open) - see the constructor - since a Command's
    // targets live in the Scene object that New/Open destroys.
    CommandManager m_CommandManager;

    ads::CDockManager* m_DockManager;

    // A member, not a constructor local: it registers a callback into m_CommandManager that keeps
    // running for the window's whole lifetime (updating the Edit menu's enabled state and labels),
    // so the object owning the QActions that callback touches has to outlive the constructor too.
    // std::optional because it's built partway through the constructor body, after m_DockManager.
    std::optional<MenuChrome> m_MenuChrome;

    std::shared_ptr<ConsoleSink> m_ConsoleSink;
};
}  // namespace Matcha
