#include "MenuChrome.h"

#include <Matcha.h>

#include <DockWidget.h>

#include <QFileDialog>
#include <QMessageBox>

namespace MatchaEditor
{
namespace
{
// Shared by Save Scene (when no path is set yet) and Save Scene As - prompts for a path and
// hands it to SceneManager::SaveSceneAs() if the user didn't cancel. Returns false if the user
// cancelled the dialog (nothing saved), true otherwise - ConfirmDiscardUnsavedChanges uses that
// to know whether "Save" from its own prompt actually went through.
bool PromptSaveSceneAs(QWidget* parent, Matcha::EngineContext& context)
{
    QString path = QFileDialog::getSaveFileName(parent, "Save Scene As", QString(), "Matcha Scene (*.matcha)");
    if (path.isEmpty())
        return false;

    context.GetSceneManager().SaveSceneAs(path.toStdString());
    return true;
}
}  // namespace

bool ConfirmDiscardUnsavedChanges(QWidget* parent, Matcha::EngineContext& context)
{
    Matcha::SceneManager& sceneManager = context.GetSceneManager();
    if (!sceneManager.IsDirty())
        return true;

    QMessageBox::StandardButton choice =
        QMessageBox::question(parent, "Unsaved Changes", "The current scene has unsaved changes. Save them before continuing?",
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    switch (choice)
    {
    case QMessageBox::Save:
        if (sceneManager.GetFilePath().empty())
            return PromptSaveSceneAs(parent, context);

        sceneManager.SaveScene();
        return true;

    case QMessageBox::Discard:
        return true;

    default:
        return false;
    }
}

MenuChrome::MenuChrome(QMainWindow* mainWindow, Matcha::EngineContext& context)
{
    m_MenuBar = new QMenuBar(mainWindow);

    m_FileMenu = m_MenuBar->addMenu("File");
    QAction* newSceneAction = m_FileMenu->addAction("New Scene");
    QObject::connect(newSceneAction, &QAction::triggered, mainWindow, [&context, mainWindow] {
        if (ConfirmDiscardUnsavedChanges(mainWindow, context))
            context.GetSceneManager().NewScene();
    });

    QAction* openSceneAction = m_FileMenu->addAction("Open Scene...");
    QObject::connect(openSceneAction, &QAction::triggered, mainWindow, [&context, mainWindow] {
        if (!ConfirmDiscardUnsavedChanges(mainWindow, context))
            return;

        QString path = QFileDialog::getOpenFileName(mainWindow, "Open Scene", QString(), "Matcha Scene (*.matcha)");
        if (path.isEmpty())
            return;

        context.GetSceneManager().OpenScene(path.toStdString());
    });

    QAction* saveSceneAction = m_FileMenu->addAction("Save Scene");
    QObject::connect(saveSceneAction, &QAction::triggered, mainWindow, [&context, mainWindow] {
        Matcha::SceneManager& sceneManager = context.GetSceneManager();

        // First save of a scene that's never had a path set behaves like Save As - prompts for
        // one - rather than SceneManager::SaveScene()'s own no-op-with-a-log-warning behavior,
        // which has no visible feedback for someone clicking a menu item.
        if (sceneManager.GetFilePath().empty())
            PromptSaveSceneAs(mainWindow, context);
        else
            sceneManager.SaveScene();
    });

    QAction* saveSceneAsAction = m_FileMenu->addAction("Save Scene As...");
    QObject::connect(saveSceneAsAction, &QAction::triggered, mainWindow,
                     [&context, mainWindow] { PromptSaveSceneAs(mainWindow, context); });

    m_FileMenu->addSeparator();
    QAction* exitAction = m_FileMenu->addAction("Exit");
    // Routed through close() rather than qApp->quit(), so EditorMainWindow::closeEvent()'s
    // unsaved-changes prompt applies to Exit the same way it applies to the window's own X
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
