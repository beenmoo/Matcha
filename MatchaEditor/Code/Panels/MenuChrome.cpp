#include "MenuChrome.h"

#include <Matcha.h>

#include <DockWidget.h>

namespace MatchaEditor
{
MenuChrome::MenuChrome(QMainWindow* mainWindow, Matcha::EngineContext& context)
{
    m_MenuBar = new QMenuBar(mainWindow);

    m_FileMenu = m_MenuBar->addMenu("File");
    // New Scene needs no file I/O, so it's real - SceneManager::NewScene() is fully implemented.
    // Open/Save/Save As stay disabled rather than silently doing nothing when clicked: they're
    // stubs in SceneManager until SceneSerializer exists. Flip setEnabled(true) once it does;
    // nothing else about this menu needs to change.
    QAction* newSceneAction = m_FileMenu->addAction("New Scene");
    QObject::connect(newSceneAction, &QAction::triggered, mainWindow,
                      [&context] { context.GetSceneManager().NewScene(); });
    m_FileMenu->addAction("Open Scene...")->setEnabled(false);
    m_FileMenu->addAction("Save Scene")->setEnabled(false);
    m_FileMenu->addAction("Save Scene As...")->setEnabled(false);
    m_FileMenu->addSeparator();
    QAction* exitAction = m_FileMenu->addAction("Exit");
    // Routed through close() rather than qApp->quit(), so a future closeEvent() override (e.g.
    // an unsaved-changes prompt) applies to Exit the same way it would to the window's own X
    // button, instead of Exit bypassing it.
    QObject::connect(exitAction, &QAction::triggered, mainWindow, &QMainWindow::close);

    m_EditMenu = m_MenuBar->addMenu("Edit");
    // Disabled - no undo/redo system exists yet.
    m_EditMenu->addAction("Undo")->setEnabled(false);
    m_EditMenu->addAction("Redo")->setEnabled(false);

    m_ViewMenu = m_MenuBar->addMenu("View");
    m_PanelsMenu = m_ViewMenu->addMenu("Panels");

    mainWindow->setMenuBar(m_MenuBar);
}

void MenuChrome::AddPanel(ads::CDockWidget* panel)
{
    m_PanelsMenu->addAction(panel->toggleViewAction());
}
}  // namespace MatchaEditor
