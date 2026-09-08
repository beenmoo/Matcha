#include "EditorMainWindow.h"
#include "ConsoleSink.h"
#include "EditorCamera.h"
#include "ViewportInteraction.h"
#include "Panels/AssetBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/MenuChrome.h"
#include "Widgets/AssetBrowserWidget.h"
#include "Core/Logger.h"
#include "Core/Qt/QtViewportWidget.h"

#include <Matcha.h>

#include <spdlog/spdlog.h>

#include <DockAreaWidget.h>
#include <DockManager.h>

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QUrl>
#include <QSettings>
#include <QProcess>
#include <QProcessEnvironment>

#include <algorithm>

namespace MatchaEditor
{
namespace
{
void AttachSink(const spdlog::sink_ptr& sink, const char* loggerName)
{
    if (auto logger = spdlog::get(loggerName))
        logger->sinks().push_back(sink);
}

void DetachSink(const spdlog::sink_ptr& sink, const char* loggerName)
{
    if (auto logger = spdlog::get(loggerName))
    {
        auto& sinks = logger->sinks();
        sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
    }
}

// A plain Qt-drawn pixmap, not extracted from any native HICON - sidesteps a Qt/Windows bug
// where converting certain system icons to a QPixmap asserts on the AND-mask's bitmap format.
QIcon MakePlaceholderIcon()
{
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0x6a, 0x9a, 0x4a));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(pixmap.rect().adjusted(2, 2, -2, -2));

    return QIcon(pixmap);
}

// Qt-Advanced-Docking-System's CDockManager sets its own default stylesheet directly on itself
// during construction (an embedded resource, src/stylesheets/default.css upstream). Appending
// rules after construction overrides pieces of it: later rules win Qt's CSS cascade for
// equal-specificity selectors, so this doesn't need to replace ADS's own sheet, just follow it.
// Three unrelated fixes live here:
//  - Icon size: every tab/title-bar button icon defaults to 16px, which is what makes tabs read
//    as "big". The tab's own left margin is a second, smaller contributor to that - it's computed
//    from QFontMetrics of the application's default font at each tab's construction time
//    (DockWidgetTabPrivate::createLayout(), confirmed by reading ADS 4.5.0's actual source), not
//    stylesheet-driven, so it can only shrink via a global QApplication font change - left alone
//    here since that would affect text everywhere, not just tabs.
//  - Contrast: default.css colors inactive-tab text via palette(dark), which Theme.cpp sets to
//    button.darker(150) (~RGB 46,46,46) - almost invisible against the tab's own palette(window)
//    background (60,60,60). Confirmed by computing both, not just by eye.
//  - Flatness: default.css's active tab uses a qlineargradient (palette(window) to palette(light))
//    - the only gradient anywhere in this editor's theme, which is otherwise flat everywhere else
//    - and the close button's hover/press states darken with a black rgba overlay, invisible (or
//    backwards-looking) on a dark background instead of lightening it.
//  - Selection: the active tab's own background is flattened to palette(base) - a deliberate
//    accent border was tried here too (palette(highlight), matching the color everywhere else in
//    this app uses for selection) but reverted: "active" in ADS means "frontmost tab of its own
//    dock area's tab group", and this editor's default layout never actually tabs any panels
//    together (Scene Hierarchy/Viewport/Inspector/Console each get their own separate dock area in
//    EditorMainWindow.cpp) - a lone tab with no siblings is trivially "active", so every panel got
//    a loud accent simultaneously instead of only whichever one a user had actually brought
//    forward in a real tab group. The subtler background-only version doesn't read as a false
//    signal the same way, so it stays; a real "you are here" accent needs to only ever apply once
//    two or more panels are genuinely tabbed together, which isn't something plain QSS can
//    condition on (sibling count isn't a selectable property) - would need actual C++ if wanted.
QString DockChromeStyleSheetOverrides()
{
    return QStringLiteral(
        "ads--CTitleBarButton { qproperty-iconSize: 12px; }"
        "#tabCloseButton { qproperty-iconSize: 12px; padding: 0px -4px; }"
        "#tabsMenuButton { qproperty-iconSize: 12px; }"
        "#dockAreaCloseButton { qproperty-iconSize: 12px; }"
        "#detachGroupButton { qproperty-iconSize: 12px; }"
        "#dockAreaAutoHideButton { qproperty-iconSize: 12px; }"
        "#dockAreaMinimizeButton { qproperty-iconSize: 12px; }"
        "ads--CDockWidgetTab[activeTab=\"true\"] { background: palette(base); }"
        "ads--CDockWidgetTab QLabel { color: #a0a0a0; }"
        "ads--CDockWidgetTab[activeTab=\"true\"] QLabel { color: palette(foreground); }"
        "#tabCloseButton:hover { background: rgba(255, 255, 255, 32); border: none; }"
        "#tabCloseButton:pressed { background: rgba(255, 255, 255, 56); }");
}
}  // namespace

