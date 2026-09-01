#pragma once

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>

namespace ads
{
class CDockWidget;
}  // namespace ads

namespace Matcha
{
class EngineContext;
}  // namespace Matcha

namespace MatchaEditor
{
class CommandManager;

// Returns true if it's fine to proceed with something that would discard the current scene's
// unsaved changes (New Scene, Open Scene, closing the window) - false if the user cancelled.
// A no-op (returns true immediately) if the scene isn't dirty. Otherwise prompts Save/Discard/
// Cancel; Save routes through the same "prompt for a path if none is set yet" flow the File menu's
// own Save action uses. Declared here (not just used internally by MenuChrome's own actions)
// since EditorMainWindow's closeEvent() needs the identical check for the window's X button/Exit.
[[nodiscard]] bool ConfirmDiscardUnsavedChanges(QWidget* parent, Matcha::EngineContext& context);

// Owns the editor's top-level menu bar structure (File/Edit/View, and View's Panels submenu).
// Deliberately just exposes each QMenu rather than wrapping menu-building in its own API -
// adding a new action anywhere is `menuChrome.GetFileMenu()->addAction(...)` at the call site,
// no new MenuChrome code needed. AddPanel() is the one exception: turning a dock widget into a
// Panels-menu entry isn't just "add an action" (see its own comment below), so that one piece of
// logic is centralized here instead of repeated at every call site.
class MenuChrome
{
public:
    explicit MenuChrome(QMainWindow* mainWindow, Matcha::EngineContext& context, CommandManager& commandManager);

    QMenuBar* GetMenuBar() const { return m_MenuBar; }
    QMenu* GetFileMenu() const { return m_FileMenu; }
    QMenu* GetEditMenu() const { return m_EditMenu; }
    QMenu* GetViewMenu() const { return m_ViewMenu; }
    QMenu* GetPanelsMenu() const { return m_PanelsMenu; }

    // Adds the panel's own built-in toggle-view action (checkable, stays in sync with whether
    // it's open or closed, labeled with the panel's title) to View > Panels - this is the only
    // way to bring a panel back after closing it, since ADS otherwise just hides a closed dock
    // widget with no other route back to it.
    void AddPanel(ads::CDockWidget* panel);

private:
    QMenuBar* m_MenuBar;
    QMenu* m_FileMenu;
    QMenu* m_EditMenu;
    QMenu* m_ViewMenu;
    QMenu* m_PanelsMenu;

    QAction* m_UndoAction;
    QAction* m_RedoAction;
};
}  // namespace MatchaEditor
