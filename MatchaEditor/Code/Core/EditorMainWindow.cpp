#include "EditorMainWindow.h"
#include "ConsoleSink.h"
#include "Panels/ConsolePanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/InspectorPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/MenuChrome.h"
#include "Core/Logger.h"
#include "Core/Qt/QtViewportWidget.h"

#include <spdlog/spdlog.h>

#include <DockAreaWidget.h>
#include <DockManager.h>

#include <QCloseEvent>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>

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

EditorMainWindow::EditorMainWindow(Matcha::EngineContext& context, Matcha::QtViewportWidget* viewport, QWidget* parent)
    : QMainWindow(parent),
      m_Context(context)
{
    setWindowIcon(MakePlaceholderIcon());

    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::DockAreaHasTabsMenuButton, true);
    ads::CDockManager::setConfigFlag(ads::CDockManager::OpaqueSplitterResize, true);
    m_DockManager = new ads::CDockManager(this);
    m_DockManager->setStyleSheet(m_DockManager->styleSheet() + DockChromeStyleSheetOverrides());
    setCentralWidget(m_DockManager);

    MenuChrome menuChrome(this, context, m_CommandManager);

    // Unlike Scene::AddOnSceneChanged (which InspectorPanel/SceneHierarchyWidget have to
    // re-subscribe to on every scene swap, since it dies with the Scene it's attached to), these
    // two are SceneManager's own callbacks - SceneManager itself outlives any one Scene, so a
    // single subscription here covers every scene for the rest of the editor's lifetime.
    UpdateWindowTitle();
    context.GetSceneManager().AddOnDirtyChanged([this] { UpdateWindowTitle(); });
    context.GetSceneManager().AddOnSceneReplaced([this] { UpdateWindowTitle(); });

    // A Command resolves its target(s) fresh out of the live Scene at every Execute()/Undo() (see
    // CommandManager.h) - once New/Open destroys that Scene, every command on both stacks is
    // meaningless, so drop them rather than leave them to fail silently (or resolve into whatever
    // entity happens to reuse the same UUID by coincidence, which can't happen, but is exactly
    // the kind of thing this guards against on principle).
    context.GetSceneManager().AddOnSceneReplaced([this] { m_CommandManager.Clear(); });

    SceneHierarchyPanel* sceneHierarchyPanel = new SceneHierarchyPanel(m_DockManager, context, m_CommandManager, this);
    ads::CDockAreaWidget* sceneHierarchyArea = m_DockManager->addDockWidget(ads::LeftDockWidgetArea, sceneHierarchyPanel);
    menuChrome.AddPanel(sceneHierarchyPanel);

    ViewportPanel* viewportPanel = new ViewportPanel(m_DockManager, viewport, this);
    ads::CDockAreaWidget* viewportArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, viewportPanel, sceneHierarchyArea);
    menuChrome.AddPanel(viewportPanel);

    InspectorPanel* inspectorPanel = new InspectorPanel(m_DockManager, context, m_CommandManager, this);
    ads::CDockAreaWidget* inspectorArea = m_DockManager->addDockWidget(ads::RightDockWidgetArea, inspectorPanel, viewportArea);
    menuChrome.AddPanel(inspectorPanel);
    connect(sceneHierarchyPanel, &SceneHierarchyPanel::SelectionChanged, inspectorPanel, &InspectorPanel::SetSelectedEntities);

    ConsolePanel* consolePanel = new ConsolePanel(m_DockManager, this);
    ads::CDockAreaWidget* consoleArea = m_DockManager->addDockWidget(ads::BottomDockWidgetArea, consolePanel);
    menuChrome.AddPanel(consolePanel);

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
    if (ConfirmDiscardUnsavedChanges(this, m_Context))
        event->accept();
    else
        event->ignore();
}

void EditorMainWindow::UpdateWindowTitle()
{
    Matcha::SceneManager& sceneManager = m_Context.GetSceneManager();

    QString sceneName = sceneManager.GetFilePath().empty()
                            ? "Untitled"
                            : QFileInfo(QString::fromStdString(sceneManager.GetFilePath())).completeBaseName();

    setWindowTitle(QString("%1%2 - Matcha Editor").arg(sceneManager.IsDirty() ? "*" : "", sceneName));
}
}  // namespace MatchaEditor
