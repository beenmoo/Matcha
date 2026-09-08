#include "MenuChrome.h"
#include "Core/CommandManager.h"
#include "Widgets/PreferencesDialog.h"

#include <Matcha.h>

#include <DockWidget.h>

#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>

#include <string>

namespace MatchaEditor
{
namespace
{
// Shared by Save Scene (when no path is set yet) and Save Scene As - prompts for a path and
// hands it to SceneManager::SaveSceneAs() if the user didn't cancel. Returns false if the user
// cancelled the dialog (nothing saved), true otherwise - ConfirmDiscardUnsavedChanges uses that
// to know whether "Save" from its own prompt actually went through.
bool PromptSaveSceneAs(QWidget* parent, Matcha::SceneManager& sceneManager)
{
    QString path = QFileDialog::getSaveFileName(parent, "Save Scene As", QString(), "Matcha Scene (*.matcha)");
    if (path.isEmpty())
        return false;

    sceneManager.SaveSceneAs(path.toStdString());
    return true;
}
}  // namespace

bool ConfirmDiscardUnsavedChanges(QWidget* parent, Matcha::SceneManager& sceneManager)
{
    if (!sceneManager.IsDirty())
        return true;

    QMessageBox::StandardButton choice =
        QMessageBox::question(parent, "Unsaved Changes", "The current scene has unsaved changes. Save them before continuing?",
                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    switch (choice)
    {
    case QMessageBox::Save:
        if (sceneManager.GetFilePath().empty())
            return PromptSaveSceneAs(parent, sceneManager);

        sceneManager.SaveScene();
        return true;

    case QMessageBox::Discard:
        return true;

    default:
        return false;
    }
}

MenuChrome::MenuChrome(QMainWindow* mainWindow, Matcha::SceneManager& sceneManager, Matcha::Window& window,
                       CommandManager& commandManager)
{
    m_MenuBar = new QMenuBar(mainWindow);
    mainWindow->setMenuBar(m_MenuBar);

    m_FileMenu = m_MenuBar->addMenu("File");
    QAction* newSceneAction = m_FileMenu->addAction("New Scene");
    newSceneAction->setShortcut(QKeySequence::New);
    QObject::connect(newSceneAction, &QAction::triggered, mainWindow, [&sceneManager, mainWindow] {
        if (ConfirmDiscardUnsavedChanges(mainWindow, sceneManager))
            sceneManager.NewScene();
    });

    QAction* openSceneAction = m_FileMenu->addAction("Open Scene...");
    openSceneAction->setShortcut(QKeySequence::Open);
    QObject::connect(openSceneAction, &QAction::triggered, mainWindow, [&sceneManager, &window, mainWindow] {
        if (!ConfirmDiscardUnsavedChanges(mainWindow, sceneManager))
            return;

        QString path = QFileDialog::getOpenFileName(mainWindow, "Open Scene", QString(), "Matcha Scene (*.matcha)");
        if (path.isEmpty())
            return;

        // Deserializing a scene issues GL calls - every MeshComponent regenerates its primitive
        // (VAO/VBO) and every MaterialComponent compiles its shader, both through ResourceManager.
        // Under the Qt backend the viewport's context is only current inside initializeGL/
        // resizeGL/paintGL, and this runs from a menu action instead, so without this the whole
        // scene's meshes and shaders are built against no context at all and nothing draws - a
        // black viewport, with no error to point at it. Same reasoning (and same fix) as
        // SceneHierarchyWidget::EnsureStandardMeshShader, which is the other place editor UI
        // creates GL resources outside the render loop.
        window.MakeContextCurrent();
        sceneManager.OpenScene(path.toStdString());
    });

    QAction* saveSceneAction = m_FileMenu->addAction("Save Scene");
    saveSceneAction->setShortcut(QKeySequence::Save);
    QObject::connect(saveSceneAction, &QAction::triggered, mainWindow, [&sceneManager, mainWindow] {
        // First save of a scene that's never had a path set behaves like Save As - prompts for
        // one - rather than SceneManager::SaveScene()'s own no-op-with-a-log-warning behavior,
        // which has no visible feedback for someone clicking a menu item.
        if (sceneManager.GetFilePath().empty())
            PromptSaveSceneAs(mainWindow, sceneManager);
        else
            sceneManager.SaveScene();
    });

    QAction* saveSceneAsAction = m_FileMenu->addAction("Save Scene As...");
    saveSceneAsAction->setShortcut(QKeySequence::SaveAs);
    QObject::connect(saveSceneAsAction, &QAction::triggered, mainWindow,
                     [&sceneManager, mainWindow] { PromptSaveSceneAs(mainWindow, sceneManager); });

    m_FileMenu->addSeparator();
    QAction* exitAction = m_FileMenu->addAction("Exit");
    // Routed through close() rather than qApp->quit(), so EditorMainWindow::closeEvent()'s
    // unsaved-changes prompt applies to Exit the same way it applies to the window's own X
    // button, instead of Exit bypassing it.
    QObject::connect(exitAction, &QAction::triggered, mainWindow, &QMainWindow::close);

    m_EditMenu = m_MenuBar->addMenu("Edit");

    m_UndoAction = m_EditMenu->addAction("Undo");
    m_UndoAction->setShortcut(QKeySequence::Undo);
    QObject::connect(m_UndoAction, &QAction::triggered, mainWindow, [&commandManager] { commandManager.Undo(); });

    m_RedoAction = m_EditMenu->addAction("Redo");
    m_RedoAction->setShortcut(QKeySequence::Redo);
    QObject::connect(m_RedoAction, &QAction::triggered, mainWindow, [&commandManager] { commandManager.Redo(); });

    // Keeps enabled state and label text ("Undo <description>") in sync with the stack - invoked
    // once immediately below for the initial (empty) state, then again on every ExecuteCommand()/
    // Undo()/Redo()/Clear(). Captures the QAction pointers rather than `this` so the callback
    // depends only on the actions themselves (owned by m_EditMenu, and so by the main window),
    // not on MenuChrome's own lifetime - CommandManager holds this callback for as long as the
    // editor runs.
    QAction* undoAction = m_UndoAction;
    QAction* redoAction = m_RedoAction;
    auto updateEditMenu = [undoAction, redoAction, &commandManager] {
        undoAction->setEnabled(commandManager.CanUndo());
        std::string undoDescription = commandManager.GetUndoDescription();
        undoAction->setText(undoDescription.empty() ? "Undo" : QString("Undo %1").arg(QString::fromStdString(undoDescription)));

        redoAction->setEnabled(commandManager.CanRedo());
        std::string redoDescription = commandManager.GetRedoDescription();
        redoAction->setText(redoDescription.empty() ? "Redo" : QString("Redo %1").arg(QString::fromStdString(redoDescription)));
    };
    commandManager.SetOnStackChanged(updateEditMenu);
    updateEditMenu();

    m_PreferencesAction = m_EditMenu->addAction("Preferences...");
    QObject::connect(m_PreferencesAction, &QAction::triggered, mainWindow, [mainWindow] {
        PreferencesDialog dialog(mainWindow);
        dialog.exec();
    });

    m_ViewMenu = m_MenuBar->addMenu("View");
    m_PanelsMenu = m_ViewMenu->addMenu("Panels");
}

void MenuChrome::AddPanel(ads::CDockWidget* panel)
{
    m_PanelsMenu->addAction(panel->toggleViewAction());
}
}  // namespace MatchaEditor
