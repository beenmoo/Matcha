#include "MenuChrome.h"

#include <DockWidget.h>

namespace MatchaEditor
{
MenuChrome::MenuChrome(QMainWindow* mainWindow)
{
    m_MenuBar = new QMenuBar(mainWindow);

    m_FileMenu = m_MenuBar->addMenu("File");
    // Disabled rather than silently doing nothing when clicked - no scene serialization exists
    // yet to back these. Flip setEnabled(true) and connect() once it does; nothing else about
    // this menu needs to change.
    m_FileMenu->addAction("New Scene")->setEnabled(false);
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