EditorMainWindow::EditorMainWindow(Matcha::Application& application, Matcha::SceneManager& sceneManager,
                                   Matcha::ResourceManager& resourceManager, Matcha::PythonRuntime& pythonRuntime,
                                   Matcha::Window& window, Matcha::QtViewportWidget* viewport, EditorCamera& editorCamera,
                                   QWidget* parent)
    : QMainWindow(parent),
      m_SceneManager(sceneManager)
{
    setWindowIcon(MakePlaceholderIcon());

    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaHasTabsMenuButton, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    m_DockManager = new ads::CDockManager(this);
    m_DockManager->setStyleSheet(m_DockManager->styleSheet() + DockChromeStyleSheetOverrides());
    setCentralWidget(m_DockManager);

    m_MenuChrome.emplace(this, sceneManager, window, m_CommandManager);

    // Unlike Scene::AddOnSceneChanged (which InspectorPanel/SceneHierarchyWidget have to
    // re-subscribe to on every scene swap, since it dies with the Scene it's attached to), these
    // two are SceneManager's own callbacks - SceneManager itself outlives any one Scene, so a
    // single subscription here covers every scene for the rest of the editor's lifetime.
    UpdateWindowTitle();
    sceneManager.AddOnDirtyChanged([this] { UpdateWindowTitle(); });
    sceneManager.AddOnSceneReplaced([this] { UpdateWindowTitle(); });

    // A Command resolves its target(s) fresh out of the live Scene at every Execute()/Undo() (see
    // CommandManager.h) - once New/Open destroys that Scene, every command on both stacks is
    // meaningless, so drop them rather than leave them to fail silently (or resolve into whatever
    // entity happens to reuse the same UUID by coincidence, which can't happen, but is exactly
    // the kind of thing this guards against on principle).
    sceneManager.AddOnSceneReplaced([this] { m_CommandManager.Clear(); });

    SceneHierarchyPanel* sceneHierarchyPanel =
        new SceneHierarchyPanel(m_DockManager, sceneManager, resourceManager, window, m_CommandManager, this);
    ads::CDockAreaWidget* sceneHierarchyArea = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, sceneHierarchyPanel);
    m_MenuChrome->AddPanel(sceneHierarchyPanel);

    ViewportPanel* viewportPanel = new ViewportPanel(m_DockManager, viewport, this);
    ads::CDockAreaWidget* viewportArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, viewportPanel, sceneHierarchyArea);
    m_MenuChrome->AddPanel(viewportPanel);

    // Turns a viewport click into a scene-entity selection, fed into the exact same
    // SceneHierarchyPanel::SelectEntities -> SelectionChanged -> InspectorPanel::SetSelectedEntities
    // path an outliner click already goes through - the outliner's tree selection stays the single
    // source of truth, the viewport just becomes a second way to drive it.
    m_ViewportInteraction =
        std::make_unique<ViewportInteraction>(sceneManager, resourceManager, editorCamera, *viewport, m_CommandManager, this);
    connect(viewport, &QtViewportWidget::Clicked, m_ViewportInteraction.get(), &ViewportInteraction::OnViewportClicked);
    connect(viewport, &QtViewportWidget::Released, m_ViewportInteraction.get(), &ViewportInteraction::OnViewportReleased);
    connect(m_ViewportInteraction.get(), &ViewportInteraction::EntityPicked, sceneHierarchyPanel, &SceneHierarchyPanel::SelectEntities);

    InspectorPanel* inspectorPanel =
        new InspectorPanel(m_DockManager, sceneManager, resourceManager, pythonRuntime, m_CommandManager, this);
    ads::CDockAreaWidget* inspectorArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, inspectorPanel, viewportArea);
    m_MenuChrome->AddPanel(inspectorPanel);
    connect(sceneHierarchyPanel, &SceneHierarchyPanel::SelectionChanged, inspectorPanel, &InspectorPanel::SetSelectedEntities);
    connect(sceneHierarchyPanel, &SceneHierarchyPanel::SelectionChanged, m_ViewportInteraction.get(), &ViewportInteraction::SetSelectedEntities);

    ConsolePanel* consolePanel = new ConsolePanel(m_DockManager, this);
    ads::CDockAreaWidget* consoleArea = m_DockManager->addDockWidget(ads::BottomDockWidgetArea, consolePanel);
    m_MenuChrome->AddPanel(consolePanel);

    // Tabbed alongside the Console rather than its own split area - CenterDockWidgetArea plus an
    // existing area is ADS's documented way to add a dock widget as a sibling tab of that area
    // instead of splitting it. Matches the Project/Console pairing this editor's layout is
    // otherwise modeled on (Unity, Unreal): both are "browse project state" panels that only need
    // to be visible one at a time, unlike Scene Hierarchy/Viewport/Inspector, which all want to
    // stay on screen together.
    AssetBrowserPanel* assetBrowserPanel = new AssetBrowserPanel(m_DockManager, application, this);
    m_DockManager->addDockWidget(ads::CenterDockWidgetArea, assetBrowserPanel, consoleArea);
    m_MenuChrome->AddPanel(assetBrowserPanel);

    connect(assetBrowserPanel->GetBrowserWidget(), &AssetBrowserWidget::AssetDoubleClicked, this,
            [this](const QString& assetPath) {
                // A scene switches SceneManager to it, through the same unsaved-changes guard
                // File > Open Scene already uses - a double-click shouldn't be a lower-friction
                // way to discard unsaved work than the menu action for the exact same operation.
                QString fileSuffix = QFileInfo(assetPath).suffix();

                if (fileSuffix.compare("matcha", Qt::CaseInsensitive) == 0)
                {
                    if (ConfirmDiscardUnsavedChanges(this, m_SceneManager))
                        m_SceneManager.OpenScene(assetPath.toStdString());

                    return;
                }

                // A script double-clicked from the Project window is a request to edit it,
                // not to run it - so don't pass it to PythonRuntime, which would execute it
                // immediately. Instead, open it in whatever external editor the user has
                // configured for that file type, or fall back to the OS default if none is
                // configured.
                QSettings settings("MatchaEditor");
                QString externalEditorPath = settings.value("ExternalEditor/Path", "").toString();

                if (!externalEditorPath.isEmpty() && QFileInfo::exists(externalEditorPath))
                {
                    // Launch the configured executable and pass the script path. Goes through
                    // a QProcess instance (rather than the static QProcess::startDetached
                    // overload) so the environment can be sanitized first - if this editor
                    // itself was launched from a VS Code integrated terminal (VS Code's
                    // extension host is Electron), ELECTRON_RUN_AS_NODE leaks in and gets
                    // inherited by a detached child. Any Electron app (VS Code included)
                    // reads that var to mean "run as a plain Node CLI instead of the GUI",
                    // so without stripping it here, launching VS Code as the external editor
                    // silently runs `node <script>` instead of opening a window.
                    QProcess process;
                    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
                    environment.remove("ELECTRON_RUN_AS_NODE");
                    process.setProcessEnvironment(environment);
                    process.setProgram(externalEditorPath);
                    process.setArguments(QStringList() << assetPath);
                    process.startDetached();
                }

                // Everything else (scripts, shaders, models, textures) has no in-editor viewer/
                // editor yet - hand it to whatever the OS already has associated with the
                // extension, the same "open in your configured external editor" convention
                // Unity's Project window uses for a script double-click.
                QDesktopServices::openUrl(QUrl::fromLocalFile(assetPath));
            });

    m_DockManager->setSplitterSizes(sceneHierarchyArea, {250, 1000, 350});
    m_DockManager->setSplitterSizes(consoleArea, {700, 200});

    // Not parented to the console widget - it's owned by this shared_ptr and by whichever
    // logger sinks() vectors hold a copy, so ownership can't be split with Qt's parent/child
    // deletion. The connection is torn down safely regardless, since QObject disconnects its
    // signals/slots automatically once the receiving widget is destroyed.
    m_ConsoleSink = std::make_shared<ConsoleSink>();
    connect(m_ConsoleSink.get(), &ConsoleSink::MessageLogged, consolePanel, &ConsolePanel::AppendMessage);

    AttachSink(m_ConsoleSink, MT_CORE_LOGGER);
    AttachSink(m_ConsoleSink, MT_CLIENT_LOGGER);
}

EditorMainWindow::~EditorMainWindow()
{
    DetachSink(m_ConsoleSink, MT_CORE_LOGGER);
    DetachSink(m_ConsoleSink, MT_CLIENT_LOGGER);
}

void EditorMainWindow::closeEvent(QCloseEvent* event)
{
    if (ConfirmDiscardUnsavedChanges(this, m_SceneManager))
        event->accept();
    else
        event->ignore();
}

void EditorMainWindow::UpdateWindowTitle()
{
    Matcha::SceneManager& sceneManager = m_SceneManager;

    QString sceneName = sceneManager.GetFilePath().empty()
                            ? "Untitled"
                            : QFileInfo(QString::fromStdString(sceneManager.GetFilePath())).completeBaseName();

    setWindowTitle(QString("%1%2 - Matcha Editor").arg(sceneManager.IsDirty() ? "*" : "", sceneName));
}
}  // namespace MatchaEditor
